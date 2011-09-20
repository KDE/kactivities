/*
 *   Copyright (C) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include <QUuid>
#include <QDBusConnection>

#include <KConfig>
#include <KConfigGroup>
#include <KCrash>
#include <KUrl>
#include <KDebug>

#include <KWindowSystem>

#ifdef HAVE_NEPOMUK
    #include <Nepomuk/ResourceManager>
    #include <Nepomuk/Resource>
    #include <Nepomuk/Variant>
    #include "nie.h"
#endif

#include "activitymanageradaptor.h"
#include "EventProcessor.h"

#include "config-features.h"

#ifdef HAVE_NEPOMUK
    #define NEPOMUK_RUNNING d->nepomukInitialized()
#else
    #define NEPOMUK_RUNNING false
#endif

#define ACTIVITIES_PROTOCOL "activities://"

// copied from kdelibs\kdeui\notifications\kstatusnotifieritemdbus_p.cpp
// if there is a common place for such definitions please move
#ifdef Q_OS_WIN64
__inline int toInt(WId wid)
{
	return (int)((__int64)wid);
}

#else
__inline int toInt(WId wid)
{
	return (int)wid;
}
#endif

// Private

ActivityManagerPrivate::ActivityManagerPrivate(ActivityManager * parent,
            QHash < WId, SharedInfo::WindowData > & _windows,
            QHash < KUrl, SharedInfo::ResourceData > & _resources
        )
    : haveSessions(false),
    config("activitymanagerrc"),
    windows(_windows),
    resources(_resources),
#ifdef HAVE_NEPOMUK
    m_nepomukInitCalled(false),
#endif
    q(parent)
{
    // Initializing config
    connect(&configSyncTimer, SIGNAL(timeout()),
             this, SLOT(configSync()));

    configSyncTimer.setSingleShot(true);
    configSyncTimer.setInterval(2 * 60 * 1000);

    kDebug() << "reading activities:";
    foreach(const QString & activity, activitiesConfig().keyList()) {
        kDebug() << activity;
        activities[activity] = ActivityManager::Stopped;
    }

    foreach(const QString & activity, mainConfig().readEntry("runningActivities", activities.keys())) {
        kDebug() << "setting" << activity << "as" << "running";
        if (activities.contains(activity)) {
            activities[activity] = ActivityManager::Running;
        }
    }

    currentActivity = mainConfig().readEntry("currentActivity", QString());
    kDebug() << "currentActivity is" << currentActivity;
    SharedInfo::self()->setCurrentActivity(currentActivity);

    connect(KWindowSystem::self(), SIGNAL(windowRemoved(WId)),
            this, SLOT(windowClosed(WId)));
    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)),
            this, SLOT(activeWindowChanged(WId)));

    //listen to ksmserver for starting/stopping
    ksmserverInterface = new QDBusInterface("org.kde.ksmserver", "/KSMServer", "org.kde.KSMServerInterface");
    if (ksmserverInterface->isValid()) {
        ksmserverInterface->setParent(this);
        connect(ksmserverInterface, SIGNAL(subSessionOpened()), this, SLOT(startCompleted()));
        connect(ksmserverInterface, SIGNAL(subSessionClosed()), this, SLOT(stopCompleted()));
        connect(ksmserverInterface, SIGNAL(subSessionCloseCanceled()), this, SLOT(stopCancelled())); //spelling fail :)
        haveSessions = true;
    } else {
        kDebug() << "couldn't connect to ksmserver! session stuff won't work";
        //note: in theory it's nice to try again later
        //but in practice, ksmserver is either there or it isn't (killing it logs you out)
        //so in this case there's no point. :)
        ksmserverInterface->deleteLater();
        ksmserverInterface = 0;
    }
}

ActivityManagerPrivate::~ActivityManagerPrivate()
{
    configSync();
}

void ActivityManagerPrivate::windowClosed(WId windowId)
{
    // kDebug() << "Window closed..." << windowId
    //          << "one of ours?" << windows.contains(windowId);

    if (!windows.contains(windowId)) {
        return;
    }

    foreach (const KUrl & uri, windows[windowId].resources) {
        q->RegisterResourceEvent(windows[windowId].application,
                toInt(windowId), uri.url(), Event::Closed, resources[uri].reason);
    }
}

void ActivityManagerPrivate::activeWindowChanged(WId windowId)
{
    Q_UNUSED(windowId)
    // kDebug() << "Window focussed..." << windowId
    //          << "one of ours?" << windows.contains(windowId);

}

void ActivityManagerPrivate::setActivityState(const QString & id, ActivityManager::State state)
{
    if (activities[id] == state) return;

    kDebug() << "Set the state of" << id << "to" << state;

    /**
     * Treating 'Starting' as 'Running', and 'Stopping' as 'Stopped'
     * as far as the config file is concerned
     */
    bool configNeedsUpdating = ((activities[id] & 4) != (state & 4));

    activities[id] = state;

    switch (state) {
        case ActivityManager::Running:
            kDebug() << "sending ActivityStarted signal";
            emit q->ActivityStarted(id);
            break;

        case ActivityManager::Stopped:
            kDebug() << "sending ActivityStopped signal";
            emit q->ActivityStopped(id);
            break;

        default:
            break;
    }

    kDebug() << "sending ActivityStateChanged signal";
    emit q->ActivityStateChanged(id, state);

    if (configNeedsUpdating) {
        mainConfig().writeEntry("runningActivities",
                activities.keys(ActivityManager::Running) +
                activities.keys(ActivityManager::Starting));
        scheduleConfigSync();
    }
}

