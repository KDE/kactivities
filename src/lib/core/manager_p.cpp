/*
 *   Copyright (C) 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "manager_p.h"

#include <QDBusConnection>
#include "debug_p.h"

// #include <ktoolinvocation.h>
#include <kdbusconnectionpool.h>

namespace KActivities {

Manager *Manager::s_instance = Q_NULLPTR;

#define ACTIVITY_MANAGER_DBUS_PATH \
    QStringLiteral("org.kde.ActivityManager")
#define ACTIVITY_MANAGER_DBUS_OBJECT(A) \
    QStringLiteral("/ActivityManager" A)

Manager::Manager()
    : QObject()
    , m_activities(
            new org::kde::ActivityManager::Activities(
                ACTIVITY_MANAGER_DBUS_PATH,
                ACTIVITY_MANAGER_DBUS_OBJECT("/Activities"),
                KDBusConnectionPool::threadConnection(),
                this)
            )
    , m_resources(
            new org::kde::ActivityManager::Resources(
                ACTIVITY_MANAGER_DBUS_PATH,
                ACTIVITY_MANAGER_DBUS_OBJECT("/Resources"),
                KDBusConnectionPool::threadConnection(),
                this)
            )
    , m_resourcesLinking(
            new org::kde::ActivityManager::ResourcesLinking(
                ACTIVITY_MANAGER_DBUS_PATH,
                ACTIVITY_MANAGER_DBUS_OBJECT("/Resources/Linking"),
                KDBusConnectionPool::threadConnection(),
                this)
            )
    , m_features(
            new org::kde::ActivityManager::Features(
                ACTIVITY_MANAGER_DBUS_PATH,
                ACTIVITY_MANAGER_DBUS_OBJECT("/Features"),
                KDBusConnectionPool::threadConnection(),
                this)
            )
{
    connect(&m_watcher, &QDBusServiceWatcher::serviceOwnerChanged,
            this, &Manager::serviceOwnerChanged);
}

Manager *Manager::self()
{
    qDebug() << "MANAGERIAL STUFF";
    qCDebug(KAMD_CORELIB) << "MANAGERIAL STUFF";

    if (!s_instance) {
        // check if the activity manager is already running
        if (!isServiceRunning()) {

            // not running, trying to launch it
            // QString error;

            // int ret = KToolInvocation::startServiceByDesktopPath("kactivitymanagerd.desktop", QStringList(), &error);
            // if (ret > 0) {
            //     qCDebug(KAMD_CORELIB) << "Activity: Couldn't start kactivitymanagerd: " << error << endl;
            // }

            // if (!KDBusConnectionPool::threadConnection().interface()->isServiceRegistered(ACTIVITY_MANAGER_DBUS_PATH)) {
            //     qCDebug(KAMD_CORELIB) << "Activity: The kactivitymanagerd service is still not registered";
            // } else {
            //     qCDebug(KAMD_CORELIB) << "Activity: The kactivitymanagerd service has been registered";
            // }

            #if defined(DEBUG)
            if (!QCoreApplication::property(KAMD_CORELIB ".disableAutostart").toBool()) {
                qCDebug(KAMD_CORELIB) << "Starting the activity manager daemon";
                QProcess::startDetached(QStringLiteral("kactivitymanagerd"));
            }

            #else
            QProcess::startDetached(QStringLiteral("kactivitymanagerd"));
            #endif
        }

        // creating a new instance of the class
        s_instance = new Manager();
    }

    return s_instance;
}

bool Manager::isServiceRunning()
{
    return KDBusConnectionPool::threadConnection().interface()->isServiceRegistered(ACTIVITY_MANAGER_DBUS_PATH);
}

void Manager::serviceOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(oldOwner)

    if (serviceName == ACTIVITY_MANAGER_DBUS_PATH) {
        emit serviceStatusChanged(!newOwner.isEmpty());
    }
}

Service::Activities *Manager::activities()
{
    return self()->m_activities;
}

Service::Resources *Manager::resources()
{
    return self()->m_resources;
}

Service::ResourcesLinking *Manager::resourcesLinking()
{
    return self()->m_resourcesLinking;
}

Service::Features *Manager::features()
{
    return self()->m_features;
}

} // namespace KActivities
