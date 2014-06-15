/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "activitiescache_p.h"
#include "manager_p.h"

#include <mutex>
#include <memory>

#include <QString>

#include "mainthreadexecutor_p.h"

namespace KActivities {

static QString nulluuid = QStringLiteral("00000000-0000-0000-0000-000000000000");


std::shared_ptr<ActivitiesCache> ActivitiesCache::self()
{
    static std::weak_ptr<ActivitiesCache> s_instance;
    static std::mutex singleton;
    std::lock_guard<std::mutex> singleton_lock(singleton);

    auto result = s_instance.lock();

    if (s_instance.expired()) {
        runInMainThread([&result] {
            result.reset(new ActivitiesCache());
            s_instance = result;
            });
    }

    return std::move(result);
}

ActivitiesCache::ActivitiesCache()
    : m_status(Consumer::NotRunning)
{
    // qDebug() << "ActivitiesCache: Creating a new instance";
    using org::kde::ActivityManager::Activities;

    auto activities = Manager::self()->activities();

    connect(activities, &Activities::ActivityAdded,
            this, &ActivitiesCache::updateActivity);
    connect(activities, &Activities::ActivityChanged,
            this, &ActivitiesCache::updateActivity);
    connect(activities, &Activities::ActivityRemoved,
            this, &ActivitiesCache::removeActivity);

    connect(activities, &Activities::ActivityStateChanged,
            this, &ActivitiesCache::updateActivityState);
    connect(activities, &Activities::ActivityNameChanged,
            this, &ActivitiesCache::setActivityName);
    connect(activities, &Activities::ActivityIconChanged,
            this, &ActivitiesCache::setActivityIcon);

    connect(activities, &Activities::CurrentActivityChanged,
            this, &ActivitiesCache::setCurrentActivity);

    connect(Manager::self(), &Manager::serviceStatusChanged,
            this, &ActivitiesCache::setServiceStatus);

    // These are covered by ActivityStateChanged
    // signal void org.kde.ActivityManager.Activities.ActivityStarted(QString activity)
    // signal void org.kde.ActivityManager.Activities.ActivityStopped(QString activity)

    setServiceStatus(Manager::self()->isServiceRunning());
}

void ActivitiesCache::setServiceStatus(bool status)
{
    // qDebug() << "Setting service status to:" << status;
    loadOfflineDefaults();

    if (status) {
        updateAllActivities();
    }
}

void ActivitiesCache::loadOfflineDefaults()
{
    m_status = Consumer::NotRunning;

    m_activities.clear();
    m_activities << ActivityInfo(nulluuid, QString(), QString(), Info::Running);
    m_currentActivity = nulluuid;

    emit serviceStatusChanged(m_status);
    emit activityListChanged();
}

ActivitiesCache::~ActivitiesCache()
{
    // qDebug() << "ActivitiesCache: Destroying the instance";
}

void ActivitiesCache::removeActivity(const QString &id)
{
    // qDebug() << "Removing the activity";

    auto where = std::lower_bound(
        m_activities.begin(), m_activities.end(), ActivityInfo(id));

    if (where != m_activities.end() && where->id == id) {
        m_activities.erase(where);
        emit activityRemoved(id);
        emit activityListChanged();

    } else {
        // qFatal("Requested to delete an non-existent activity");
    }
}

void ActivitiesCache::updateAllActivities()
{
    // qDebug() << "Updating all";
    m_status = Consumer::Unknown;
    emit serviceStatusChanged(m_status);

    // Loading the current activity
    auto call = Manager::self()->activities()->asyncCall(
        QStringLiteral("CurrentActivity"));
    auto watcher = new QDBusPendingCallWatcher(call, this);

    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)),
            this, SLOT(setCurrentActivityFromReply(QDBusPendingCallWatcher *)));

    // Loading all the activities
    call = Manager::self()->activities()->asyncCall(
        QStringLiteral("ListActivitiesWithInformation"));
    watcher = new QDBusPendingCallWatcher(call, this);

    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)),
            this, SLOT(setAllActivitiesFromReply(QDBusPendingCallWatcher *)));
}

