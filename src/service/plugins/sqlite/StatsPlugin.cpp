/*
 *   Copyright (C) 2011, 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
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

// Self
#include <kactivities-features.h>
#include "StatsPlugin.h"

// Qt
#include <QFileSystemWatcher>
#include <QSqlQuery>

// KDE
#include <kconfig.h>
#include <kdbusconnectionpool.h>

// Boost
#include <boost/range/algorithm/binary_search.hpp>
#include <utils/range.h>

// Local
#include "Debug.h"
#include "Database.h"
#include "ResourceScoreMaintainer.h"
#include "ResourceLinking.h"
#include "Utils.h"
#include "../../Event.h"

KAMD_EXPORT_PLUGIN(sqliteplugin, StatsPlugin, "kactivitymanagerd-plugin-sqlite.json")

StatsPlugin *StatsPlugin::s_instance = Q_NULLPTR;

StatsPlugin::StatsPlugin(QObject *parent, const QVariantList &args)
    : Plugin(parent)
    , m_activities(Q_NULLPTR)
    , m_resources(Q_NULLPTR)
    , m_configWatcher(Q_NULLPTR)
    , m_resourceLinking(new ResourceLinking(this))
{
    Q_UNUSED(args)
    s_instance = this;

    // new ScoringAdaptor(this);
    // KDBusConnectionPool::threadConnection().registerObject(
    //     QStringLiteral("/ActivityManager/Resources/Scoring"), this);

    setName(QStringLiteral("org.kde.ActivityManager.Resources.Scoring"));
}

bool StatsPlugin::init(const QHash<QString, QObject *> &modules)
{
    m_activities = modules[QStringLiteral("activities")];
    m_resources = modules[QStringLiteral("resources")];

    // Initializing the database
    resourcesDatabase();

    connect(m_resources, SIGNAL(ProcessedResourceEvents(EventList)),
            this, SLOT(addEvents(EventList)));

    loadConfiguration();

    return true;
}

void StatsPlugin::loadConfiguration()
{
    config().config()->reparseConfiguration();

    const QString configFile
        = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
          + QStringLiteral("kactivitymanagerd-pluginsrc");

    if (m_configWatcher) {
        // When saving a config file, KConfig deletes the old,
        // and creates the new one, so the watcher stops watching
        m_configWatcher->addPath(configFile);

    } else {
        m_configWatcher = new QFileSystemWatcher(QStringList{configFile}, this);

        connect(m_configWatcher, SIGNAL(fileChanged(QString)),
                this, SLOT(loadConfiguration()));
        connect(m_activities, SIGNAL(CurrentActivityChanged(QString)),
                this, SLOT(loadConfiguration()));
    }

    m_blockedByDefault = config().readEntry("blocked-by-default", false);
    m_blockAll = false;
    m_whatToRemember = (WhatToRemember)config().readEntry("what-to-remember",
                                                          (int)AllApplications);

    m_apps.clear();

    if (m_whatToRemember == SpecificApplications) {
        auto apps = config().readEntry(
            m_blockedByDefault ? "allowed-applications" : "blocked-applications",
            QStringList());

        m_apps.insert(apps.cbegin(), apps.cend());
    }

    // Delete old events, as per configuration
    // TODO: Event cleanup should be also done from time to time,
    //       not only on startup
    deleteEarlierStats(QString(), config().readEntry("keep-history-for", 0));
}

void StatsPlugin::openResourceEvent(const QString &usedActivity,
                                    const QString &initiatingAgent,
                                    const QString &targettedResource,
                                    const QDateTime &start,
                                    const QDateTime &end)
{
    Utils::prepare(resourcesDatabase(), openResourceEventQuery, QStringLiteral(
        "INSERT INTO ResourceEvent"
        "        (usedActivity,  initiatingAgent,  targettedResource,  start,  end) "
        "VALUES (:usedActivity, :initiatingAgent, :targettedResource, :start, :end)"
    ));

    Utils::exec(*openResourceEventQuery,
        ":usedActivity"      , usedActivity      ,
        ":initiatingAgent"   , initiatingAgent   ,
        ":targettedResource" , targettedResource ,
        ":start"             , start.toTime_t()  ,
        ":end"               , (end.isNull()) ? QVariant() : end.toTime_t()
    );
}

void StatsPlugin::closeResourceEvent(const QString &usedActivity,
                                     const QString &initiatingAgent,
                                     const QString &targettedResource,
                                     const QDateTime &end)
{
    Utils::prepare(resourcesDatabase(), closeResourceEventQuery, QStringLiteral(
        "UPDATE ResourceEvent "
        "SET end = :end "
        "WHERE "
            ":usedActivity      = usedActivity AND "
            ":initiatingAgent   = initiatingAgent AND "
            ":targettedResource = targettedResource AND "
            "end IS NULL"
    ));

    Utils::exec(*closeResourceEventQuery,
        ":usedActivity"      , usedActivity      ,
        ":initiatingAgent"   , initiatingAgent   ,
        ":targettedResource" , targettedResource ,
        ":end"               , end.toTime_t()
    );
}


StatsPlugin *StatsPlugin::self()
{
    return s_instance;
}

QString StatsPlugin::currentActivity() const
{
    return Plugin::callOn<QString, Qt::DirectConnection>(
        m_activities, "CurrentActivity", "QString");
}

bool StatsPlugin::acceptedEvent(const Event &event)
{
    return !(
        event.uri.startsWith(QStringLiteral("about")) ||

        // if blocked by default, the list contains allowed applications
        //     ignore event if the list doesn't contain the application
        // if not blocked by default, the list contains blocked applications
        //     ignore event if the list contains the application
        (m_whatToRemember == SpecificApplications
            && m_blockedByDefault
                != boost::binary_search(m_apps, event.application))
    );
}

void StatsPlugin::addEvents(const EventList &events)
{
    using namespace kamd::utils;

    if (m_blockAll || m_whatToRemember == NoApplications) {
        return;
    }

    for (const auto &event :
            events | filtered(&StatsPlugin::acceptedEvent, this)) {

        switch (event.type) {
            case Event::Accessed:
                openResourceEvent(
                    currentActivity(), event.application, event.uri,
                    event.timestamp, event.timestamp);
                ResourceScoreMaintainer::self()->processResource(
                    event.uri, event.application);

                break;

            case Event::Opened:
                openResourceEvent(
                    currentActivity(), event.application, event.uri,
                    event.timestamp);

                break;

            case Event::Closed:
                closeResourceEvent(
                    currentActivity(), event.application, event.uri,
                    event.timestamp);
                ResourceScoreMaintainer::self()->processResource(
                    event.uri, event.application);

                break;

            case Event::UserEventType:
                ResourceScoreMaintainer::self()->processResource(
                    event.uri, event.application);
                break;

            default:
                // Nothing yet
                // TODO: Add focus and modification
                break;
        }
    }
}

void StatsPlugin::deleteRecentStats(const QString &activity, int count,
                                    const QString &what)
{
    const auto usedActivity = activity.isEmpty() ? QVariant()
                                                 : QVariant(activity);

    // If we need to delete everything,
    // no need to bother with the count and the date

    if (what == QStringLiteral("everything")) {
        // Instantiating these every time is not a big overhead
        // since this method is rarely executed.

        auto removeEvents = resourcesDatabase().createQuery();
        removeEvents.prepare(
                "DELETE FROM ResourceEvent "
                "WHERE usedActivity = COALESCE(:usedActivity, usedActivity)"
            );

        auto removeScoreCaches = resourcesDatabase().createQuery();
        removeScoreCaches.prepare(
                "DELETE FROM ResourceScoreCache "
                "WHERE usedActivity = COALESCE(:usedActivity, usedActivity)");

        Utils::exec(removeEvents, ":usedActivity", usedActivity);
        Utils::exec(removeScoreCaches, ":usedActivity", usedActivity);

    } else {

        // Deleting a specified length of time

        auto since = QDateTime::currentDateTime();

        since = (what[0] == QLatin1Char('h')) ? since.addSecs(-count * 60 * 60)
              : (what[0] == QLatin1Char('d')) ? since.addDays(-count)
              : (what[0] == QLatin1Char('m')) ? since.addMonths(-count)
              : since;

        // Maybe we should decrease the scores for the previously
        // cached items. Thinking it is not that important -
        // if something was accessed before, and the user did not
        // remove the history, it is not really a secret.

        auto removeEvents = resourcesDatabase().createQuery();
        removeEvents.prepare(
                "DELETE FROM ResourceEvent "
                "WHERE usedActivity = COALESCE(:usedActivity, usedActivity) "
                "AND end > :since"
            );

        auto removeScoreCaches = resourcesDatabase().createQuery();
        removeScoreCaches.prepare(
                "DELETE FROM ResourceScoreCache "
                "WHERE usedActivity = COALESCE(:usedActivity, usedActivity) "
                "AND firstUpdate > :since");

        Utils::exec(removeEvents,
                ":usedActivity", usedActivity,
                ":since", since.toTime_t()
            );

        Utils::exec(removeScoreCaches,
                ":usedActivity", usedActivity,
                ":since", since.toTime_t()
            );
    }

    emit recentStatsDeleted(activity, count, what);
}

void StatsPlugin::deleteEarlierStats(const QString &activity, int months)
{
    if (months == 0) {
        return;
    }

    // Deleting a specified length of time

    const auto time = QDateTime::currentDateTime().addMonths(-months);
    const auto usedActivity = activity.isEmpty() ? QVariant()
                                                 : QVariant(activity);

    auto removeEvents = resourcesDatabase().createQuery();
    removeEvents.prepare(
            "DELETE FROM ResourceEvent "
            "WHERE usedActivity = COALESCE(:usedActivity, usedActivity) "
            "AND start < :time"
        );

    auto removeScoreCaches = resourcesDatabase().createQuery();
    removeScoreCaches.prepare(
            "DELETE FROM ResourceScoreCache "
            "WHERE usedActivity = COALESCE(:usedActivity, usedActivity) "
            "AND lastUpdate < :time");

    Utils::exec(removeEvents,
            ":usedActivity", usedActivity,
            ":time", time.toTime_t()
        );

    Utils::exec(removeScoreCaches,
            ":usedActivity", usedActivity,
            ":time", time.toTime_t()
        );

    emit earlierStatsDeleted(activity, months);
}

#include "StatsPlugin.moc"

