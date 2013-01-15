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

#ifndef ACTIVITIES_P_H
#define ACTIVITIES_P_H

#include <QString>
#include <QTimer>

#include <KConfig>
#include <KConfigGroup>

#include "Activities.h"

class KSMServer;

class QDBusInterface;
class KJob;

class Activities::Private: public QObject {
    Q_OBJECT

public:
    Private(Activities * parent);
    ~Private();

    // Loads the last activity
    // the user has used
    void loadLastActivity();

    // If the current activity is not running,
    // make some other activity current
    void ensureCurrentActivityIsRunning();

public Q_SLOTS:
    bool setCurrentActivity(const QString & activity);

public:
    void setActivityState(const QString & activity, Activities::State state);
    QHash < QString, Activities::State > activities;

    // Current activity
    QString currentActivity;

    // Configuration
    QTimer configSyncTimer;
    KConfig config;

    // Interface to the session management
    KSMServer * ksmserver;

public:
    KConfigGroup activitiesConfig();
    KConfigGroup activityIconsConfig();
    KConfigGroup mainConfig();
    QString activityName(const QString & activity);
    QString activityIcon(const QString & activity);


public Q_SLOTS:
    // Schedules config syncing to be done after
    // a predefined time interval
    // if soon == true, the syncing is performed
    // after a few seconds, otherwise a few minutes
    void scheduleConfigSync(const bool soon = false);

    // Immediately syncs the configuration file
    void configSync();

    void removeActivity(const QString & activity);
    void activitySessionStateChanged(const QString & activity, int state);

    void emitCurrentActivityChanged(const QString & activity);

private:
    Activities * const q;
};

#endif // ACTIVITIES_P_H

