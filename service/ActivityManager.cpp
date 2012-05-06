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

#include "ActivityManager.h"
#include "ActivityManager_p.h"
#include "NepomukActivityManager.h"

#include <QUuid>
#include <QDBusConnection>

#include <KConfig>
#include <KConfigGroup>
#include <KCrash>
#include <KLocale>
#include <KUrl>
#include <KDebug>

#include <KWindowSystem>

#include "activitymanageradaptor.h"
#include "EventProcessor.h"

#include "jobs/schedulers/all.h"
#include "jobs/activity/all.h"
#include "jobs/encryption/all.h"
#include "jobs/general/all.h"
#include "jobs/nepomuk/all.h"
#include "jobs/ui/all.h"

#include "ui/Ui.h"

#include "config-features.h"

// Private

ActivityManagerPrivate::ActivityManagerPrivate(ActivityManager * parent,
            QHash < WId, SharedInfo::WindowData > & _windows,
            QHash < KUrl, SharedInfo::ResourceData > & _resources
        )
    : config("activitymanagerrc"),
      windows(_windows),
      resources(_resources),
      q(parent),
      ksmserverInterface(0),
      screensaverInterface(0)
{
}

ActivityManagerPrivate::~ActivityManagerPrivate()
{
    configSync();
}

// Main

ActivityManager::ActivityManager()
    : d(new ActivityManagerPrivate(this,
            SharedInfo::self()->m_windows,
            SharedInfo::self()->m_resources))
{
    kDebug() << "\n\n-------------------------------------------------------";
    kDebug() << "Starting the KDE Activity Manager daemon" << QDateTime::currentDateTime();
    kDebug() << "-------------------------------------------------------";

    // Basic initialization //////////////////////////////////////////////////////////////////////////////////

    // TODO: We should move away from any gui-related code
    setQuitOnLastWindowClosed(false);

    // Initializing D-Bus service
    QDBusConnection dbus = QDBusConnection::sessionBus();
    new ActivityManagerAdaptor(this);
    dbus.registerService("org.kde.ActivityManager");
    dbus.registerObject("/ActivityManager", this);

    // KAMD is a daemon, if it crashes it is not a problem as
    // long as it restarts properly
    // NOTE: We have a custom crash handler
    KCrash::setFlags(KCrash::AutoRestart);

    // Initializing the event processor
    EventProcessor::self();

    // Initializing config
    connect(&d->configSyncTimer, SIGNAL(timeout()),
             d, SLOT(configSync()));

    d->configSyncTimer.setSingleShot(true);
    d->configSyncTimer.setInterval(2 * 60 * 1000);

    // Listening to active window changes
    connect(KWindowSystem::self(), SIGNAL(windowRemoved(WId)),
            d, SLOT(windowClosed(WId)));
    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)),
            d, SLOT(activeWindowChanged(WId)));

    // Listen to ksmserver for starting/stopping
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher("org.kde.ksmserver",
                                                           QDBusConnection::sessionBus(),
                                                           QDBusServiceWatcher::WatchForRegistration);
    connect(watcher, SIGNAL(serviceRegistered(QString)), d, SLOT(sessionServiceRegistered()));
    d->sessionServiceRegistered();

    // Listen to screensaver for starting/stopping
    QDBusServiceWatcher *watcher2 = new QDBusServiceWatcher("org.kde.screensaver",
                                                            QDBusConnection::sessionBus(),
                                                            QDBusServiceWatcher::WatchForRegistration);
    connect(watcher2, SIGNAL(serviceRegistered(QString)), d, SLOT(screensaverServiceRegistered()));
    d->screensaverServiceRegistered();

    // Activity initialization ///////////////////////////////////////////////////////////////////////////////

    // Reading activities from the config file

    foreach (const QString & activity, d->activitiesConfig().keyList()) {
        d->activities[activity] = ActivityManager::Stopped;
    }

    foreach (const QString & activity, d->mainConfig().readEntry("runningActivities", d->activities.keys())) {
        if (d->activities.contains(activity)) {
            d->activities[activity] = ActivityManager::Running;
        }
    }

    // Synchronizing with nepomuk

    EXEC_NEPOMUK( syncActivities(d->activities.keys(), d->activitiesConfig(), d->activityIconsConfig()) );

    d->loadLastPublicActivity();
}

