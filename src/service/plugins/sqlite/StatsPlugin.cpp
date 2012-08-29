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

#include <QFileSystemWatcher>
#include <KDebug>
#include <KStandardDirs>

#include "Rankings.h"
#include "DatabaseConnection.h"

#include <utils/nullptr.h>
#include <utils/val.h>

StatsPlugin * StatsPlugin::s_instance = nullptr;

StatsPlugin::StatsPlugin(QObject *parent, const QVariantList & args)
    : Plugin(parent),
      m_rankings(nullptr),
      m_activities(nullptr),
      m_resources(nullptr),
      m_configWatcher(nullptr)
{
    Q_UNUSED(args)
    s_instance = this;
}

bool StatsPlugin::init(const QHash < QString, QObject * > & modules)
{
    m_activities = modules["activities"];
    m_resources = modules["resources"];

    setName("org.kde.kactivitymanager.resourcescoring");

    DatabaseConnection::self();
    Rankings::init(this);

    connect(m_activities, SIGNAL(CurrentActivityChanged(QString)),
            Rankings::self(), SLOT(setCurrentActivity(QString)));

    connect(m_resources, SIGNAL(ProcessedResourceEvents(EventList)),
            this, SLOT(addEvents(EventList)));

    loadConfiguration();

    return true;
}

void StatsPlugin::loadConfiguration()
{
    config().config()->reparseConfiguration();
    static val configFile = KStandardDirs::locateLocal("config", "activitymanager-pluginsrc");

    if (m_configWatcher) {
        // When saving a config file, KConfig deletes the old,
        // and creates the new one, so the watcher stops watching
        m_configWatcher->addPath(configFile);

    } else {
        m_configWatcher = new QFileSystemWatcher(QStringList() << configFile, this);

        connect(m_configWatcher, SIGNAL(fileChanged(QString)),
                this, SLOT(loadConfiguration()));
        connect(m_activities, SIGNAL(CurrentActivityChanged(QString)),
                this, SLOT(loadConfiguration()));
    }

    // TODO: Ignore encrypted
    // TODO: Delete old events, as per configuration
    m_blockedByDefault = config().readEntry("blocked-by-default", false);
    m_blockAll = false;
    m_whatToRemember = (WhatToRemember)config().readEntry("what-to-remember", (int)AllApplications);

    m_apps.clear();

    if (m_whatToRemember == SpecificApplications) {
        m_apps = config().readEntry(
                m_blockedByDefault ? "allowed-applications" : "blocked-applications",
                QStringList()
            ).toSet();
    }

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
    if (m_blockAll || m_whatToRemember == NoApplications) return;

    for (int i = 0; i < events.size(); i++) {
        val & event = events[i];
        val currentActivity = Plugin::callOn <QString, Qt::DirectConnection> (m_activities, "CurrentActivity", "QString");

        // if blocked by default, the list contains allowed applications
        //     ignore event if the list doesn't contain the application
        // if not blocked by default, the list contains blocked applications
        //     ignore event if the list contains the application
        if (
                (m_whatToRemember == SpecificApplications) &&
                (m_blockedByDefault
                    ? !m_apps.contains(event.application)
                    :  m_apps.contains(event.application)
                )
           ) continue;

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

bool StatsPlugin::isFeatureOperational(const QStringList & feature) const
{
    Q_UNUSED(feature)
    return false;
}

bool StatsPlugin::isFeatureEnabled(const QStringList & feature) const
{
    Q_UNUSED(feature)
    return false;

}

void StatsPlugin::setFeatureEnabled(const QStringList & feature, bool value)
{
    Q_UNUSED(feature)
    Q_UNUSED(value)
}

QStringList StatsPlugin::listFeatures(const QStringList & feature) const
{
    Q_UNUSED(feature)
    static val features = (QStringList() << "scoring" << "more");

    return features;
}

KAMD_EXPORT_PLUGIN(StatsPlugin, "activitymanger_plugin_sqlite")
