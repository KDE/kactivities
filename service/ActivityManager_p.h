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

#ifndef ACTIVITY_MANAGER_P_H_
#define ACTIVITY_MANAGER_P_H_

#include <QSet>
#include <QString>
#include <QTimer>

#include <KConfig>
#include <KConfigGroup>
#include <KUrl>

#include "ActivityManager.h"
#include "Event.h"
#include "SharedInfo.h"
#include "config-features.h"

#ifdef HAVE_NEPOMUK
    #include <Nepomuk/ResourceManager>
    #include <Nepomuk/Resource>
#endif

#if !defined(HAVE_NEPOMUK) && !defined(Q_CC_MSVC)
    #warning "No Nepomuk, disabling some activity related features"
#endif

class QDBusInterface;

class ActivityManagerPrivate: public QObject {
    Q_OBJECT

public:
    ActivityManagerPrivate(ActivityManager * parent,
            QHash < WId, SharedInfo::WindowData > & _windows,
            QHash < KUrl, SharedInfo::ResourceData > & _resources
        );
    ~ActivityManagerPrivate();

    void addRunningActivity(const QString & id);
    void removeRunningActivity(const QString & id);

    void ensureCurrentActivityIsRunning();
    bool setCurrentActivity(const QString & id);

    void setActivityState(const QString & id, ActivityManager::State state);
    QHash < QString, ActivityManager::State > activities;

    // Current activity
    QString currentActivity;

    //opening/closing activity (ksmserver can only handle one at a time)
    QString transitioningActivity;
    bool haveSessions; //whether ksmserver's available

    // Configuration
    QTimer configSyncTimer;
    KConfig config;

    QHash < WId, SharedInfo::WindowData > & windows;
    QHash < KUrl, SharedInfo::ResourceData > & resources;

public:
    void initConifg();

    KConfigGroup activitiesConfig();
    KConfigGroup mainConfig();
    QString activityName(const QString & id);

#ifdef HAVE_NEPOMUK
    Nepomuk::Resource activityResource(const QString & id);
    bool nepomukInitialized();
    mutable bool m_nepomukInitCalled;
#endif // HAVE_NEPOMUK

public Q_SLOTS:
    void scheduleConfigSync();
    void configSync();
    void windowClosed(WId windowId);
    void activeWindowChanged(WId windowId);

    void startCompleted();
    void stopCompleted();
    void stopCancelled();

    //for avoiding dbus deadlocks
    void reallyStartActivity(const QString & id);
    void reallyStopActivity(const QString & id);

    void backstoreAvailable();

private:
    ActivityManager * const q;
    QDBusInterface *ksmserverInterface; //just keeping it for the signals

};

#endif // ACTIVITY_MANAGER_P_H_

