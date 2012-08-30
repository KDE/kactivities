/*
 * Copyright (c) 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef ACTIVITIES_CONSUMER_P_H
#define ACTIVITIES_CONSUMER_P_H

#include "consumer.h"

#include <QSet>

class QDBusPendingCallWatcher;

namespace KActivities {

class ConsumerPrivate: public QObject {
    Q_OBJECT

public:
    static ConsumerPrivate * self(QObject * consumer);
    void free(QObject * consumer);

public Q_SLOTS:
    void setServicePresent(bool present);
    void initializeCachedData();

    void currentActivityCallFinished(QDBusPendingCallWatcher * call);
    void listAllActivitiesCallFinished(QDBusPendingCallWatcher * call);
    void listRunningActivitiesCallFinished(QDBusPendingCallWatcher * call);

    void setCurrentActivity(const QString & activity);
    void addActivity(const QString & activity);
    void removeActivity(const QString & activity);
    void setActivityState(const QString & activity, int state);

Q_SIGNALS:
    void serviceStatusChanged(KActivities::Consumer::ServiceStatus status);

    void currentActivityChanged(const QString & id);
    void activityAdded(const QString & id);
    void activityRemoved(const QString & id);

public:
    QString currentActivity;
    QStringList allActivities;
    QStringList runningActivities;

    QDBusPendingCallWatcher * currentActivityCallWatcher;
    QDBusPendingCallWatcher * listAllActivitiesCallWatcher;
    QDBusPendingCallWatcher * listRunningActivitiesCallWatcher;

    QSet <QObject *> consumers;

private:
    ConsumerPrivate();
    static ConsumerPrivate * s_instance;
};

} // namespace KActivities

#endif // ACTIVITIES_CONSUMER_P_H

