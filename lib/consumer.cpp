/*
 * Copyright (c) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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
    connect(Manager::self(), SIGNAL(CurrentActivityChanged(const QString &)),
            this, SIGNAL(currentActivityChanged(const QString &)));
    connect(Manager::self(), SIGNAL(ActivityAdded(QString)),
            this, SIGNAL(activityAdded(QString)));
    connect(Manager::self(), SIGNAL(ActivityRemoved(QString)),
            this, SIGNAL(activityRemoved(QString)));

    connect(Manager::self(), SIGNAL(presenceChanged(bool)),
            d, SLOT(setServicePresent(bool)));
    connect(d, SIGNAL(serviceStatusChanged(KActivities::Consumer::ServiceStatus)),
            this, SIGNAL(serviceStatusChanged(KActivities::Consumer::ServiceStatus)));
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
    KACTIVITYCONSUMER_DBUS_RETURN(
        QString, Manager::self()->CurrentActivity(), QString() );
}

QStringList Consumer::listActivities(Info::State state) const
{
    KACTIVITYCONSUMER_DBUS_RETURN(
        QStringList, Manager::self()->ListActivities(state), QStringList() );
}

QStringList Consumer::listActivities() const
{
    KACTIVITYCONSUMER_DBUS_RETURN(
        QStringList, Manager::self()->ListActivities(), QStringList() );
}

void Consumer::linkResourceToActivity(const QUrl & uri, const QString & activityId)
{
    Manager::self()->LinkResourceToActivity(uri.toString(), activityId);
}

void Consumer::unlinkResourceFromActivity(const QUrl & uri, const QString & activityId)
{
    Manager::self()->UnlinkResourceFromActivity(uri.toString(), activityId);
}

bool Consumer::isResourceLinkedToActivity(const QUrl & uri, const QString & activityId) const
{
    KACTIVITYCONSUMER_DBUS_RETURN(
        bool, Manager::self()->IsResourceLinkedToActivity(uri.toString(), activityId), false );

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