KConfigGroup ActivityManagerPrivate::activitiesConfig()
{
    return KConfigGroup(&config, "activities");
}

KConfigGroup ActivityManagerPrivate::mainConfig()
{
    return KConfigGroup(&config, "main");
}

void ActivityManagerPrivate::ensureCurrentActivityIsRunning()
{
    QStringList runningActivities = q->ListActivities(ActivityManager::Running);

    if (!runningActivities.contains(currentActivity)) {
        if (runningActivities.size() > 0) {
            setCurrentActivity(runningActivities.first());
        } else {
            kDebug() << "there are no running activities! eek!";
        }
    }
}

bool ActivityManagerPrivate::setCurrentActivity(const QString & id)
{
    kDebug() << id;
    if (id.isEmpty()) {
        currentActivity.clear();

    } else {
        if (!activities.contains(id)) {
            return false;
        }

        if (currentActivity != id) {
            kDebug() << "registering the events";
            // Closing the previous activity:
            if (!currentActivity.isEmpty()) {
                q->RegisterResourceEvent(
                        "kactivitymanagerd", 0,
                        "activities://" + currentActivity,
                        Event::Closed, Event::User
                    );
            }

            q->RegisterResourceEvent(
                    "kactivitymanagerd", 0,
                    "activities://" + id,
                    Event::Accessed, Event::User
                );
            q->RegisterResourceEvent(
                    "kactivitymanagerd", 0,
                    "activities://" + id,
                    Event::Opened, Event::User
                );
        }

        q->StartActivity(id);

        currentActivity = id;
        mainConfig().writeEntry("currentActivity", id);

        scheduleConfigSync();
    }

    kDebug() << (void*) SharedInfo::self() << "Rankings << shared info";
    SharedInfo::self()->setCurrentActivity(id);
    emit q->CurrentActivityChanged(id);
    return true;
}

QString ActivityManagerPrivate::activityName(const QString & id)
{
    return activitiesConfig().readEntry(id, QString());
}

void ActivityManagerPrivate::scheduleConfigSync()
{
    if (!configSyncTimer.isActive()) {
        configSyncTimer.start();
    }
}

void ActivityManagerPrivate::configSync()
{
    configSyncTimer.stop();
    config.sync();
}

#ifdef HAVE_NEPOMUK

Nepomuk::Resource ActivityManagerPrivate::activityResource(const QString & id)
{
    kDebug() << "testing for nepomuk";

    if (nepomukInitialized()) {
        return Nepomuk::Resource(KUrl(ACTIVITIES_PROTOCOL + id));
    } else {
        return Nepomuk::Resource();
    }
}

/* lazy init of nepomuk */
bool ActivityManagerPrivate::nepomukInitialized()
{
    if (m_nepomukInitCalled) return
        Nepomuk::ResourceManager::instance()->initialized();

    m_nepomukInitCalled = true;

    connect(Nepomuk::ResourceManager::instance(), SIGNAL(nepomukSystemStarted()), this, SLOT(backstoreAvailable()));

    return (Nepomuk::ResourceManager::instance()->init() == 0);
}

