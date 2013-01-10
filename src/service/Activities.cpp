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

#include "NepomukActivityManager.h"

#include "activitiesadaptor.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QUuid>
#include <QDebug>

#include <KLocalizedString>

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
    QDBusConnection::sessionBus().registerObject(
            ACTIVITY_MANAGER_OBJECT_PATH(Activities), this);

    // Initializing config

    connect(&d->configSyncTimer, SIGNAL(timeout()),
             d.get(), SLOT(configSync()));

    d->configSyncTimer.setSingleShot(true);

    d->ksmserver = new KSMServer(this);
    connect(d->ksmserver, SIGNAL(activitySessionStateChanged(QString, int)),
            d.get(), SLOT(activitySessionStateChanged(QString, int)));

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

    // Synchronizing with nepomuk

#ifdef HAVE_NEPOMUK
    EXEC_NEPOMUK( init(this) );

    connect(this, SIGNAL(CurrentActivityChanged(QString)),
            NepomukActivityManager::self(), SLOT(setCurrentActivity(QString)));
    connect(this, SIGNAL(ActivityAdded(QString)),
            NepomukActivityManager::self(), SLOT(addActivity(QString)));
    connect(this, SIGNAL(ActivityRemoved(QString)),
            NepomukActivityManager::self(), SLOT(removeActivity(QString)));
#endif

    d->loadLastActivity();
}

Activities::~Activities()
{
}

QString Activities::CurrentActivity() const
{
    return d->currentActivity;
}

bool Activities::SetCurrentActivity(const QString & id)
{
    if (id.isEmpty()) {
        return false;
    }

    return d->setCurrentActivity(id);
}

