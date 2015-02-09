/*
 *   Copyright (C) 2011, 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLUGINS_SQLITE_STATS_PLUGIN_H
#define PLUGINS_SQLITE_STATS_PLUGIN_H

// Qt
#include <QObject>

// Boost and STL
#include <memory>
#include <boost/container/flat_set.hpp>

// Local
#include <Plugin.h>

class QSqlQuery;
class QFileSystemWatcher;
class ResourceLinking;

/**
 * Communication with the outer world.
 *
 * - Handles configuration
 * - Filters the events based on the user's configuration.
 */
class StatsPlugin : public Plugin {
    Q_OBJECT
    // Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.Resources.Scoring")
    // Q_PLUGIN_METADATA(IID "org.kde.ActivityManager.plugins.sqlite")

public:
    explicit StatsPlugin(QObject *parent = Q_NULLPTR,
                         const QVariantList &args = QVariantList());

    static StatsPlugin *self();

    virtual bool init(const QHash<QString, QObject *> &modules) Q_DECL_OVERRIDE;

    QString currentActivity() const;

    inline
    QObject *activitiesInterface() const { return m_activities; }

//
// D-BUS Interface methods
//

public Q_SLOTS:
    void DeleteRecentStats(const QString &activity, int count,
                           const QString &what);

    void DeleteEarlierStats(const QString &activity, int months);


Q_SIGNALS:
    void ResourceScoreUpdated(const QString &activity, const QString &client,
                              const QString &resource, double score);

    void RecentStatsDeleted(const QString &activity, int count,
                            const QString &what);

    void EarlierStatsDeleted(const QString &activity, int months);

//
// End D-BUS Interface methods
//


private Q_SLOTS:
    void addEvents(const EventList &events);
    void loadConfiguration();

    void openResourceEvent(const QString &usedActivity,
                           const QString &initiatingAgent,
                           const QString &targettedResource,
                           const QDateTime &start,
                           const QDateTime &end = QDateTime());

    void closeResourceEvent(const QString &usedActivity,
                            const QString &initiatingAgent,
                            const QString &targettedResource,
                            const QDateTime &end);

    void saveResourceTitle(const QString &uri, const QString &title,
                           bool autoTitle = false);
    void saveResourceMimetype(const QString &uri, const QString &mimetype,
                              bool autoMimetype = false);
    void insertResourceInfo(const QString &uri);
    void detectResourceInfo(const QString &uri);

private:
    inline bool acceptedEvent(const Event &event);

    enum WhatToRemember {
        AllApplications = 0,
        SpecificApplications = 1,
        NoApplications = 2
    };

    QObject *m_activities;
    QObject *m_resources;
    QFileSystemWatcher *m_configWatcher;

    boost::container::flat_set<QString> m_apps;

    std::unique_ptr<QSqlQuery> openResourceEventQuery;
    std::unique_ptr<QSqlQuery> closeResourceEventQuery;

    std::unique_ptr<QSqlQuery> insertResourceInfoQuery;
    std::unique_ptr<QSqlQuery> saveResourceTitleQuery;
    std::unique_ptr<QSqlQuery> saveResourceMimetypeQuery;

    bool m_blockedByDefault : 1;
    bool m_blockAll : 1;
    WhatToRemember m_whatToRemember : 2;

    ResourceLinking *m_resourceLinking;

    static StatsPlugin *s_instance;
};

#endif // PLUGINS_SQLITE_STATS_PLUGIN_H
