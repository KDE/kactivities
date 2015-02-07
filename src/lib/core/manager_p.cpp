/*
 *   Copyright (C) 2010, 2011, 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include <mutex>

#include <QCoreApplication>
#include <QDBusConnection>

#include "debug_p.h"
#include "mainthreadexecutor_p.h"

#include "common/dbus/common.h"

namespace KActivities {

Manager *Manager::s_instance = Q_NULLPTR;

Manager::Manager()
    : QObject()
    , m_watcher(KAMD_DBUS_SERVICE, QDBusConnection::sessionBus())
    , m_activities(new KAMD_DBUS_CLASS_INTERFACE(Activities, Activities, this))
    , m_resources(new KAMD_DBUS_CLASS_INTERFACE(Resources, Resources, this))
    , m_resourcesLinking(new KAMD_DBUS_CLASS_INTERFACE(Resources/Linking, ResourcesLinking, this))
    , m_features(new KAMD_DBUS_CLASS_INTERFACE(Features, Features, this))
{
    connect(&m_watcher, &QDBusServiceWatcher::serviceOwnerChanged,
            this, &Manager::serviceOwnerChanged);
}

Manager *Manager::self()
{
    static std::mutex singleton;
    std::lock_guard<std::mutex> singleton_lock(singleton);

    if (!s_instance) {

        runInMainThread([] () {
            // check if the activity manager is already running
            if (!Manager::isServiceRunning()) {

                #if defined(QT_DEBUG)
                QLoggingCategory::setFilterRules(QStringLiteral("org.kde.kactivities.lib.core.debug=true"));

                qCDebug(KAMD_CORELIB) << "Should we start the daemon?";
                if (!QCoreApplication::instance()
                         ->property("org.kde.KActivities.core.disableAutostart")
                         .toBool()) {
                    qCDebug(KAMD_CORELIB) << "Starting the activity manager daemon";
                    QProcess::startDetached(QStringLiteral("kactivitymanagerd"));
                }

                #else
                QProcess::startDetached(QStringLiteral("kactivitymanagerd"));
                #endif
            }

            // creating a new instance of the class
            Manager::s_instance = new Manager();

        });
    }

    return s_instance;
}

bool Manager::isServiceRunning()
{
    return QDBusConnection::sessionBus().interface()->isServiceRegistered(KAMD_DBUS_SERVICE);
}

void Manager::serviceOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(oldOwner)

    // qDebug() << "Service: " << serviceName;

    if (serviceName == KAMD_DBUS_SERVICE) {
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
