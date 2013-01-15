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

#include "consumer.h"
#include "consumer_p.h"
#include "manager_p.h"

#include <QDebug>

namespace KActivities {

static QString nulluuid = "00000000-0000-0000-0000-000000000000";
ConsumerPrivate * ConsumerPrivate::s_instance = 0;

ConsumerPrivate * ConsumerPrivate::self(QObject * consumer)
{
    if (!s_instance) {
        s_instance = new ConsumerPrivate();
    }

    s_instance->consumers << consumer;

    return s_instance;
}

void ConsumerPrivate::free(QObject * consumer)
{
    consumers.remove(consumer);

    if (consumers.isEmpty()) {
        s_instance = 0;
        deleteLater();
    }
}

KAMD_RETRIEVE_REMOTE_VALUE_HANDLER(QString,     ConsumerPrivate, currentActivity, nulluuid)
KAMD_RETRIEVE_REMOTE_VALUE_HANDLER(QStringList, ConsumerPrivate, listActivities, QStringList())
KAMD_RETRIEVE_REMOTE_VALUE_HANDLER(QStringList, ConsumerPrivate, runningActivities, QStringList())

ConsumerPrivate::ConsumerPrivate()
    : currentActivityCallWatcher(0),
      listActivitiesCallWatcher(0),
      runningActivitiesCallWatcher(0)
{
    connect(Manager::activities(), SIGNAL(CurrentActivityChanged(const QString &)),
            this, SLOT(setCurrentActivity(const QString &)));
    connect(Manager::activities(), SIGNAL(ActivityAdded(QString)),
            this, SLOT(addActivity(QString)));
    connect(Manager::activities(), SIGNAL(ActivityRemoved(QString)),
            this, SLOT(removeActivity(QString)));
    connect(Manager::activities(), SIGNAL(ActivityStateChanged(QString,int)),
            this, SLOT(setActivityState(QString, int)));

    connect(Manager::self(), SIGNAL(servicePresenceChanged(bool)),
            this, SLOT(setServicePresent(bool)));

    qDebug() << "We are checking whether the service is present" <<
        Manager::isServicePresent();

    if (Manager::isServicePresent()) {
        initializeCachedData();
    }
}

void ConsumerPrivate::setServicePresent(bool present)
{
    emit serviceStatusChanged(
            present ? Consumer::Running : Consumer::NotRunning
        );

    if (present) {
        initializeCachedData();
    }
}

void ConsumerPrivate::initializeCachedData()
{
    KAMD_RETRIEVE_REMOTE_VALUE(currentActivity, CurrentActivity(), this);
    KAMD_RETRIEVE_REMOTE_VALUE(listActivities, ListActivities(), this);
    KAMD_RETRIEVE_REMOTE_VALUE(runningActivities, ListActivities(Info::Running), this);
}

void ConsumerPrivate::setCurrentActivity(const QString & activity)
{
    qDebug() << "current activity is" << activity;
    currentActivity = activity;

    emit currentActivityChanged(activity);
}

void ConsumerPrivate::addActivity(const QString & activity)
{
    qDebug() << "new activity added" << activity;
    if (!listActivities.contains(activity)) {
        listActivities << activity;
        runningActivities << activity;
    }

    emit activityAdded(activity);
}

void ConsumerPrivate::removeActivity(const QString & activity)
{
    qDebug() << "activity removed added" << activity;
    listActivities.removeAll(activity);
    runningActivities.removeAll(activity);

    emit activityRemoved(activity);
}

void ConsumerPrivate::setActivityState(const QString & activity, int state)
{
    if (!listActivities.contains(activity)) {
         qWarning("trying to alter state of unknown activity!!");
         return; // denied
    }

    if (state == Info::Running) {
        if (!runningActivities.contains(activity))
            runningActivities << activity;

    } else {
        runningActivities.removeAll(activity);
    }
}

Consumer::Consumer(QObject * parent)
    : QObject(parent), d(ConsumerPrivate::self(this))
{
    connect(d, SIGNAL(serviceStatusChanged(KActivities::Consumer::ServiceStatus)),
            this, SIGNAL(serviceStatusChanged(KActivities::Consumer::ServiceStatus)));
    connect(d, SIGNAL(currentActivityChanged(QString)),
            this, SIGNAL(currentActivityChanged(QString)));
    connect(d, SIGNAL(activityAdded(QString)),
            this, SIGNAL(activityAdded(QString)));
    connect(d, SIGNAL(activityRemoved(QString)),
            this, SIGNAL(activityRemoved(QString)));

}

Consumer::~Consumer()
{
    d->free(this);
}

KAMD_REMOTE_VALUE_GETTER(QString, Consumer, currentActivity, nulluuid)
KAMD_REMOTE_VALUE_GETTER(QStringList, Consumer, listActivities, QStringList(nulluuid))

QStringList Consumer::listActivities(Info::State state) const
{
    if (state == Info::Running) {
        if (!Manager::isServicePresent()) return QStringList(nulluuid);

        waitForCallFinished(d->runningActivitiesCallWatcher, &d->runningActivitiesMutex);

        qDebug() << "Returning the running activities" << d->runningActivities;

        return d->runningActivities;
    }

    KAMD_RETRIEVE_REMOTE_VALUE_SYNC(
            QStringList, activities, ListActivities(state), QStringList(nulluuid)
        );
}

void Consumer::linkResourceToActivity(const QUrl & uri, const QString & activity)
{
    if (Manager::isServicePresent())
        Manager::resourcesLinking()->LinkResourceToActivity(uri.toString(), activity);
}

void Consumer::unlinkResourceFromActivity(const QUrl & uri, const QString & activity)
{
    if (Manager::isServicePresent())
        Manager::resourcesLinking()->UnlinkResourceFromActivity(uri.toString(), activity);
}

bool Consumer::isResourceLinkedToActivity(const QUrl & uri, const QString & activity) const
{
    KAMD_RETRIEVE_REMOTE_VALUE_SYNC(
            bool, resourcesLinking, IsResourceLinkedToActivity(uri.toString(), activity), false
        );
}

Consumer::ServiceStatus Consumer::serviceStatus()
{
    if (!Manager::isServicePresent()) {
        return NotRunning;
    }

    return Running;
}

} // namespace KActivities

