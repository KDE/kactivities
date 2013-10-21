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

#include <QWeakPointer>

namespace KActivities {

static QString nulluuid = QStringLiteral("00000000-0000-0000-0000-000000000000");

QWeakPointer<ActivitiesCache> ActivitiesCache::s_instance;

QSharedPointer<ActivitiesCache> ActivitiesCache::self()
{
    if (!s_instance) {
        QSharedPointer<ActivitiesCache> instance(new ActivitiesCache());
        s_instance = instance;
        return instance;

    } else return s_instance.toStrongRef();
}

ActivitiesCache::ActivitiesCache()
    : m_status(Consumer::NotRunning)
{
    qDebug() << "Creating a new instance";
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
    connect(activities, &Activities::CurrentActivityChanged,
            this, &ActivitiesCache::setCurrentActivity);

    connect(Manager::self(), &Manager::serviceStatusChanged,
            this, &ActivitiesCache::setServiceStatus);

    setServiceStatus(Manager::self()->isServiceRunning());
}

void ActivitiesCache::setServiceStatus(bool status)
{
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
}

ActivitiesCache::~ActivitiesCache()
{
    qDebug() << "Destroying the instance";
}

void ActivitiesCache::removeActivity(const QString &id)
{
    qDebug() << "Removing the activity";

    auto where = std::lower_bound(
        m_activities.begin(), m_activities.end(), ActivityInfo(id));

    if (where != m_activities.end() && where->id == id) {
        m_activities.erase(where);
    }
}

void ActivitiesCache::updateAllActivities()
{
    qDebug() << "Updating all";
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
    qDebug() << "Updating activity" << id;

    auto call = Manager::self()->activities()->asyncCall(
        QStringLiteral("ActivityInformation"), id);
    auto watcher = new QDBusPendingCallWatcher(call, this);

    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)),
            this, SLOT(setActivityInfoFromReply(QDBusPendingCallWatcher *)));
}

void ActivitiesCache::updateActivityState(const QString &id, int state)
{
    qDebug() << "Updating activity state" << id << "to" << state;

    auto where = std::lower_bound(
        m_activities.begin(), m_activities.end(), ActivityInfo(id));

    if (where != m_activities.end() && where->id == id) {
        where->state = state;

        emit activityStateChanged(id, state);
    }
}

template <typename _Result, typename _Functor>
void ActivitiesCache::passInfoFromReply(QDBusPendingCallWatcher *watcher, _Functor f)
{
    QDBusPendingReply<_Result> reply = *watcher;

    if (!reply.isError()) {
        auto replyValue = reply.template argumentAt <0>();
        qDebug() << "Got some reply" << replyValue;

        ((*this).*f)(replyValue);
    }

    watcher->deleteLater();
}

void ActivitiesCache::setActivityInfoFromReply(QDBusPendingCallWatcher *watcher)
{
    qDebug() << "reply...";
    passInfoFromReply<ActivityInfo>(watcher, &ActivitiesCache::setActivityInfo);
}

void ActivitiesCache::setAllActivitiesFromReply(QDBusPendingCallWatcher *watcher)
{
    qDebug() << "reply...";
    passInfoFromReply<ActivityInfoList>(watcher, &ActivitiesCache::setAllActivities);
}

void ActivitiesCache::setCurrentActivityFromReply(QDBusPendingCallWatcher *watcher)
{
    qDebug() << "reply...";
    passInfoFromReply<QString>(watcher, &ActivitiesCache::setCurrentActivity);
}

void ActivitiesCache::setActivityInfo(const ActivityInfo &info)
{
    qDebug() << "Setting activity info" << info.id;

    auto where
        = std::lower_bound(m_activities.begin(), m_activities.end(), info);

    if (where == m_activities.end() || where->id != info.id) {
        m_activities.insert(where, info);
        emit activityAdded(info.id);

    } else {
        *where = info;
        emit activityChanged(info.id);
    }
}

void ActivitiesCache::setAllActivities(const ActivityInfoList &_activities)
{
    qDebug() << "Setting all activities";

    m_activities.clear();

    ActivityInfoList activities = _activities;

    qSort(activities.begin(), activities.end());

    foreach (const ActivityInfo &info, activities) {
        m_activities << info;
    }

    m_status = Consumer::Running;
    emit serviceStatusChanged(m_status);
}

void ActivitiesCache::setCurrentActivity(const QString &activity)
{
    qDebug() << "Setting current activity to" << activity;

    if (m_currentActivity == activity) {
        return;
    }

    m_currentActivity = activity;

    emit currentActivityChanged(activity);
}

} // namespace KActivities