bool Activities::Private::setCurrentActivity(const QString & activity)
{
    using namespace Jobs;
    using namespace Jobs::General;

    DEFINE_ORDERED_SCHEDULER(setCurrentActivityJob);

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

    //   - unmount private activities
    //   - change the current activity and signal the change

    setCurrentActivityJob

    <<  // Change the activity
        General::call(this, "emitCurrentActivityChanged", activity);

    setCurrentActivityJob.start();

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

void Activities::Private::emitCurrentActivityChanged(const QString & id)
{
    // Saving the current activity, and notifying
    // clients of the change

    currentActivity = id;
    mainConfig().writeEntry("currentActivity", id);

    EXEC_NEPOMUK( setCurrentActivity(id) );

    scheduleConfigSync();

    emit q->CurrentActivityChanged(id);
}

QString Activities::AddActivity(const QString & name)
{
    QString id;

    // Ensuring a new Uuid. The loop should usually end after only
    // one iteration

    val & existingActivities = d->activities.keys();
    while (id.isEmpty() || existingActivities.contains(id)) {
        id = QUuid::createUuid();
        id.replace(QRegExp("[{}]"), QString());
    }

    // Saves the activity info to the config

    d->setActivityState(id, Running);

    SetActivityName(id, name);

    emit ActivityAdded(id);

    d->scheduleConfigSync(true);
    return id;
}

void Activities::RemoveActivity(const QString & activity)
{
    // Sanity checks
    if (!d->activities.contains(activity)) return;

    DEFINE_ORDERED_SCHEDULER(removeActivityJob);

    using namespace Jobs;
    using namespace Jobs::General;

    //   - stop
    //   - if it was current, switch to a running activity
    //   - remove from configs
    //   - signal the event

    removeActivityJob

    <<  // Remove
        General::call(d.get(), "removeActivity", activity, true /* wait finished */);

    removeActivityJob.start();
}

void Activities::Private::removeActivity(const QString & activity)
{
    // TODO: transitioningActivity should be removed,
    // we need to check whether the activity is in a
    // proper state to be deleted

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

QString Activities::Private::activityName(const QString & id)
{
    return activitiesConfig().readEntry(id, QString());
}

QString Activities::Private::activityIcon(const QString & id)
{
    return activityIconsConfig().readEntry(id, QString());
}

void Activities::Private::scheduleConfigSync(const bool soon)
{
    static val shortInterval = 5 * 1000;
    static val longInterval  = 2 * 60 * 1000;

    // short interval has priority to the long one
    if (soon) {
        if (configSyncTimer.interval() != shortInterval) {
            // always change to shortInterval if the current one is longInterval.
            configSyncTimer.stop();
            configSyncTimer.setInterval(shortInterval);
        }
    } else if (configSyncTimer.interval() != longInterval && !configSyncTimer.isActive()) {
        configSyncTimer.setInterval(longInterval);
    }

    if (!configSyncTimer.isActive()) {
        configSyncTimer.start();
    }
}

void Activities::Private::configSync()
{
    configSyncTimer.stop();
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

    foreach (const QString & activityId, ListActivities()) {
        result << ActivityInformation(activityId);
    }

    return result;
}

ActivityInfo Activities::ActivityInformation(const QString & activityId) const
{
    ActivityInfo activityInfo;
    activityInfo.id    = activityId;
    activityInfo.name  = ActivityName(activityId);
    activityInfo.icon  = ActivityIcon(activityId);
    activityInfo.state = ActivityState(activityId);
    return activityInfo;
}

QString Activities::ActivityName(const QString & id) const
{
    return d->activityName(id);
}

void Activities::SetActivityName(const QString & id, const QString & name)
{
    if (!d->activities.contains(id)) return;

    if (name == d->activityName(id)) return;

    d->activitiesConfig().writeEntry(id, name);

    EXEC_NEPOMUK( setActivityName(id, name) );

    d->scheduleConfigSync();

    emit ActivityNameChanged(id, name);
    emit ActivityChanged(id);
}

QString Activities::ActivityIcon(const QString & id) const
{
    return d->activityIcon(id);
}

void Activities::SetActivityIcon(const QString & id, const QString & icon)
{
    if (!d->activities.contains(id)) return;

    d->activityIconsConfig().writeEntry(id, icon);

    EXEC_NEPOMUK( setActivityIcon(id, icon) );

    d->scheduleConfigSync();

    emit ActivityIconChanged(id, icon);
    emit ActivityChanged(id);
}

void Activities::Private::setActivityState(const QString & id, Activities::State state)
{
    if (activities[id] == state) return;

    // Treating 'Starting' as 'Running', and 'Stopping' as 'Stopped'
    // as far as the config file is concerned
    bool configNeedsUpdating = ((activities[id] & 4) != (state & 4));

    activities[id] = state;

    switch (state) {
        case Activities::Running:
            emit q->ActivityStarted(id);
            break;

        case Activities::Stopped:
            emit q->ActivityStopped(id);
            break;

        default:
            break;
    }

    emit q->ActivityStateChanged(id, state);

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

void Activities::StartActivity(const QString & id)
{
    qDebug() << "Starting activity" << id;

    if (!d->activities.contains(id) ||
            d->activities[id] != Stopped) {
        qDebug() << "Activity is not stopped..." << d->activities[id];
        return;
    }

    qDebug() << "Starting the session";
    d->setActivityState(id, Starting);
    d->ksmserver->startActivitySession(id);
}

void Activities::StopActivity(const QString & id)
{
    qDebug() << "Stopping activity" << id;

    if (!d->activities.contains(id) ||
            d->activities[id] == Stopped) {
        qDebug() << "Already stopped";
        return;
    }

    qDebug() << "Stopping the session";
    d->setActivityState(id, Stopping);
    d->ksmserver->stopActivitySession(id);
}

void Activities::Private::activitySessionStateChanged(const QString & activity, int status)
{
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

int Activities::ActivityState(const QString & id) const
{
    return d->activities.contains(id) ? d->activities[id] : Invalid;
}

bool Activities::isFeatureOperational(const QStringList & feature) const
{
    Q_UNUSED(feature)

    // if (feature.first() == "encryption") {
    //     return Jobs::Encryption::Common::isEnabled();
    // }

    return false;
}

bool Activities::isFeatureEnabled(const QStringList & feature) const
{
    return
        (feature.size() != 2)         ? false :
        // (feature[0] == "encryption")  ? d->isActivityEncrypted(feature[1]) :
        /* otherwise */                false;
}

void Activities::setFeatureEnabled(const QStringList & feature, bool value)
{
    Q_UNUSED(value)

    if (feature.size() != 2) return;

    // if (feature[0] == "encryption") {
    //      d->setActivityEncrypted(feature[1], value);
    // }
}

QStringList Activities::listFeatures(const QStringList & feature) const
{
    Q_UNUSED(feature)
    static QStringList features/* ("encryption") */;

    return features;
}

