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

#include <QDBusInterface>
#include <KDebug>

void ActivityManagerPrivate::sessionServiceRegistered()
{
    delete ksmserverInterface;
    ksmserverInterface = new QDBusInterface("org.kde.ksmserver", "/KSMServer", "org.kde.KSMServerInterface");
    if (ksmserverInterface->isValid()) {
        ksmserverInterface->setParent(this);
        connect(ksmserverInterface, SIGNAL(subSessionOpened()), this, SLOT(startCompleted()));
        connect(ksmserverInterface, SIGNAL(subSessionClosed()), this, SLOT(stopCompleted()));
        connect(ksmserverInterface, SIGNAL(subSessionCloseCanceled()), this, SLOT(stopCancelled())); //spelling fail :)
    } else {
        delete ksmserverInterface;
        ksmserverInterface = 0;
        kDebug() << "couldn't connect to ksmserver! session stuff won't work";
    }
}

void ActivityManagerPrivate::setActivityState(const QString & id, ActivityManager::State state)
{
    if (activities[id] == state) return;

    // kDebug() << "Set the state of" << id << "to" << state;

    /**
     * Treating 'Starting' as 'Running', and 'Stopping' as 'Stopped'
     * as far as the config file is concerned
     */
    bool configNeedsUpdating = ((activities[id] & 4) != (state & 4));

    activities[id] = state;

    switch (state) {
        case ActivityManager::Running:
            // kDebug() << "sending ActivityStarted signal";
            emit q->ActivityStarted(id);
            break;

        case ActivityManager::Stopped:
            // kDebug() << "sending ActivityStopped signal";
            emit q->ActivityStopped(id);
            break;

        default:
            break;
    }

    // kDebug() << "sending ActivityStateChanged signal";
    emit q->ActivityStateChanged(id, state);

    if (configNeedsUpdating) {
        mainConfig().writeEntry("runningActivities",
                activities.keys(ActivityManager::Running) +
                activities.keys(ActivityManager::Starting));
        scheduleConfigSync();
    }
}

void ActivityManagerPrivate::ensureCurrentActivityIsRunning()
{
    QStringList runningActivities = q->ListActivities(ActivityManager::Running);

    if (!runningActivities.contains(currentActivity)) {
        if (runningActivities.size() > 0) {
            kDebug() << "Somebody called ensureCurrentActivityIsRunning?";
            setCurrentActivity(runningActivities.first());
        } else {
            // kDebug() << "there are no running activities! eek!";
        }
    }
}

// Main

void ActivityManager::StartActivity(const QString & id)
{
    // kDebug() << id;

    if (!d->activities.contains(id) ||
            d->activities[id] != Stopped) {
        return;
    }

    if (!d->transitioningActivity.isEmpty()) {
        // kDebug() << "busy!!";
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
    QDBusInterface kwin("org.kde.kwin", "/KWin", "org.kde.KWin");
    if (kwin.isValid()) {
        QDBusMessage reply = kwin.call("startActivity", id);
        if (reply.type() == QDBusMessage::ErrorMessage) {
            // kDebug() << "dbus error:" << reply.errorMessage();

        } else {
            QList<QVariant> ret = reply.arguments();
            if (ret.length() == 1 && ret.first().toBool()) {
                called = true;
            } else {
                // kDebug() << "call returned false; probably ksmserver is busy";
                setActivityState(transitioningActivity, ActivityManager::Stopped);
                transitioningActivity.clear();
                return; //assume we're mid-logout and just don't touch anything
            }
        }
    } else {
        // kDebug() << "couldn't get kwin interface";
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
        // kDebug() << "huh?";
        return;
    }
    setActivityState(transitioningActivity, ActivityManager::Running);
    transitioningActivity.clear();
}

void ActivityManager::StopActivity(const QString & id)
{
    // kDebug() << id;

    if (!d->activities.contains(id) ||
            d->activities[id] == Stopped) {
        return;
    }

    if (!d->transitioningActivity.isEmpty()) {
        // kDebug() << "busy!!";
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
    QDBusInterface kwin("org.kde.kwin", "/KWin", "org.kde.KWin");
    if (kwin.isValid()) {
        QDBusMessage reply = kwin.call("stopActivity", id);
        if (reply.type() == QDBusMessage::ErrorMessage) {
            // kDebug() << "dbus error:" << reply.errorMessage();

        } else {
            QList<QVariant> ret = reply.arguments();
            if (ret.length() == 1 && ret.first().toBool()) {
                called = true;

            } else {
                // kDebug() << "call returned false; probably ksmserver is busy";
                stopCancelled();
                return; //assume we're mid-logout and just don't touch anything
            }
        }
    } else {
        // kDebug() << "couldn't get kwin interface";
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
        // kDebug() << "huh?";
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
        // kDebug() << "huh?";
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
        // kDebug() << "state of" << id << "is" << d->activities[id];
        return d->activities[id];
    }
}

