/*
 *   Copyright (C) 2011, 2012 Ivan Cukic ivan.cukic(at)kde.org
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

// Self
#include "Rankings.h"

// STL
#include <algorithm>

// Qt
#include <QDBusConnection>
#include <QVariantList>
#include <QSqlQuery>

// KDE
#include <kdbusconnectionpool.h>

// Utils
#include <utils/range.h>
#include <utils/remove_if.h>
#include <utils/qsqlquery.h>

// Local
#include "Debug.h"
#include "ResourceScoreCache.h"
#include "Database.h"
#include "StatsPlugin.h"
#include "rankingsadaptor.h"


#define clientInterface(dbusPath)                                              \
    QDBusInterface(dbusPath, QStringLiteral("/RankingsClient"),                \
                   QStringLiteral("org.kde.ActivityManager.RankingsClient"))

Rankings *Rankings::self()
{
    static Rankings instance;
    return &instance;
}

Rankings::Rankings()
{
    new RankingsAdaptor(this);
    KDBusConnectionPool::threadConnection().registerObject(
        QStringLiteral("/Rankings"), this);
}

Rankings::~Rankings()
{
}

void Rankings::registerClient(const QString &client, const QString &requestId,
                              const QString &activity,
                              const QString &application)
{
    m_clients.insert(ClientPattern(client, requestId, activity, application));

    static const auto query = QStringLiteral(
        "SELECT targettedResource, cachedScore "
        "FROM kext_ResourceScoreCache " // this should be kao_ResourceScoreCache, but lets leave it
        "WHERE %1 AND %2 "
        "AND cachedScore > 0 "
        "ORDER BY cachedScore DESC LIMIT 30");

    static const auto usedActivity = QStringLiteral("usedActivity = '%1'");
    static const auto initiatingAgent = QStringLiteral("initiatingAgent = '%1'");

    auto results = Database::self()->exec(
        query.arg(activity.isEmpty() ? QStringLiteral("1")
                                     : usedActivity.arg(activity))
             .arg(application.isEmpty() ? QStringLiteral("1")
                                        : initiatingAgent.arg(application)));

    QHash<QString, QVariant> update;

    for (const auto &result: results) {
        const auto url = result[0].toString();
        const auto score = result[1].toReal();

        update[url] = score;
    }

    clientInterface(client).asyncCall(QStringLiteral("updated"), requestId,
                                      QStringLiteral("replace"),
                                      QVariant(update));
}

void Rankings::deregisterClient(const QString &client, const QString &requestId)
{
    m_clients.erase(ClientPattern(client, requestId));
}

void Rankings::resourceScoreUpdated(const QString &activity,
                                    const QString &application,
                                    const QString &resource, qreal score)
{
    using namespace kamd::utils;

    Q_UNUSED(application);

    QHash<QString, QVariant> update;

    update[resource] = score;

    for (const auto &client :
            m_clients | filtered(ClientPattern::matches, activity, application)
    ) {
        clientInterface(client.dbusPath)
            .asyncCall(QStringLiteral("updated"), client.requestId,
                       QStringLiteral("incremental"), QVariant(update));
    }
}

