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

void ConsumerPrivate::setServicePresent(bool present)
{
    emit serviceStatusChanged(
            present ? Consumer::Running : Consumer::NotRunning
        );
}

Consumer::Consumer(QObject * parent)
    : QObject(parent), d(new ConsumerPrivate())
{
    if (!ConsumerPrivateCommon::s_instance) {
        ConsumerPrivateCommon::s_instance = new ConsumerPrivateCommon();
    }

    connect(Manager::activities(), SIGNAL(CurrentActivityChanged(const QString &)),
            this, SIGNAL(currentActivityChanged(const QString &)));
    connect(Manager::activities(), SIGNAL(ActivityAdded(QString)),
            this, SIGNAL(activityAdded(QString)));
    connect(Manager::activities(), SIGNAL(ActivityRemoved(QString)),
            this, SIGNAL(activityRemoved(QString)));

    connect(Manager::activities(), SIGNAL(presenceChanged(bool)),
            d, SLOT(setServicePresent(bool)));
    connect(d, SIGNAL(serviceStatusChanged(KActivities::Consumer::ServiceStatus)),
            this, SIGNAL(serviceStatusChanged(KActivities::Consumer::ServiceStatus)));

    // Getting the current activity
    const QDBusPendingCall & currentActivityCall = Manager::activities()->CurrentActivity();
    QDBusPendingCallWatcher * currentActivityCallWatcher = new QDBusPendingCallWatcher(currentActivityCall, this);

    QObject::connect(currentActivityCallWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            ConsumerPrivateCommon::s_instance, SLOT(currentActivityCallFinished(QDBusPendingCallWatcher*)));

    // Getting the list of activities
    const QDBusPendingCall & listActivitiesCall = Manager::activities()->ListActivities();
    QDBusPendingCallWatcher * listActivitiesCallWatcher = new QDBusPendingCallWatcher(listActivitiesCall, this);

    QObject::connect(listActivitiesCallWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            ConsumerPrivateCommon::s_instance, SLOT(listActivitiesCallFinished(QDBusPendingCallWatcher*)));
}

ConsumerPrivateCommon * ConsumerPrivateCommon::s_instance = 0;

ConsumerPrivateCommon::ConsumerPrivateCommon()
{
    connect(Manager::activities(), SIGNAL(CurrentActivityChanged(const QString &)),
            this, SLOT(currentActivityChanged(const QString &)));
    connect(Manager::activities(), SIGNAL(ActivityAdded(QString)),
            this, SLOT(activityAdded(QString)));
    connect(Manager::activities(), SIGNAL(ActivityRemoved(QString)),
            this, SLOT(activityRemoved(QString)));
}

void ConsumerPrivateCommon::currentActivityCallFinished(QDBusPendingCallWatcher *call)
{
    if (!currentActivity.isEmpty()) return;

    QDBusPendingReply <QString> reply = *call;

    if (!reply.isError()) {
        currentActivity = reply.argumentAt<0>();
        kDebug() << currentActivity;
    }

    call->deleteLater();
}

void ConsumerPrivateCommon::listActivitiesCallFinished(QDBusPendingCallWatcher *call)
{
    if (!listActivities.isEmpty()) return;

    QDBusPendingReply <QStringList> reply = *call;

    if (!reply.isError()) {
        listActivities = reply.argumentAt<0>();
        kDebug() << listActivities;
    }

    call->deleteLater();
}

void ConsumerPrivateCommon::currentActivityChanged(const QString & activity)
{
    kDebug() << "current activity is" << activity;
    currentActivity = activity;
}

void ConsumerPrivateCommon::activityAdded(const QString & activity)
{
    kDebug() << "new activity added" << activity;
    if (!listActivities.contains(activity)) {
        listActivities << activity;
    }
}

void ConsumerPrivateCommon::activityRemoved(const QString & activity)
{
    kDebug() << "activity removed added" << activity;
    if (listActivities.contains(activity)) {
        listActivities.removeAll(activity);
    }
}


Consumer::~Consumer()
{
    delete d;
}

// macro defines a shorthand for validating and returning a d-bus result
// @param TYPE type of the result
// @param METHOD invocation of the d-bus method
// @param DEFAULT value to be used if the reply was not valid
#define KACTIVITYCONSUMER_DBUS_RETURN(TYPE, METHOD, DEFAULT)  \
    QDBusReply < TYPE > dbusReply = METHOD;                   \
    if (dbusReply.isValid()) {                                \
        return dbusReply.value();                             \
    } else {                                                  \
        kDebug() << "d-bus reply was invalid"                 \
                 << dbusReply.value()                         \
                 << dbusReply.error();                        \
        return DEFAULT;                                       \
    }

QString Consumer::currentActivity() const
{
    if (ConsumerPrivateCommon::s_instance->currentActivity.isEmpty()) {
        KACTIVITYCONSUMER_DBUS_RETURN(
            QString, Manager::activities()->CurrentActivity(), QString() );
    } else {
        return ConsumerPrivateCommon::s_instance->currentActivity;
    }
}

QStringList Consumer::listActivities(Info::State state) const
{
    KACTIVITYCONSUMER_DBUS_RETURN(
        QStringList, Manager::activities()->ListActivities(state), QStringList() );
}

QStringList Consumer::listActivities() const
{
    if (ConsumerPrivateCommon::s_instance->listActivities.isEmpty()) {
        KACTIVITYCONSUMER_DBUS_RETURN(
            QStringList, Manager::activities()->ListActivities(), QStringList() );
    } else {
        return ConsumerPrivateCommon::s_instance->listActivities;
    }
}

void Consumer::linkResourceToActivity(const QUrl & uri, const QString & activityId)
{
    Manager::resources()->LinkResourceToActivity(uri.toString(), activityId);
}

void Consumer::unlinkResourceFromActivity(const QUrl & uri, const QString & activityId)
{
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
    if (!Manager::isActivityServiceRunning()) {
        return NotRunning;
    }

    return Running;
}

} // namespace KActivities