void ActivitiesCache::updateActivity(const QString &id)
{
    // qDebug() << "Updating activity" << id;

    auto call = Manager::self()->activities()->asyncCall(
        QStringLiteral("ActivityInformation"), id);
    auto watcher = new QDBusPendingCallWatcher(call, this);

    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)),
            this, SLOT(setActivityInfoFromReply(QDBusPendingCallWatcher *)));
}

void ActivitiesCache::updateActivityState(const QString &id, int state)
{
    // qDebug() << "Updating activity state" << id << "to" << state;

    auto where = std::lower_bound(
        m_activities.begin(), m_activities.end(), ActivityInfo(id));

    if (where != m_activities.end() && where->id == id) {
        where->state = state;

        emit activityStateChanged(id, state);

    } else {
        // qFatal("Requested to update the state of an non-existent activity");
    }
}

template <typename _Result, typename _Functor>
void ActivitiesCache::passInfoFromReply(QDBusPendingCallWatcher *watcher, _Functor f)
{
    QDBusPendingReply<_Result> reply = *watcher;

    if (!reply.isError()) {
        auto replyValue = reply.template argumentAt <0>();
        // qDebug() << "Got some reply" << replyValue;

        ((*this).*f)(replyValue);
    }

    watcher->deleteLater();
}

void ActivitiesCache::setActivityInfoFromReply(QDBusPendingCallWatcher *watcher)
{
    // qDebug() << "reply...";
    passInfoFromReply<ActivityInfo>(watcher, &ActivitiesCache::setActivityInfo);
}

void ActivitiesCache::setAllActivitiesFromReply(QDBusPendingCallWatcher *watcher)
{
    // qDebug() << "reply...";
    passInfoFromReply<ActivityInfoList>(watcher, &ActivitiesCache::setAllActivities);
}

void ActivitiesCache::setCurrentActivityFromReply(QDBusPendingCallWatcher *watcher)
{
    // qDebug() << "reply...";
    passInfoFromReply<QString>(watcher, &ActivitiesCache::setCurrentActivity);
}

void ActivitiesCache::setActivityInfo(const ActivityInfo &info)
{
    // qDebug() << "Setting activity info" << info.id;

    auto where
        = std::lower_bound(m_activities.begin(), m_activities.end(), info);

    if (where == m_activities.end() || where->id != info.id) {
        // We haven't found the activity with the specified id.
        // This means it is a new activity.
        m_activities.insert(where, info);
        emit activityAdded(info.id);
        emit activityListChanged();

    } else {
        // An existing activity changed
        *where = info;
        emit activityChanged(info.id);
    }
}

void ActivitiesCache::setActivityName(const QString &id, const QString &name)
{
    auto where = std::lower_bound(
        m_activities.begin(), m_activities.end(), ActivityInfo(id));

    if (where != m_activities.end() && where->id == id) {
        where->name = name;

        emit activityNameChanged(id, name);

    } else {
        // qFatal("Requested to rename an non-existent activity");
    }
}

void ActivitiesCache::setActivityIcon(const QString &id, const QString &icon)
{
    auto where = std::lower_bound(
        m_activities.begin(), m_activities.end(), ActivityInfo(id));

    if (where != m_activities.end() && where->id == id) {
        where->icon = icon;

        emit activityIconChanged(id, icon);

    } else {
        // qFatal("Requested to change the icon of an non-existent activity");
    }
}

void ActivitiesCache::setAllActivities(const ActivityInfoList &_activities)
{
    // qDebug() << "Setting all activities";

    m_activities.clear();

    ActivityInfoList activities = _activities;

    qSort(activities.begin(), activities.end());

    foreach (const ActivityInfo &info, activities) {
        m_activities << info;
    }

    m_status = Consumer::Running;
    emit serviceStatusChanged(m_status);
    emit activityListChanged();
}

void ActivitiesCache::setCurrentActivity(const QString &activity)
{
    // qDebug() << "Setting current activity to" << activity;

    if (m_currentActivity == activity) {
        return;
    }

    m_currentActivity = activity;

    emit currentActivityChanged(activity);
}

} // namespace KActivities

