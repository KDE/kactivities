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
#include <QDebug>

// KDE
#include <kconfig.h>
#include <kdbusconnectionpool.h>
#include <kfileitem.h>

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
#include "resourcescoringadaptor.h"
#include "common/specialvalues.h"

KAMD_EXPORT_PLUGIN(sqliteplugin, StatsPlugin, "kactivitymanagerd-plugin-sqlite.json")

StatsPlugin *StatsPlugin::s_instance = Q_NULLPTR;

StatsPlugin::StatsPlugin(QObject *parent, const QVariantList &args)
    : Plugin(parent)
    , m_activities(Q_NULLPTR)
    , m_resources(Q_NULLPTR)
    , m_resourceLinking(new ResourceLinking(this))
{
    Q_UNUSED(args);
    s_instance = this;

    new ResourcesScoringAdaptor(this);
    KDBusConnectionPool::threadConnection().registerObject(
        QStringLiteral("/ActivityManager/Resources/Scoring"), this);

    setName(QStringLiteral("org.kde.ActivityManager.Resources.Scoring"));
}

bool StatsPlugin::init(QHash<QString, QObject *> &modules)
{
    Plugin::init(modules);

    m_activities = modules[QStringLiteral("activities")];
    m_resources = modules[QStringLiteral("resources")];

    // Initializing the database
    resourcesDatabase();

    connect(m_resources, SIGNAL(ProcessedResourceEvents(EventList)),
            this, SLOT(addEvents(EventList)));
    connect(m_resources, SIGNAL(RegisteredResourceMimetype(QString, QString)),
            this, SLOT(saveResourceMimetype(QString, QString)));
    connect(m_resources, SIGNAL(RegisteredResourceTitle(QString, QString)),
            this, SLOT(saveResourceTitle(QString, QString)));

    connect(modules[QStringLiteral("config")], SIGNAL(pluginConfigChanged()),
            this, SLOT(loadConfiguration()));

    loadConfiguration();

    return true;
}

void StatsPlugin::loadConfiguration()
{
    config().config()->reparseConfiguration();

    const QString configFile
        = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
          + QStringLiteral("kactivitymanagerd-pluginsrc");

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

    // Delete old events, as per configuration.
    // For people who do not restart their computers, we should do this from
    // time to time. Doing this twice a day should be more than enough.
    deleteOldEvents();
    m_deleteOldEventsTimer.setInterval(12 * 60 * 60 * 1000);
    connect(&m_deleteOldEventsTimer, &QTimer::timeout,
            this, &StatsPlugin::deleteOldEvents);

    // Loading URL filters
    m_urlFilters.clear();

    auto filters = config().readEntry("url-filters",
            QStringList() << "about:*" // Ignore about: stuff
                          << "*/.*"    // Ignore hidden files
                          << "/"       // Ignore root
                          << "/tmp/*"  // Ignore everything in /tmp
            );

    for (const auto& filter: filters) {
        m_urlFilters << QRegExp(filter, Qt::CaseInsensitive, QRegExp::WildcardUnix);
    }
}

void StatsPlugin::deleteOldEvents()
{
    DeleteEarlierStats(QString(), config().readEntry("keep-history-for", 0));
}

