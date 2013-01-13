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

#ifndef ACTIVITIES_MANAGER_P
#define ACTIVITIES_MANAGER_P

#include <common/dbus/org.kde.ActivityManager.Activities.h>

#include "activities_interface.h"
#include "resources_interface.h"
#include "resources_linking_interface.h"
#include "features_interface.h"

#include <QDBusServiceWatcher>

namespace Service = org::kde::ActivityManager;

namespace KActivities {

class Manager: public QObject {
    Q_OBJECT

public:
    static Manager * self();

    static bool isServicePresent();

    static Service::Activities       * activities();
    static Service::Resources        * resources();
    static Service::ResourcesLinking * resourcesLinking();
    static Service::Features         * features();

public Q_SLOTS:
    void serviceOwnerChanged(const QString & serviceName, const QString & oldOwner, const QString & newOwner);

Q_SIGNALS:
    void servicePresenceChanged(bool present);

private:
    Manager();

    QDBusServiceWatcher m_watcher;

    static Manager * s_instance;

    Service::Activities        * const m_activities;
    Service::Resources         * const m_resources;
    Service::ResourcesLinking  * const m_resourcesLinking;
    Service::Features          * const m_features;
};

} // namespace KActivities

#endif // ACTIVITIES_MANAGER_P

