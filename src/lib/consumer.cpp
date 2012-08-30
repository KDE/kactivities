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

#include <kdebug.h>

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

void ConsumerPrivate::currentActivityCallFinished(QDBusPendingCallWatcher * call)
{
    QDBusPendingReply <QString> reply = * call;

    currentActivity = reply.isError()
        ? nulluuid
        : reply.argumentAt<0>();

    currentActivityCallWatcher = 0;
    call->deleteLater();
}

void ConsumerPrivate::listAllActivitiesCallFinished(QDBusPendingCallWatcher * call)
{
    QDBusPendingReply <QStringList> reply = * call;

    if (!reply.isError()) {
        allActivities = reply.argumentAt<0>();
    } else {
        allActivities.clear();
    }

    listAllActivitiesCallWatcher = 0;
    call->deleteLater();
}

void ConsumerPrivate::listRunningActivitiesCallFinished(QDBusPendingCallWatcher * call)
{
    QDBusPendingReply <QStringList> reply = * call;

    if (!reply.isError()) {
        runningActivities = reply.argumentAt<0>();
    } else {
        runningActivities.clear();
    }

    listRunningActivitiesCallWatcher = 0;
    call->deleteLater();
}

ConsumerPrivate::ConsumerPrivate()
    : currentActivityCallWatcher(0),
      listAllActivitiesCallWatcher(0),
      listRunningActivitiesCallWatcher(0)
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
    // Getting the current activity
    const QDBusPendingCall & currentActivityCall = Manager::activities()->CurrentActivity();
    currentActivityCallWatcher = new QDBusPendingCallWatcher(currentActivityCall, this);

    connect(currentActivityCallWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(currentActivityCallFinished(QDBusPendingCallWatcher*)));

    // Getting the list of activities
    const QDBusPendingCall & listAllActivitiesCall = Manager::activities()->ListActivities();
    listAllActivitiesCallWatcher = new QDBusPendingCallWatcher(listAllActivitiesCall, this);

    connect(listAllActivitiesCallWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(listAllActivitiesCallFinished(QDBusPendingCallWatcher*)));

    // Getting the list of running activities
    const QDBusPendingCall & listRunningActivitiesCall = Manager::activities()->ListActivities(Info::Running);
    listRunningActivitiesCallWatcher = new QDBusPendingCallWatcher(listRunningActivitiesCall, this);

    connect(listRunningActivitiesCallWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(listRunningActivitiesCallFinished(QDBusPendingCallWatcher*)));
}

void ConsumerPrivate::setCurrentActivity(const QString & activity)
{
    kDebug() << "current activity is" << activity;
    currentActivity = activity;

    emit currentActivityChanged(activity);
}

void ConsumerPrivate::addActivity(const QString & activity)
{
    kDebug() << "new activity added" << activity;
    if (!allActivities.contains(activity)) {
        allActivities << activity;
        runningActivities << activity;
    }

    emit activityAdded(activity);
}

void ConsumerPrivate::removeActivity(const QString & activity)
{
    kDebug() << "activity removed added" << activity;
    allActivities.removeAll(activity);
    runningActivities.removeAll(activity);

    emit activityRemoved(activity);
}

void ConsumerPrivate::setActivityState(const QString & activity, int state)
{
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

// macro defines a shorthand for validating and returning a d-bus result
// @param TYPE type of the result
// @param METHOD invocation of the d-bus method
// @param DEFAULT value to be used if the reply was not valid
#define KACTIVITYCONSUMER_DBUS_RETURN(TYPE, METHOD, DEFAULT)  \
    if (!Manager::isServicePresent()) return DEFAULT;         \
                                                              \
    QDBusReply < TYPE > dbusReply = METHOD;                   \
    if (dbusReply.isValid()) {                                \
        return dbusReply.value();                             \
    } else {                                                  \
        kDebug() << "d-bus reply was invalid"                 \
                 << dbusReply.value()                         \
                 << dbusReply.error();                        \
        return DEFAULT;                                       \
    }

static inline void waitForCallFinished(QDBusPendingCallWatcher * watcher)
{
    if (watcher) {
        watcher->waitForFinished();
    }
}

QString Consumer::currentActivity() const
{
    if (!Manager::isServicePresent()) return nulluuid;

    waitForCallFinished(d->currentActivityCallWatcher);

    kDebug() << "Returning the current activity" << d->currentActivity;

    return d->currentActivity;
}

QStringList Consumer::listActivities(Info::State state) const
{
    if (state == Info::Running) {
        if (!Manager::isServicePresent()) return QStringList(nulluuid);

        waitForCallFinished(d->listRunningActivitiesCallWatcher);

        kDebug() << "Returning the running activities" << d->runningActivities;

        return d->runningActivities;
    }

    KACTIVITYCONSUMER_DBUS_RETURN(
        QStringList, Manager::activities()->ListActivities(state), QStringList() );
}

QStringList Consumer::listActivities() const
{
    waitForCallFinished(d->listAllActivitiesCallWatcher);

    kDebug() << "Returning the activity list" <<
        d->allActivities;

    return d->allActivities;
}

void Consumer::linkResourceToActivity(const QUrl & uri, const QString & activityId)
{
    if (Manager::isServicePresent())
        Manager::resources()->LinkResourceToActivity(uri.toString(), activityId);
}

void Consumer::unlinkResourceFromActivity(const QUrl & uri, const QString & activityId)
{
    if (Manager::isServicePresent())
        Manager::resources()->UnlinkResourceFromActivity(uri.toString(), activityId);
}

bool Consumer::isResourceLinkedToActivity(const QUrl & uri, const QString & activityId) const
{
    KACTIVITYCONSUMER_DBUS_RETURN(
        bool, Manager::resources()->IsResourceLinkedToActivity(uri.toString(), activityId), false );

}

#undef KACTIVITYCONSUMER_DBUS_RETURN

Consumer::ServiceStatus Consumer::serviceStatus()
{
    if (!Manager::isServicePresent()) {
        return NotRunning;
    }

    return Running;
}

} // namespace KActivities

