/*
 *   Copyright (C) 2010 - 2016 by Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.
 *   If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ACTIVITIES_MANAGER_P
#define ACTIVITIES_MANAGER_P

#include <common/dbus/org.kde.ActivityManager.Activities.h>

#include "application_interface.h"
#include "activities_interface.h"
#include "resources_interface.h"
#include "resources_linking_interface.h"
#include "features_interface.h"

#include <QDBusServiceWatcher>

namespace Service = org::kde::ActivityManager;

namespace KActivities {

class Manager : public QObject {
    Q_OBJECT

public:
    static Manager *self();

    static bool isServiceRunning();

    static Service::Activities *activities();
    static Service::Resources *resources();
    static Service::ResourcesLinking *resourcesLinking();
    static Service::Features *features();

public Q_SLOTS:
    void serviceOwnerChanged(const QString &serviceName,
                             const QString &oldOwner, const QString &newOwner);

Q_SIGNALS:
    void serviceStatusChanged(bool status);

private:
    Manager();

    QDBusServiceWatcher m_watcher;

    static Manager *s_instance;

    Service::Application *const m_service;
    Service::Activities *const m_activities;
    Service::Resources *const m_resources;
    Service::ResourcesLinking *const m_resourcesLinking;
    Service::Features *const m_features;
    bool m_serviceRunning;

    friend class ManagerInstantiator;
};

} // namespace KActivities

#endif // ACTIVITIES_MANAGER_P