ActivityManager::~ActivityManager()
{
    delete d;
}

void ActivityManager::SetActivityEncrypted(const QString & activity, bool encrypted)
{
    using namespace Jobs;
    using namespace Jobs::Ui;
    using namespace Jobs::Encryption;
    using namespace Jobs::Encryption::Common;
    // using namespace Jobs::General;

    // Is the encryption enabled?
    if (!Encryption::Common::isEnabled()) return;

    // check whether the previous state was the same
    if (encrypted == isActivityEncrypted(activity)) return;

    DEFINE_ORDERED_SCHEDULER(setActivityEncryptedJob);

    if (encrypted) {
        //   - ask for password
        //   - initialize mount
        //   - move the files, nepomuk stuff (TODO)

        setActivityEncryptedJob

        <<  // Ask for the password
            askPassword(i18n("Activity password"), i18n("Enter the password to use for encryption"), true)

        <<  // Try to mount, or die trying :)
            DO_OR_DIE(
                mount(activity),
                message(i18n("Error"), i18n("Error setting up the activity encryption")))

        <<  // Initialize the directories, move previous activity data from non-private folders
            initializeStructure(activity, InitializeStructure::InitializeInEncrypted)

        #ifdef HAVE_NEPOMUK
        <<  // Move the files that are linked to the activity to the encrypted folder
            Jobs::Nepomuk::move(activity, true)
        #endif

        <<  // Delete the old activity folder
            initializeStructure(activity, InitializeStructure::DeinitializeNormal)

        ;

    } else {
        //   - unmount
        //   - ask for password
        //   - mount
        //   - move the files, nepomuk stuff (TODO)
        //   - unmount
        //   - delete encryption directories

        setActivityEncryptedJob

        <<  // Unmount the activity
            unmount(activity)

        <<  // Mask the Ui as busy
            setBusy()

        <<  // Retrying to get the password until it succeeds or the user cancels password entry
            RETRY_JOB(
                askPassword(i18n("Unprotect activity"), i18n("You are cancelling the protection of this activity. Its content will become public again and be accessed without a password.")),
                mount(activity),
                message(i18n("Error"), i18n("Error unlocking the activity.\nYou've probably entered a wrong password."))
            )

        #ifdef HAVE_NEPOMUK
        <<  // Move the files that are linked to the activity to the encrypted folder
            Jobs::Nepomuk::move(activity, false)
        #endif

        <<  // Initialize the directories for the normal
            initializeStructure(activity, InitializeStructure::InitializeInNormal)

        <<  // Unmount the activity yet again
            FALLIBLE_JOB(unmount(activity))

        <<  // This will remove encrypted folder
            initializeStructure(activity, InitializeStructure::DeinitializeEncrypted)

        ;
    }

    setActivityEncryptedJob

    <<  // Signal the change
        General::call(this, "ActivityChanged", activity);

    connect(&setActivityEncryptedJob, SIGNAL(finished(KJob*)),
            ::Ui::self(), SLOT(unsetBusy()));

    setActivityEncryptedJob.start();
}

bool ActivityManager::IsActivityEncrypted(const QString & activity) const
{
    return Jobs::Encryption::Common::isActivityEncrypted(activity);
}

QString ActivityManager::CurrentActivity() const
{
    return d->currentActivity;
}

bool ActivityManager::SetCurrentActivity(const QString & id)
{
    if (id.isEmpty()) {
        return false;
    }

    return d->setCurrentActivity(id);
}