void StatsPlugin::openResourceEvent(const QString &usedActivity,
                                    const QString &initiatingAgent,
                                    const QString &targettedResource,
                                    const QDateTime &start,
                                    const QDateTime &end)
{
    Q_ASSERT_X(!initiatingAgent.isEmpty(),
               "StatsPlugin::openResourceEvent",
               "Agent shoud not be empty");
    Q_ASSERT_X(!usedActivity.isEmpty(),
               "StatsPlugin::openResourceEvent",
               "Activity shoud not be empty");
    Q_ASSERT_X(!targettedResource.isEmpty(),
               "StatsPlugin::openResourceEvent",
               "Resource shoud not be empty");

    detectResourceInfo(targettedResource);

    Utils::prepare(resourcesDatabase(), openResourceEventQuery, QStringLiteral(
        "INSERT INTO ResourceEvent"
        "        (usedActivity,  initiatingAgent,  targettedResource,  start,  end) "
        "VALUES (:usedActivity, :initiatingAgent, :targettedResource, :start, :end)"
    ));

    Utils::exec(Utils::FailOnError, *openResourceEventQuery,
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
    Q_ASSERT_X(!initiatingAgent.isEmpty(),
               "StatsPlugin::closeResourceEvent",
               "Agent shoud not be empty");
    Q_ASSERT_X(!usedActivity.isEmpty(),
               "StatsPlugin::closeResourceEvent",
               "Activity shoud not be empty");
    Q_ASSERT_X(!targettedResource.isEmpty(),
               "StatsPlugin::closeResourceEvent",
               "Resource shoud not be empty");

    Utils::prepare(resourcesDatabase(), closeResourceEventQuery, QStringLiteral(
        "UPDATE ResourceEvent "
        "SET end = :end "
        "WHERE "
            ":usedActivity      = usedActivity AND "
            ":initiatingAgent   = initiatingAgent AND "
            ":targettedResource = targettedResource AND "
            "end IS NULL"
    ));

    Utils::exec(Utils::FailOnError, *closeResourceEventQuery,
        ":usedActivity"      , usedActivity      ,
        ":initiatingAgent"   , initiatingAgent   ,
        ":targettedResource" , targettedResource ,
        ":end"               , end.toTime_t()
    );
}

void StatsPlugin::detectResourceInfo(const QString &_uri)
{
    QString file = _uri;

    if (!file.startsWith('/')) {
        QUrl uri(_uri);

        if (!uri.isLocalFile()) return;

        file = uri.toLocalFile();

        if(!QFile::exists(file)) return;
    }

    KFileItem item(file);

    if (insertResourceInfo(file)) {
        saveResourceMimetype(file, item.mimetype(), true);

        const auto text = item.text();
        saveResourceTitle(file, text.isEmpty() ? _uri : text, true);
    }
}

bool StatsPlugin::insertResourceInfo(const QString &uri)
{

    Utils::prepare(resourcesDatabase(), getResourceInfoQuery, QStringLiteral(
        "SELECT targettedResource FROM ResourceInfo WHERE "
            "  targettedResource = :targettedResource "
    ));

    getResourceInfoQuery->bindValue(":targettedResource", uri);
    Utils::exec(Utils::FailOnError, *getResourceInfoQuery);

    if (getResourceInfoQuery->next()) {
        return false;
    }

    Utils::prepare(resourcesDatabase(), insertResourceInfoQuery, QStringLiteral(
        "INSERT INTO ResourceInfo( "
            "  targettedResource"
            ", title"
            ", autoTitle"
            ", mimetype"
            ", autoMimetype"
        ") VALUES ("
            "  :targettedResource"
            ", '' "
            ", 1 "
            ", '' "
            ", 1 "
        ")"
    ));

    Utils::exec(Utils::FailOnError, *insertResourceInfoQuery,
        ":targettedResource", uri
    );

    return true;
}

void StatsPlugin::saveResourceTitle(const QString &uri, const QString &title,
                                    bool autoTitle)
{
    insertResourceInfo(uri);

    DATABASE_TRANSACTION(resourcesDatabase());

    Utils::prepare(resourcesDatabase(), saveResourceTitleQuery, QStringLiteral(
        "UPDATE ResourceInfo SET "
            "  title = :title"
            ", autoTitle = :autoTitle "
        "WHERE "
            "targettedResource = :targettedResource "
    ));

    Utils::exec(Utils::FailOnError, *saveResourceTitleQuery,
        ":targettedResource" , uri                     ,
        ":title"             , title                   ,
        ":autoTitle"         , (autoTitle ? "1" : "0")
    );
}

void StatsPlugin::saveResourceMimetype(const QString &uri,
                                       const QString &mimetype,
                                       bool autoMimetype)
{
    insertResourceInfo(uri);

    DATABASE_TRANSACTION(resourcesDatabase());

    Utils::prepare(resourcesDatabase(), saveResourceMimetypeQuery, QStringLiteral(
        "UPDATE ResourceInfo SET "
            "  mimetype = :mimetype"
            ", autoMimetype = :autoMimetype "
        "WHERE "
            "targettedResource = :targettedResource "
    ));

    Utils::exec(Utils::FailOnError, *saveResourceMimetypeQuery,
        ":targettedResource" , uri                        ,
        ":mimetype"          , mimetype                   ,
        ":autoMimetype"      , (autoMimetype ? "1" : "0")
    );
}


StatsPlugin *StatsPlugin::self()
{
    return s_instance;
}

QString StatsPlugin::currentActivity() const
{
    return Plugin::callOnRet<QString, Qt::DirectConnection>(
        m_activities, "CurrentActivity", "QString");
}

bool StatsPlugin::acceptedEvent(const Event &event)
{
    using std::bind;
    using std::any_of;
    using namespace std::placeholders;

    return !(
        event.uri.isEmpty() ||
        any_of(m_urlFilters.cbegin(), m_urlFilters.cend(),
               bind(&QRegExp::exactMatch, _1, event.uri)) ||

        // if blocked by default, the list contains allowed applications
        //     ignore event if the list doesn't contain the application
        // if not blocked by default, the list contains blocked applications
        //     ignore event if the list contains the application
        (m_whatToRemember == SpecificApplications
            && m_blockedByDefault
                != boost::binary_search(m_apps, event.application))
    );
}

Event StatsPlugin::validateEvent(Event event)
{
    if (event.uri.startsWith(QStringLiteral("file://"))) {
        event.uri = QUrl(event.uri).toLocalFile();
    }

    if (event.uri.startsWith(QStringLiteral("/"))) {
        QFileInfo file(event.uri);

        event.uri = file.exists() ? file.canonicalFilePath() : QString();
    }

    return event;
}

void StatsPlugin::addEvents(const EventList &events)
{
    using namespace kamd::utils;

    if (m_blockAll || m_whatToRemember == NoApplications) {
        return;
    }

    const auto &eventsToProcess =
        events | transformed(&StatsPlugin::validateEvent, this)
               | filtered(&StatsPlugin::acceptedEvent, this);

    if (eventsToProcess.begin() == eventsToProcess.end()) return;

    DATABASE_TRANSACTION(resourcesDatabase());

    for (auto event : eventsToProcess) {

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

void StatsPlugin::DeleteRecentStats(const QString &activity, int count,
                                    const QString &what)
{
    const auto usedActivity = activity.isEmpty() ? QVariant()
                                                 : QVariant(activity);

    // If we need to delete everything,
    // no need to bother with the count and the date

    DATABASE_TRANSACTION(resourcesDatabase());

    if (what == QStringLiteral("everything")) {
        // Instantiating these every time is not a big overhead
        // since this method is rarely executed.

        auto removeEventsQuery = resourcesDatabase().createQuery();
        removeEventsQuery.prepare(
                "DELETE FROM ResourceEvent "
                "WHERE usedActivity = COALESCE(:usedActivity, usedActivity)"
            );

        auto removeScoreCachesQuery = resourcesDatabase().createQuery();
        removeScoreCachesQuery.prepare(
                "DELETE FROM ResourceScoreCache "
                "WHERE usedActivity = COALESCE(:usedActivity, usedActivity)");

        Utils::exec(Utils::FailOnError, removeEventsQuery, ":usedActivity", usedActivity);
        Utils::exec(Utils::FailOnError, removeScoreCachesQuery, ":usedActivity", usedActivity);

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

        auto removeEventsQuery = resourcesDatabase().createQuery();
        removeEventsQuery.prepare(
                "DELETE FROM ResourceEvent "
                "WHERE usedActivity = COALESCE(:usedActivity, usedActivity) "
                "AND end > :since"
            );

        auto removeScoreCachesQuery = resourcesDatabase().createQuery();
        removeScoreCachesQuery.prepare(
                "DELETE FROM ResourceScoreCache "
                "WHERE usedActivity = COALESCE(:usedActivity, usedActivity) "
                "AND firstUpdate > :since");

        Utils::exec(Utils::FailOnError, removeEventsQuery,
                ":usedActivity", usedActivity,
                ":since", since.toTime_t()
            );

        Utils::exec(Utils::FailOnError, removeScoreCachesQuery,
                ":usedActivity", usedActivity,
                ":since", since.toTime_t()
            );
    }

    emit RecentStatsDeleted(activity, count, what);
}

void StatsPlugin::DeleteEarlierStats(const QString &activity, int months)
{
    if (months == 0) {
        return;
    }

    // Deleting a specified length of time

    DATABASE_TRANSACTION(resourcesDatabase());

    const auto time = QDateTime::currentDateTime().addMonths(-months);
    const auto usedActivity = activity.isEmpty() ? QVariant()
                                                 : QVariant(activity);

    auto removeEventsQuery = resourcesDatabase().createQuery();
    removeEventsQuery.prepare(
            "DELETE FROM ResourceEvent "
            "WHERE usedActivity = COALESCE(:usedActivity, usedActivity) "
            "AND start < :time"
        );

    auto removeScoreCachesQuery = resourcesDatabase().createQuery();
    removeScoreCachesQuery.prepare(
            "DELETE FROM ResourceScoreCache "
            "WHERE usedActivity = COALESCE(:usedActivity, usedActivity) "
            "AND lastUpdate < :time");

    Utils::exec(Utils::FailOnError, removeEventsQuery,
            ":usedActivity", usedActivity,
            ":time", time.toTime_t()
        );

    Utils::exec(Utils::FailOnError, removeScoreCachesQuery,
            ":usedActivity", usedActivity,
            ":time", time.toTime_t()
        );

    emit EarlierStatsDeleted(activity, months);
}

void StatsPlugin::DeleteStatsForResource(const QString &activity,
                                         const QString &client,
                                         const QString &resource)
{
    Q_ASSERT_X(!client.isEmpty(),
               "StatsPlugin::DeleteStatsForResource",
               "Agent shoud not be empty");
    Q_ASSERT_X(!activity.isEmpty(),
               "StatsPlugin::DeleteStatsForResource",
               "Activity shoud not be empty");
    Q_ASSERT_X(!resource.isEmpty(),
               "StatsPlugin::DeleteStatsForResource",
               "Resource shoud not be empty");
    Q_ASSERT_X(client != CURRENT_AGENT_TAG,
               "StatsPlugin::DeleteStatsForResource",
               "We can not handle CURRENT_AGENT_TAG here");

    DATABASE_TRANSACTION(resourcesDatabase());

    // Check against sql injection
    if (activity.contains('\'') || client.contains('\'')) return;

    const auto activityFilter =
            activity == ANY_ACTIVITY_TAG ? " 1 " :
                QStringLiteral(" usedActivity = '%1' ").arg(
                    activity == CURRENT_ACTIVITY_TAG ?
                            currentActivity() : activity
                );

    const auto clientFilter =
            client == ANY_AGENT_TAG ? " 1 " :
                QStringLiteral(" initiatingAgent = '%1' ").arg(client);

    auto removeEventsQuery = resourcesDatabase().createQuery();
    removeEventsQuery.prepare(
            "DELETE FROM ResourceEvent "
            "WHERE "
                + activityFilter + " AND "
                + clientFilter + " AND "
                + "targettedResource GLOB :targettedResource"
        );

    auto removeScoreCachesQuery = resourcesDatabase().createQuery();
    removeScoreCachesQuery.prepare(
            "DELETE FROM ResourceScoreCache "
            "WHERE "
                + activityFilter + " AND "
                + clientFilter + " AND "
                + "targettedResource GLOB :targettedResource"
        );

    Utils::exec(Utils::FailOnError, removeEventsQuery,
                ":targettedResource", resource);

    Utils::exec(Utils::FailOnError, removeScoreCachesQuery,
                ":targettedResource", resource);

    emit ResourceScoreDeleted(activity, client, resource);
}

#include "StatsPlugin.moc"