void ActivityManagerPrivate::backstoreAvailable()
{
    //emit q->BackstoreAvailable();
    //kick the icons, so that clients don't need to know that they depend on nepomuk
    for (QHash<QString, ActivityManager::State>::const_iterator i = activities.constBegin();
         i != activities.constEnd(); ++i) {
        emit q->ActivityChanged(i.key());
    }
}

#else // HAVE_NEPOMUK

void ActivityManagerPrivate::backstoreAvailable()
{
}

#endif // HAVE_NEPOMUK

// Main

ActivityManager::ActivityManager()
    : d(new ActivityManagerPrivate(this,
            SharedInfo::self()->m_windows,
            SharedInfo::self()->m_resources))
{

    QDBusConnection dbus = QDBusConnection::sessionBus();
    new ActivityManagerAdaptor(this);
    dbus.registerObject("/ActivityManager", this);

    // TODO: Sync activities in nepomuk with currently existing ones
    // but later, when we are sure nepomuk is running

    // ensureCurrentActivityIsRunning();

    KCrash::setFlags(KCrash::AutoRestart);

    EventProcessor::self();

    kDebug() << "RegisterResourceEvent open" << d->currentActivity;
    RegisterResourceEvent(
            "kactivitymanagerd", 0,
            "activities://" + d->currentActivity,
            Event::Accessed, Event::User
        );
    RegisterResourceEvent(
            "kactivitymanagerd", 0,
            "activities://" + d->currentActivity,
            Event::Opened, Event::User
        );

}

ActivityManager::~ActivityManager()
{
    kDebug() << "RegisterResourceEvent close" << d->currentActivity;
    RegisterResourceEvent(
            "kactivitymanagerd", 0,
            "activities://" + d->currentActivity,
            Event::Closed, Event::User
        );
    delete d;
}

void ActivityManager::Start()
{
    // doing absolutely nothing
}

void ActivityManager::Stop()
{
    d->configSync();
    QCoreApplication::quit();
}

bool ActivityManager::IsBackstoreAvailable() const
{
    return NEPOMUK_RUNNING;
}


// workspace activities control

QString ActivityManager::CurrentActivity() const
{
    return d->currentActivity;
}

bool ActivityManager::SetCurrentActivity(const QString & id)
{
    kDebug() << id;

    if (id.isEmpty()) {
        return false;
    }

    return d->setCurrentActivity(id);
}

QString ActivityManager::AddActivity(const QString & name)
{
    kDebug() << name;

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

    d->configSync();
    return id;
}

void ActivityManager::RemoveActivity(const QString & id)
{
    kDebug() << id;

    if (d->activities.size() < 2 ||
            !d->activities.contains(id)) {
        return;
    }

    // If the activity is running, stash it
    StopActivity(id);

    d->setActivityState(id, Invalid);

    // Removing the activity
    d->activities.remove(id);
    d->activitiesConfig().deleteEntry(id);

    // If the removed activity was the current one,
    // set another activity as current
    if (d->currentActivity == id) {
        d->ensureCurrentActivityIsRunning();
    }

    if (d->transitioningActivity == id) {
        //very unlikely, but perhaps not impossible
        //but it being deleted doesn't mean ksmserver is un-busy..
        //in fact, I'm not quite sure what would happen.... FIXME
        //so I'll just add some output to warn that it happened.
        kDebug() << "deleting activity in transition. watch out!";
    }

    emit ActivityRemoved(id);
    d->configSync();
}

void ActivityManager::StartActivity(const QString & id)
{
    kDebug() << id;

    if (!d->activities.contains(id) ||
            d->activities[id] != Stopped) {
        return;
    }

    if (!d->transitioningActivity.isEmpty()) {
        kDebug() << "busy!!";
        //TODO: implement a queue instead
        return;
    }

    d->transitioningActivity = id;
    d->setActivityState(id, Starting);

    //ugly hack to avoid dbus deadlocks
    QMetaObject::invokeMethod(d, "reallyStartActivity", Qt::QueuedConnection, Q_ARG(QString, id));
}

