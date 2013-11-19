/*
 *   Copyright (C) 2011, 2012 Ivan Cukic <ivan.cukic@kde.org>
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

#ifndef PLUGINS_SQLITE_RANKINGS_H
#define PLUGINS_SQLITE_RANKINGS_H

// Qt
#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QDBusInterface>

// Boost
#include <boost/container/flat_set.hpp>


class Rankings : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.Rankings")

public:
    static Rankings *self();

    void resourceScoreUpdated(const QString &activity,
                              const QString &application,
                              const QString &resource, qreal score);

    ~Rankings();

public Q_SLOTS:
    /**
     * Registers a new client for the specified activity and application
     * @param dbusPath d-bus name
     * @param requestId applications can have interest in different items
     * @param activity activity to track. If empty, all activities are
     *                 aggregated
     * @param application application to track. If empty, all applications are
     *                    aggregated
     */
    void registerClient(const QString &dbusPath,
                        const QString &requestId,
                        const QString &activity = QString(),
                        const QString &application = QString());

    /**
     * Deregisters a dbusPath
     */
    void deregisterClient(const QString &dbusPath,
                          const QString &requestId);

private:
    Rankings();

    static Rankings *s_instance;

    struct ResultItem {
        QString uri;
        qreal score;
    };

    struct ClientPattern {
        ClientPattern(const QString &dbusPath, const QString &requestId,
                      const QString &activity = QString(),
                      const QString &application = QString())
            : dbusPath(dbusPath)
            , requestId(requestId)
            , activity(activity)
            , application(application)
        {
        }

        QString dbusPath;
        QString requestId;
        QString activity;
        QString application;

        bool operator<(const ClientPattern &other) const
        {
            return dbusPath < other.dbusPath ||
                   (dbusPath == other.dbusPath && requestId < other.requestId);
        }

        bool operator==(const ClientPattern &other) const
        {
            return dbusPath == other.dbusPath && requestId == other.requestId;
        }

        static inline bool matches(const QString &activity,
                                   const QString &application,
                                   const ClientPattern &dbusPath)
        {
            return
                (
                    dbusPath.activity.isEmpty() ||
                    activity == dbusPath.activity
                ) && (
                    dbusPath.application.isEmpty() ||
                    application == dbusPath.application
                );
        }
    };

private:
    boost::container::flat_set<ClientPattern> m_clients;
};

#endif // PLUGINS_SQLITE_RANKINGS_H
