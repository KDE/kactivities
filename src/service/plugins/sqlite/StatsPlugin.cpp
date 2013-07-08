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
#include "scoringadaptor.h"

#include "../../Event.h"
#include <kdbusconnectionpool.h>

#include <QFileSystemWatcher>
#include <QSqlQuery>
#include <QDebug>

#include <KStandardDirs>

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

    new ScoringAdaptor(this);
    KDBusConnectionPool::threadConnection().registerObject("/ActivityManager/Resources/Scoring", this);

    setName("org.kde.ActivityManager.Resources.Scoring");
}

bool StatsPlugin::init(const QHash < QString, QObject * > & modules)
{
    qDebug() << "These are the registered modules: " << modules.keys();

    m_activities = modules["activities"];
    m_resources = modules["resources"];

    DatabaseConnection::self();
    // Rankings::init(this);

    // connect(m_activities, SIGNAL(CurrentActivityChanged(QString)),
    //         Rankings::self(), SLOT(setCurrentActivity(QString)));

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

    // Delete old events, as per configuration
    // TODO: This should be also done from time to time,
    // not only on startup
    deleteEarlierStats(QString(), config().readEntry("keep-history-for", 0));
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

        if (event.uri.startsWith(QLatin1String("about"))) continue;

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

void StatsPlugin::deleteRecentStats(const QString & activity, int count, const QString & what)
{
    val activityCheck = activity.isEmpty() ?
        QString(" 1 ") :
        QString(" usedActivity = '" + activity + "' ");

    // If we need to delete everything,
    // no need to bother with the count and the date

    if (what == "everything") {
        DatabaseConnection::self()->database().exec(
                "DELETE FROM kext_ResourceScoreCache WHERE " + activityCheck
            );
        DatabaseConnection::self()->database().exec(
                "DELETE FROM nuao_DesktopEvent WHERE " + activityCheck
            );

    } else {

        // Deleting a specified length of time

        auto now = QDateTime::currentDateTime();

        if (what == "h") {
            now = now.addSecs(-count * 60 * 60);

        } else if (what == "d") {
            now = now.addDays(-count);

        } else if (what == "m") {
            now = now.addMonths(-count);
        }

        // Maybe we should decrease the scores for the previosuly
        // cached items. Thkinking it is not that important -
        // if something was accessed before, it is not really a secret

        static val queryRSC = QString(
                "DELETE FROM kext_ResourceScoreCache "
                " WHERE %1 "
                " AND firstUpdate > %2 "
            );
        static val queryDE = QString(
                "DELETE FROM nuao_DesktopEvent "
                " WHERE %1 "
                " AND end > %2 "
            );

        DatabaseConnection::self()->database().exec(
            queryRSC
                .arg(activityCheck)
                .arg(now.toTime_t())
            );
        DatabaseConnection::self()->database().exec(
            queryDE
                .arg(activityCheck)
                .arg(now.toTime_t())
            );
    }

    emit recentStatsDeleted(activity, count, what);
}

void StatsPlugin::deleteEarlierStats(const QString & activity, int months)
{
    if (months == 0) return;

    val activityCheck = activity.isEmpty() ?
        QString(" 1 ") :
        QString(" usedActivity = '" + activity + "' ");

    // Deleting a specified length of time

    val time = QDateTime::currentDateTime().addMonths(-months);

    static val queryRSC = QString(
            "DELETE FROM kext_ResourceScoreCache "
            " WHERE %1 "
            " AND lastUpdate < %2 "
        );
    static val queryDE = QString(
            "DELETE FROM nuao_DesktopEvent "
            " WHERE %1 "
            " AND start < %2 "
        );

    DatabaseConnection::self()->database().exec(
        queryRSC
            .arg(activityCheck)
            .arg(time.toTime_t())
        );

    DatabaseConnection::self()->database().exec(
        queryDE
            .arg(activityCheck)
            .arg(time.toTime_t())
        );

    emit earlierStatsDeleted(activity, months);
}

KAMD_EXPORT_PLUGIN(StatsPlugin, "activitymanger_plugin_sqlite")