bool ActivityManagerPrivate::setCurrentActivity(const QString & activity)
{
    using namespace Jobs;
    using namespace Jobs::Ui;
    using namespace Jobs::Encryption;
    using namespace Jobs::Encryption::Common;
    using namespace Jobs::General;

    DEFINE_ORDERED_SCHEDULER(setCurrentActivityJob);

    // If the activity is empty, this means we are entering a limbo state
    if (activity.isEmpty()) {
        // If the current activity is private, unmount it
        if (isActivityEncrypted(currentActivity)) {
            setCurrentActivityJob

            <<  // Unmounting the activity, ignore the potential failure
                FALLIBLE_JOB(unmount(currentActivity));

            setCurrentActivityJob.start();
        }

        currentActivity.clear();
        emit q->CurrentActivityChanged(currentActivity);
        return true;
    }

    // Sanity checks
    if (!activities.contains(activity)) return false;
    if (currentActivity == activity)    return true;

    activitiesDesktopsConfig().writeEntry(currentActivity, QString::number(KWindowSystem::currentDesktop()));

    // Start activity
    // TODO: Move this to job-based execution
    q->StartActivity(activity);

    // If encrypted:
    //   - ask for password
    //   - try to unlock, cancel on fail
    //   continue like it is not encrypted
    //
    // If not encrypted
    //   - unmount private activities
    //   - change the current activity and signal the change

    // If the new activity is private
    if (isActivityEncrypted(activity)) {

        setCurrentActivityJob

        <<  // Mask the Ui as busy
            setBusy()

        <<  // Retrying to get the password until it succeeds or the user cancels password entry
            RETRY_JOB(
                askPassword(i18n("Unlock activity"), i18n("Enter the password to unlock the activity",
                            false, !currentActivityBeforeScreenLock.isEmpty())),
                mount(activity),
                message(i18n("Error"), i18n("Error unlocking the activity.\nYou've probably entered a wrong password."))
            )

        ;

        connect(&setCurrentActivityJob, SIGNAL(finished(KJob*)),
                                  this, SLOT(checkForSetCurrentActivityError(KJob*)));
    }

    // If the current activity is private, unmount it
    if (isActivityEncrypted(currentActivity)) {
        setCurrentActivityJob

        <<  // Unmounting the activity, ignore the potential failure
            FALLIBLE_JOB(unmount(currentActivity));
    }

    setCurrentActivityJob

    <<  // JIC, unmount everything apart from the new activity
        unmountExcept(activity)

    <<  // Change the activity
        General::call(this, "emitCurrentActivityChanged", activity);

    setCurrentActivityJob.start();

    connect(&setCurrentActivityJob, SIGNAL(finished(KJob*)),
            ::Ui::self(), SLOT(unsetBusy()));

    return true;
}

void ActivityManagerPrivate::loadLastPublicActivity()
{
    // Try to load the last used public (non-private) activity

    QString lastPublicActivity = mainConfig().readEntry("lastUnlockedActivity", QString());

    // If the last one turns out to be encrypted, try to load any public activity
    if (lastPublicActivity.isEmpty() || Jobs::Encryption::Common::isActivityEncrypted(lastPublicActivity)) {
        lastPublicActivity.clear();

        foreach (const QString & activity, activities.keys()) {
            if (!Jobs::Encryption::Common::isActivityEncrypted(activity)) {
                lastPublicActivity = activity;
                break;
            }
        }
    }

    if (!lastPublicActivity.isEmpty()) {
        // Setting the found public activity to be the current one
        kDebug() << "Setting the activity to be the last public activity" << lastPublicActivity;
        setCurrentActivity(lastPublicActivity);

    } else {
        // First, lets notify everybody that there is no current activity
        // Needs to be present so that until the activity gets unlocked,
        // the environment knows we are in a kind of limbo state
        setCurrentActivity(QString());

        // If there are no public activities, try to load the last used activity
        QString lastUsedActivity = mainConfig().readEntry("currentActivity", QString());

        if (lastUsedActivity.isEmpty() && activities.size() > 0) {
            setCurrentActivity(activities.keys().at(0));

        } else {
            setCurrentActivity(lastUsedActivity);

        }
    }
}

void ActivityManagerPrivate::checkForSetCurrentActivityError(KJob * job)
{
    if (job->error()) {
        loadLastPublicActivity();
    }
}