void ActivityManagerPrivate::reallyStartActivity(const QString & id)
{
    bool called = false;
    // start the starting :)
    if (haveSessions) {
        QDBusInterface kwin("org.kde.kwin", "/KWin", "org.kde.KWin");
        if (kwin.isValid()) {
            QDBusMessage reply = kwin.call("startActivity", id);
            if (reply.type() == QDBusMessage::ErrorMessage) {
                kDebug() << "dbus error:" << reply.errorMessage();
            } else {
                QList<QVariant> ret = reply.arguments();
                if (ret.length() == 1 && ret.first().toBool()) {
                    called = true;
                } else {
                    kDebug() << "call returned false; probably ksmserver is busy";
                    setActivityState(transitioningActivity, ActivityManager::Stopped);
                    transitioningActivity.clear();
                    return; //assume we're mid-logout and just don't touch anything
                }
            }
        } else {
            kDebug() << "couldn't get kwin interface";
        }
    }

    if (!called) {
        //maybe they use compiz?
        //go ahead without the session
        startCompleted();
    }
    configSync(); //force immediate sync
}

void ActivityManagerPrivate::startCompleted()
{
    if (transitioningActivity.isEmpty()) {
        kDebug() << "huh?";
        return;
    }
    setActivityState(transitioningActivity, ActivityManager::Running);
    transitioningActivity.clear();
}

void ActivityManager::StopActivity(const QString & id)
{
    kDebug() << id;

    if (!d->activities.contains(id) ||
            d->activities[id] == Stopped) {
        return;
    }

    if (!d->transitioningActivity.isEmpty()) {
        kDebug() << "busy!!";
        //TODO: implement a queue instead
        return;
    }

    d->transitioningActivity = id;
    d->setActivityState(id, Stopping);

    //ugly hack to avoid dbus deadlocks
    QMetaObject::invokeMethod(d, "reallyStopActivity", Qt::QueuedConnection, Q_ARG(QString, id));
}

void ActivityManagerPrivate::reallyStopActivity(const QString & id)
{
    bool called = false;
    // start the stopping :)
    if (haveSessions) {
        QDBusInterface kwin("org.kde.kwin", "/KWin", "org.kde.KWin");
        if (kwin.isValid()) {
            QDBusMessage reply = kwin.call("stopActivity", id);
            if (reply.type() == QDBusMessage::ErrorMessage) {
                kDebug() << "dbus error:" << reply.errorMessage();
            } else {
                QList<QVariant> ret = reply.arguments();
                if (ret.length() == 1 && ret.first().toBool()) {
                    called = true;
                } else {
                    kDebug() << "call returned false; probably ksmserver is busy";
                    stopCancelled();
                    return; //assume we're mid-logout and just don't touch anything
                }
            }
        } else {
            kDebug() << "couldn't get kwin interface";
        }
    }

    if (!called) {
        //maybe they use compiz?
        //go ahead without the session
        stopCompleted();
    }
}

void ActivityManagerPrivate::stopCompleted()
{
    if (transitioningActivity.isEmpty()) {
        kDebug() << "huh?";
        return;
    }
    setActivityState(transitioningActivity, ActivityManager::Stopped);
    if (currentActivity == transitioningActivity) {
        ensureCurrentActivityIsRunning();
    }
    transitioningActivity.clear();
    configSync(); //force immediate sync
}

void ActivityManagerPrivate::stopCancelled()
{
    if (transitioningActivity.isEmpty()) {
        kDebug() << "huh?";
        return;
    }
    setActivityState(transitioningActivity, ActivityManager::Running);
    transitioningActivity.clear();
}

int ActivityManager::ActivityState(const QString & id) const
{
    //kDebug() << id << "- is it in" << d->activities << "?";
    if (!d->activities.contains(id)) {
        return Invalid;
    } else {
        kDebug() << "state of" << id << "is" << d->activities[id];
        return d->activities[id];
    }
}

QStringList ActivityManager::ListActivities() const
{
    return d->activities.keys();
}

QStringList ActivityManager::ListActivities(int state) const
{
    return d->activities.keys((State)state);
}

QString ActivityManager::ActivityName(const QString & id) const
{
    return d->activityName(id);
}

void ActivityManager::SetActivityName(const QString & id, const QString & name)
{
    kDebug() << id << name;

    if (!d->activities.contains(id)) {
        return;
    }

    d->activitiesConfig().writeEntry(id, name);

#ifdef HAVE_NEPOMUK
    if (NEPOMUK_RUNNING) {
        d->activityResource(id).setLabel(name);
    }
#endif

    d->scheduleConfigSync();

    kDebug() << "emit ActivityChanged" << id;
    emit ActivityChanged(id);
}

