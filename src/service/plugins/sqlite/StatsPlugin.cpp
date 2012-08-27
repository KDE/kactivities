/*
 *   Copyright (C) 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include <config-features.h>

#include "StatsPlugin.h"
#include "ResourceScoreMaintainer.h"

#include "../../Event.h"

#ifdef HAVE_NEPOMUK
#include "kao.h"
#endif

#include <KDebug>

#include "Rankings.h"
#include "DatabaseConnection.h"

#include <utils/val.h>

StatsPlugin * StatsPlugin::s_instance = nullptr;

StatsPlugin::StatsPlugin(QObject *parent, const QVariantList & args)
    : Plugin(parent)
{
    Q_UNUSED(args)
    s_instance = this;
}

bool StatsPlugin::init(const QHash < QString, QObject * > & modules)
{
    m_activities = modules["activities"];
    m_resources = modules["resources"];

    setName("org.kde.kactivitymanager.sqlite");

    DatabaseConnection::self();
    Rankings::init(this);

    connect(m_activities, SIGNAL(CurrentActivityChanged(QString)),
            Rankings::self(), SLOT(setCurrentActivity(QString)));

    connect(m_resources, SIGNAL(ProcessedResourceEvents(EventList)),
            this, SLOT(addEvents(EventList)));

    return true;
}

StatsPlugin * StatsPlugin::self()
{
    return s_instance;
}

QString StatsPlugin::currentActivity() const
{
    return Plugin::callOn <QString, Qt::DirectConnection> (m_activities, "CurrentActivity", "QString");
}

void StatsPlugin::addEvents(const EventList & events)
{
    for (int i = 0; i < events.size(); i++) {
        val & event = events[i];
        val currentActivity = Plugin::callOn <QString, Qt::DirectConnection> (m_activities, "CurrentActivity", "QString");

        switch (event.type) {
            case Event::Accessed:
                DatabaseConnection::self()->openDesktopEvent(
                        currentActivity, event.application, event.uri, event.timestamp, event.timestamp);
                ResourceScoreMaintainer::self()->processResource(event.uri, event.application);

                break;

            case Event::Opened:
                DatabaseConnection::self()->openDesktopEvent(
                        currentActivity, event.application, event.uri, event.timestamp);

                break;

            case Event::Closed:
                DatabaseConnection::self()->closeDesktopEvent(
                        currentActivity, event.application, event.uri, event.timestamp);
                ResourceScoreMaintainer::self()->processResource(event.uri, event.application);

                break;

            case Event::UserEventType:
                ResourceScoreMaintainer::self()->processResource(event.uri, event.application);
                break;

            default:
                // Nothing yet
                // TODO: Add focus and modification
                break;
        }
    }
}

KAMD_EXPORT_PLUGIN(StatsPlugin, "activitymanger_plugin_sqlite")
