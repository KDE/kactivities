/*
 *   Copyright (C) 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "Activities.h"
#include "Activities_p.h"

#include "activitiesadaptor.h"

#include <QDBusConnection>
#include <QUuid>
#include <QDebug>

#include <kauthorized.h>
#include <kdbusconnectionpool.h>
#include <config-features.h>

#include "jobs/activity/all.h"
#include "jobs/general/all.h"
#include "jobs/schedulers/all.h"
#include "jobs/ksmserver/KSMServer.h"

#include "common.h"

#include <utils/nullptr.h>
#include <utils/d_ptr_implementation.h>
#include <utils/find_if_assoc.h>
#include <utils/val.h>

// Private

Activities::Private::Private(Activities * parent)
    : config("activitymanagerrc"),
      q(parent)
{
}

Activities::Private::~Private()
{
    configSync();
}

// Main

Activities::Activities(QObject * parent)
    : Module("activities", parent), d(this)
{
    qDebug() << "\n\n-------------------------------------------------------";
    qDebug() << "Starting the KDE Activity Manager daemon" << QDateTime::currentDateTime();
    qDebug() << "-------------------------------------------------------";

    // Basic initialization //////////////////////////////////////////////////////////////////////////////////

    // Initializing D-Bus service

    new ActivitiesAdaptor(this);
    KDBusConnectionPool::threadConnection().registerObject(
            ACTIVITY_MANAGER_OBJECT_PATH(Activities), this);

    // Initializing config

    d->connect(&d->configSyncTimer, SIGNAL(timeout()),
            SLOT(configSync()));

    d->configSyncTimer.setSingleShot(true);

    d->ksmserver = new KSMServer(this);
    d->connect(d->ksmserver, SIGNAL(activitySessionStateChanged(QString, int)),
            SLOT(activitySessionStateChanged(QString, int)));

    // Activity initialization ///////////////////////////////////////////////////////////////////////////////

    // Reading activities from the config file

    foreach (val & activity, d->activitiesConfig().keyList()) {
        d->activities[activity] = Activities::Stopped;
    }

    val & runningActivities = d->mainConfig().readEntry("runningActivities", d->activities.keys());

    foreach (val & activity, runningActivities) {
        if (d->activities.contains(activity)) {
            d->activities[activity] = Activities::Running;
        }
    }

    d->loadLastActivity();
}

Activities::~Activities()
{
}

QString Activities::CurrentActivity() const
{
    return d->currentActivity;
}

bool Activities::SetCurrentActivity(const QString & activity)
{
    // Public method can not put us in a limbo state
    if (activity.isEmpty()) {
        return false;
    }

    return d->setCurrentActivity(activity);
}

bool Activities::Private::setCurrentActivity(const QString & activity)
{
    using namespace Jobs;
    using namespace Jobs::General;

    // If the activity is empty, this means we are entering a limbo state
    if (activity.isEmpty()) {
        currentActivity.clear();
        emit q->CurrentActivityChanged(currentActivity);
        return true;
    }

    // Sanity checks
    if (!activities.contains(activity)) return false;
    if (currentActivity == activity)    return true;

    // Start activity
    // TODO: Move this to job-based execution
    q->StartActivity(activity);

    //   - change the current activity and signal the change
    emitCurrentActivityChanged(activity);

    return true;
}

void Activities::Private::loadLastActivity()
{
    // If there are no public activities, try to load the last used activity
    val & lastUsedActivity = mainConfig().readEntry("currentActivity", QString());

    setCurrentActivity(
        (lastUsedActivity.isEmpty() && activities.size() > 0)
            ? activities.keys().at(0)
            : lastUsedActivity
        );
}

void Activities::Private::emitCurrentActivityChanged(const QString & activity)
{
    // Saving the current activity, and notifying
    // clients of the change

    currentActivity = activity;
    mainConfig().writeEntry("currentActivity", activity);

    scheduleConfigSync();

    emit q->CurrentActivityChanged(activity);
}

QString Activities::AddActivity(const QString & name)
{
    if (!KAuthorized::authorize("plasma-desktop/add_activities")) {
        return QString();
    }

    if (name.isEmpty()) {
        Q_ASSERT(!name.isEmpty());
        return QString();
    }

    QString activity;

    // Ensuring a new Uuid. The loop should usually end after only
    // one iteration

    val & existingActivities = d->activities.keys();
    while (activity.isEmpty() || existingActivities.contains(activity)) {
        activity = QUuid::createUuid();
        activity.replace(QRegExp("[{}]"), QString());
    }

    // Saves the activity info to the config

    d->activities[activity] = Invalid;
    d->setActivityState(activity, Running);

    SetActivityName(activity, name);

    emit ActivityAdded(activity);

    d->scheduleConfigSync(true);
    return activity;
}

void Activities::RemoveActivity(const QString & activity)
{
    if (!KAuthorized::authorize("plasma-desktop/add_activities")) {
        return;
    }

    // Sanity checks
    if (!d->activities.contains(activity)) {
        return;
    }

    d->removeActivity(activity);
}

void Activities::Private::removeActivity(const QString & activity)
{
    qDebug() << activities << activity;
    Q_ASSERT(!activity.isEmpty());
    Q_ASSERT(activities.contains(activity));

    // If the activity is running, stash it
    q->StopActivity(activity);

    setActivityState(activity, Activities::Invalid);

    // Removing the activity
    activities.remove(activity);
    activitiesConfig().deleteEntry(activity);

    // If the removed activity was the current one,
    // set another activity as current
    if (currentActivity == activity) {
        ensureCurrentActivityIsRunning();
    }

    emit q->ActivityRemoved(activity);
    configSync();
}


KConfigGroup Activities::Private::activityIconsConfig()
{
    return KConfigGroup(&config, "activities-icons");
}

KConfigGroup Activities::Private::activitiesConfig()
{
    return KConfigGroup(&config, "activities");
}

KConfigGroup Activities::Private::mainConfig()
{
    return KConfigGroup(&config, "main");
}

QString Activities::Private::activityName(const QString & activity)
{
    return activitiesConfig().readEntry(activity, QString());
}

QString Activities::Private::activityIcon(const QString & activity)
{
    return activityIconsConfig().readEntry(activity, QString());
}

void Activities::Private::scheduleConfigSync(const bool soon)
{
    static const auto shortInterval = 1000;
    static const auto longInterval = 2 * 60 * 1000;

    // If the timer is not running, or has a longer interval than we need,
    // start it
    if ((soon && configSyncTimer.interval() > shortInterval)
            || !configSyncTimer.isActive()) {

        QMetaObject::invokeMethod(
            &configSyncTimer, "start", Qt::QueuedConnection,
            Q_ARG(int, soon ? shortInterval : longInterval));
    }
}

void Activities::Private::configSync()
{
    QMetaObject::invokeMethod(&configSyncTimer, "stop", Qt::QueuedConnection);
    config.sync();
}

QStringList Activities::ListActivities() const
{
    qDebug() << "This is the current thread id for Activities" << QThread::currentThreadId() << QThread::currentThread();
    return d->activities.keys();
}

QStringList Activities::ListActivities(int state) const
{
    return d->activities.keys((State)state);
}

QList<ActivityInfo> Activities::ListActivitiesWithInformation() const
{
    QList<ActivityInfo> result;

    foreach (const QString & activity, ListActivities()) {
        result << ActivityInformation(activity);
    }

    return result;
}

ActivityInfo Activities::ActivityInformation(const QString & activity) const
{
    if (!d->activities.contains(activity)) return ActivityInfo();

    ActivityInfo activityInfo;
    activityInfo.id    = activity;
    activityInfo.name  = ActivityName(activity);
    activityInfo.icon  = ActivityIcon(activity);
    activityInfo.state = ActivityState(activity);
    return activityInfo;
}

QString Activities::ActivityName(const QString & activity) const
{
    if (!d->activities.contains(activity)) return QString();

    return d->activityName(activity);
}

void Activities::SetActivityName(const QString & activity, const QString & name)
{
    if (!d->activities.contains(activity)) return;

    if (name == d->activityName(activity)) return;

    d->activitiesConfig().writeEntry(activity, name);

    d->scheduleConfigSync(true);

    emit ActivityNameChanged(activity, name);
    emit ActivityChanged(activity);
}

QString Activities::ActivityIcon(const QString & activity) const
{
    if (!d->activities.contains(activity)) return QString();

    return d->activityIcon(activity);
}

void Activities::SetActivityIcon(const QString & activity, const QString & icon)
{
    if (!d->activities.contains(activity)) return;

    d->activityIconsConfig().writeEntry(activity, icon);

    d->scheduleConfigSync();

    emit ActivityIconChanged(activity, icon);
    emit ActivityChanged(activity);
}

void Activities::Private::setActivityState(const QString & activity, Activities::State state)
{
    qDebug() << activities << activity;
    Q_ASSERT(activities.contains(activity));

    if (activities.value(activity) == state) return;

    // Treating 'Starting' as 'Running', and 'Stopping' as 'Stopped'
    // as far as the config file is concerned
    bool configNeedsUpdating = ((activities[activity] & 4) != (state & 4));

    activities[activity] = state;

    switch (state) {
        case Activities::Running:
            emit q->ActivityStarted(activity);
            break;

        case Activities::Stopped:
            emit q->ActivityStopped(activity);
            break;

        default:
            break;
    }

    emit q->ActivityStateChanged(activity, state);

    if (configNeedsUpdating) {
        mainConfig().writeEntry("runningActivities",
                activities.keys(Activities::Running) +
                activities.keys(Activities::Starting));
        scheduleConfigSync();
    }
}

void Activities::Private::ensureCurrentActivityIsRunning()
{
    // If the current activity is not running,
    // make some other activity current

    val & runningActivities = q->ListActivities(Activities::Running);

    if (!runningActivities.contains(currentActivity)) {
        if (runningActivities.size() > 0) {
            qDebug() << "Somebody called ensureCurrentActivityIsRunning?";
            setCurrentActivity(runningActivities.first());
        }
    }
}

// Main

void Activities::StartActivity(const QString & activity)
{
    if (!d->activities.contains(activity) ||
            d->activities[activity] != Stopped) {
        return;
    }

    qDebug() << "Starting the session";
    d->setActivityState(activity, Starting);
    d->ksmserver->startActivitySession(activity);
}

void Activities::StopActivity(const QString & activity)
{
    if (!d->activities.contains(activity) ||
            d->activities[activity] == Stopped) {
        return;
    }

    qDebug() << "Stopping the session";
    d->setActivityState(activity, Stopping);
    d->ksmserver->stopActivitySession(activity);
}

void Activities::Private::activitySessionStateChanged(const QString & activity, int status)
{
    if (!activities.contains(activity)) return;

    switch (status) {
        case KSMServer::Started:
        case KSMServer::FailedToStop:
            setActivityState(activity, Activities::Running);
            break;

        case KSMServer::Stopped:
            setActivityState(activity, Activities::Stopped);

            if (currentActivity == activity) {
                ensureCurrentActivityIsRunning();
            }

            break;
    }

    configSync();
}

int Activities::ActivityState(const QString & activity) const
{
    return d->activities.contains(activity) ? d->activities[activity] : Invalid;
}

bool Activities::isFeatureOperational(const QStringList & feature) const
{
    Q_UNUSED(feature)

    return false;
}

bool Activities::isFeatureEnabled(const QStringList & feature) const
{
    Q_UNUSED(feature)

    return false;
}

void Activities::setFeatureEnabled(const QStringList & feature, bool value)
{
    Q_UNUSED(feature)
    Q_UNUSED(value)
}

QStringList Activities::listFeatures(const QStringList & feature) const
{
    Q_UNUSED(feature)

    return QStringList();
}