void ActivityManagerPrivate::emitCurrentActivityChanged(const QString & id)
{
    currentActivity = id;
    mainConfig().writeEntry("currentActivity", id);

    using namespace Jobs::Encryption::Common;

    if (!isActivityEncrypted(id)) {
        mainConfig().writeEntry("lastUnlockedActivity", id);
    }

    scheduleConfigSync();

    SharedInfo::self()->setCurrentActivity(id);
    emit q->CurrentActivityChanged(id);

    if (activitiesDesktopsConfig().hasKey(id)) {
        int desktopId = activitiesDesktopsConfig().readEntry(id).toInt();

        if (desktopId <= KWindowSystem::numberOfDesktops() && desktopId >= 0) {
            KWindowSystem::setCurrentDesktop(desktopId);
	}
    }
}

QString ActivityManager::AddActivity(const QString & name)
{
    // kDebug() << name;

    QString id;

    // Ensuring a new Uuid. The loop should usually end after only
    // one iteration
    QStringList existingActivities = d->activities.keys();
    while (id.isEmpty() || existingActivities.contains(id)) {
        id = QUuid::createUuid();
        id.replace(QRegExp("[{}]"), QString());
    }

    d->setActivityState(id, Running);

    SetActivityName(id, name);

    emit ActivityAdded(id);

    d->scheduleConfigSync(true);
    return id;
}

void ActivityManager::RemoveActivity(const QString & activity)
{
    // Sanity checks
    if (!d->activities.contains(activity)) return;

    DEFINE_ORDERED_SCHEDULER(removeActivityJob);

    // If encrypted:
    //   - unmount if current (hmh, we can't delete the current activity?)
    //   - ask for the password, and warn the user
    //   - delete files (TODO)
    //   - delete encryption directories (TODO)

    using namespace Jobs;
    using namespace Jobs::Ui;
    using namespace Jobs::Encryption;
    using namespace Jobs::Encryption::Common;
    using namespace Jobs::General;

    if (Jobs::Encryption::Common::isActivityEncrypted(activity)) {

        removeActivityJob

        <<  // We need to ask the user whether he really wants to delete the
            // activity once more since it implies removing the data as well
            // in the case of private activities
            TEST_JOB(
                ask(i18n("Confirmation"),
                    i18n("If you delete a private activity, all the documents and files that belong to it will also be deleted."),
                    QStringList()
                        << "Delete the activity"
                        << "Cancel"
                ), -1 /* Expecting the first choice */
            )

        <<  // unmount the activity
            unmount(activity)

        <<  // Retrying to get the password until it succeeds or the user cancels password entry
            RETRY_JOB(
                askPassword(i18n("Delete activity"), i18n("Enter the password to delete this protected activity.")),
                mount(activity),
                message(i18n("Error"), i18n("Error unlocking the activity.\nYou've probably entered a wrong password."))
            )

        <<  // Unmount the activity yet again, and deinitialize
            unmount(activity);
    }

    // If not encrypted:
    //   - stop
    //   - if it was current, switch to a running activity
    //   - remove from configs
    //   - signal the event

    removeActivityJob

    <<  // Delete the activity data
        initializeStructure(activity, InitializeStructure::DeinitializeBoth)

    <<  // Remove
        General::call(d, "removeActivity", activity, true /* waitFinished */);

    removeActivityJob.start();
}

void ActivityManagerPrivate::removeActivity(const QString & activity)
{
    // If the activity is running, stash it
    q->StopActivity(activity);

    setActivityState(activity, ActivityManager::Invalid);

    // Removing the activity
    activities.remove(activity);
    activitiesConfig().deleteEntry(activity);

    // If the removed activity was the current one,
    // set another activity as current
    if (currentActivity == activity) {
        ensureCurrentActivityIsRunning();
    }

    if (transitioningActivity == activity) {
        //very unlikely, but perhaps not impossible
        //but it being deleted doesn't mean ksmserver is un-busy..
        //in fact, I'm not quite sure what would happen.... FIXME
        //so I'll just add some output to warn that it happened.
        // kDebug() << "deleting activity in transition. watch out!";
    }

    emit q->ActivityRemoved(activity);
    configSync();
}