QString ActivityManager::ActivityDescription(const QString & id) const
{
    if (!NEPOMUK_RUNNING || !d->activities.contains(id)) {
        return QString();
    }

#ifdef HAVE_NEPOMUK
    return d->activityResource(id).description();
#endif
}

void ActivityManager::SetActivityDescription(const QString & id, const QString & description)
{
    kDebug() << id << description;

    if (!NEPOMUK_RUNNING || !d->activities.contains(id)) {
        return;
    }

#ifdef HAVE_NEPOMUK
    d->activityResource(id).setDescription(description);
#endif

    kDebug() << "emit ActivityChanged" << id;
    emit ActivityChanged(id);
}

QString ActivityManager::ActivityIcon(const QString & id) const
{
    if (!NEPOMUK_RUNNING || !d->activities.contains(id)) {
        return QString();
    }

#ifdef HAVE_NEPOMUK
    QStringList symbols = d->activityResource(id).symbols();

    if (symbols.isEmpty()) {
        return QString();
    } else {
        return symbols.first();
    }
#else
    return QString();
#endif
}

void ActivityManager::SetActivityIcon(const QString & id, const QString & icon)
{
    kDebug() << id << icon;

    if (!NEPOMUK_RUNNING || !d->activities.contains(id)) {
        return;
    }

#ifdef HAVE_NEPOMUK
    d->activityResource(id).setSymbols(QStringList() << icon);

    kDebug() << "emit ActivityChanged" << id;
    emit ActivityChanged(id);
#endif
}


// Resource related mothods
void ActivityManager::RegisterResourceEvent(const QString & application, uint _windowId,
        const QString & uri, uint event, uint reason)
{
    if (event > Event::LastEventType || reason > Event::LastEventReason)
        return;

    KUrl kuri(uri);
    WId windowId = (WId) _windowId;

    kDebug() << "New event on the horizon" << application << _windowId << windowId << event << Event::Opened;

#ifdef HAVE_NEPOMUK
    if (uri.startsWith("nepomuk:")) {
        Nepomuk::Resource resource(kuri);

        if (resource.hasProperty(Nepomuk::Vocabulary::NIE::url())) {
            kuri = resource.property(Nepomuk::Vocabulary::NIE::url()).toUrl();
            kDebug() << "Passing real url" << kuri;
        } else {
            kWarning() << "Passing nepomuk:// url" << kuri;
        }
    }
#endif

    if (event == Event::Opened) {

        kDebug() << "Saving the open event for the window" << windowId;

        d->windows[windowId].resources << kuri;
        d->resources[kuri].activities << CurrentActivity();

        kDebug() << d->windows.keys();

    } else if (event == Event::Closed) {

        // TODO: Remove from d->resources if needed
        d->windows.remove(windowId);

    }

    EventProcessor::self()->addEvent(application, windowId,
            kuri.url(), (Event::Type) event, (Event::Reason) reason);

}

void ActivityManager::RegisterResourceMimeType(const QString & uri, const QString & mimetype)
{
    KUrl kuri(uri);

    d->resources[kuri].mimetype = mimetype;
}

void ActivityManager::RegisterResourceTitle(const QString & uri, const QString & title)
{
    KUrl kuri(uri);

    d->resources[kuri].title = title;
}

void ActivityManager::LinkResourceToActivity(const QString & uri, const QString & activity)
{
#ifdef HAVE_NEPOMUK
    if (!d->nepomukInitialized()) return;

    kDebug() << "Linking" << uri << "to" << activity << CurrentActivity();

    Nepomuk::Resource(KUrl(uri)).
        addIsRelated(d->activityResource(
            activity.isEmpty() ?
                CurrentActivity() : activity
            )
        );

#endif
}

// void ActivityManager::UnlinkResourceFromActivity(const QString & uri, const QString & activity)
// {
// #ifdef HAVE_NEPOMUK
//     if (!d->nepomukInitialized()) return;
//
//     Nepomuk::Resource(KUrl(uri)).
//         addIsRelated(d->activityResource(
//             activity.isEmpty() ?
//                 CurrentActivity() : activity
//             )
//         );
//
// #endif
// }

// static
ActivityManager * ActivityManager::self()
{
    return static_cast<ActivityManager*>(kapp);
}


