/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "resultset.h"

// Qt
#include <QSqlQuery>
#include <QSqlError>
#include <QCoreApplication>

// Local
#include <common/database/Database.h>
#include <utils/debug_and_return.h>

// Boost and STL
#include <boost/range/algorithm/transform.hpp>
#include <iterator>
#include <functional>
#include <mutex>
#include <boost/optional.hpp>

// KActivities
#include <kactivities/consumer.h>

namespace KActivities {
namespace Experimental {
namespace Stats {

using namespace Terms;

// Main class

class ResultSet::Private {
public:
    Common::Database::Ptr db;
    QSqlQuery query;
    Query queryDefinition;

    mutable std::unique_ptr<KActivities::Consumer> activities;
    mutable std::mutex activities_mutex;

    void initQuery()
    {
        if (query.isActive()) {
            return;
        }

        auto selection = queryDefinition.selection();

        query = db->execQuery(
                selection == LinkedResources ? linkedResourcesQuery()
              : selection == UsedResources   ? usedResourcesQuery()
              : selection == AllResources    ? allResourcesQuery()
              : QString());

        Q_ASSERT_X(query.isActive(), "ResultSet initQuery", "Query is not valid");

        // TODO: Implement types
        // QStringList types() const;

    }

    QString getCurrentActivity() const
    {
        // We need to get the current activity synchonously,
        // this means waiting for the service to be available.
        // It should not introduce blockages since there usually
        // is a global activity cache in applications that care
        // about activities.

        if (!activities) {
            std::unique_lock<std::mutex> lock(activities_mutex);
            activities.reset(new KActivities::Consumer());
        }

        while (activities->serviceStatus() == KActivities::Consumer::Unknown) {
            QCoreApplication::instance()->processEvents();
        }

        return activities->currentActivity();
    }

    QString agentClause(const QString &agent) const
    {
        if (agent == ":any") return "1";

        return "rsc.initiatingAgent = '" + (
                agent == ":global"  ? "" :
                agent == ":current" ? QCoreApplication::instance()->applicationName() :
                                      agent
            ) + "'";
    }

    QString activityClause(const QString &activity) const
    {
        if (activity == ":any") return "1";

        return "rsc.usedActivity = '" + (
                activity == ":global"  ? "" :
                activity == ":current" ? getCurrentActivity() :
                                         activity
            ) + "'";
    }

    /**
     * Transforms the input list's elements with the f member method,
     * and returns the resulting list
     */
    template <typename F>
    inline
    QStringList transformedList(const QStringList &input, F f) const
    {
        using namespace std::placeholders;

        QStringList result;
        boost::transform(input,
                         std::back_inserter(result),
                         std::bind(f, this, _1));

        return result;
    }

    QString linkedResourcesQuery() const
    {
        // static const QString _query =
        //     "SELECT   rl.targettedResource "
        //     "FROM     ResourceLink rl "
        //     "    LEFT JOIN ResourceScoreCache rsc "
        //     "    ON   rl.targettedResource = rsc.targettedResource "
        //     "WHERE    ($agentsFilter) AND ($activitiesFilter) "
        //     "ORDER BY $orderingColumn rsc.targettedResource ASC ";
        static const QString _query =
            "SELECT   rl.targettedResource "
            "FROM     ResourceLink rl "
            "WHERE    ($agentsFilter) AND ($activitiesFilter) "
            "ORDER BY rsc.targettedResource ASC ";

        Q_ASSERT_X(queryDefinition.ordering() == OrderAlphabetically,
                "Linked resource query creation",
                "Alternative orderings for the linked resources are not supported");

        // ORDER BY column
        // auto ordering = queryDefinition.ordering();
        // QString orderingColumn =
        //         ordering == HighScoredFirst      ? "rsc.cachedScore DESC,"
        //       : ordering == RecentlyCreatedFirst ? "rsc.firstUpdate DESC,"
        //       : ordering == RecentlyUsedFirst    ? "rsc.lastUpdate DESC,"
        //       : QString();


        // WHERE clause for filtering on agents
        QStringList agentsFilter = transformedList(
                queryDefinition.agents(), &Private::agentClause);

        // WHERE clause for filtering on activities
        QStringList activitiesFilter = transformedList(
                queryDefinition.activities(), &Private::activityClause);

        auto query = _query;

        return query
                // .replace("$orderingColumn", orderingColumn)
                .replace("$agentsFilter", agentsFilter.join(" OR "))
                .replace("$activitiesFilter", activitiesFilter.join(" OR "))
            ;
    }

    QString usedResourcesQuery() const
    {
        // TODO: We need to group by and sum scores at some point.
        //       This is a minor use-case, but someone might want to
        //       have lists for two agents merged into one or
        //       something similar. This applies to other queries
        //       as well.
        static const QString _query =
            "SELECT   rsc.targettedResource, rsc.cachedScore "
            "FROM     ResourceScoreCache rsc "
            "    LEFT JOIN ResourceInfo ri "
            "    ON rsc.targettedResource = ri.targettedResource "
            "WHERE    ($agentsFilter) AND ($activitiesFilter) "
            "ORDER BY $orderingColumn rsc.targettedResource ASC";

        // ORDER BY column
        auto ordering = queryDefinition.ordering();
        QString orderingColumn =
                ordering == HighScoredFirst      ? "rsc.cachedScore DESC,"
              : ordering == RecentlyCreatedFirst ? "rsc.firstUpdate DESC,"
              : ordering == RecentlyUsedFirst    ? "rsc.lastUpdate DESC,"
              : QString();


        // WHERE clause for filtering on agents
        QStringList agentsFilter = transformedList(
                queryDefinition.agents(), &Private::agentClause);

        // WHERE clause for filtering on activities
        QStringList activitiesFilter = transformedList(
                queryDefinition.activities(), &Private::activityClause);

        auto query = _query;

        return query
                .replace("$orderingColumn", orderingColumn)
                .replace("$agentsFilter", agentsFilter.join(" OR "))
                .replace("$activitiesFilter", activitiesFilter.join(" OR "))
            ;
    }

    QString allResourcesQuery() const
    {
        // TODO: Get this to work
        static QString query =
            "SELECT * FROM ResourceLink ";

        return query;
    }

};

ResultSet::ResultSet(Query query)
    : d(new Private())
{
    using namespace Common;

    d->db = Database::instance(Database::ResourcesDatabase, Database::ReadOnly);
    Q_ASSERT_X(d->db, "ResultSet constructor", "Database is NULL");

    d->queryDefinition = query;

    d->initQuery();
}

ResultSet::~ResultSet()
{
    delete d;
}

ResultSet::Result ResultSet::at(int index) const
{
    Q_ASSERT_X(d->query.isActive(), "ResultSet::at", "Query is not active");
    // Q_ASSERT_X(d->query.size() >= 0, "ResultSet::at", "This query is not countable?!");
    // Q_ASSERT_X(index < d->query.size(), "ResultSet::at", "This index does not exist");

    d->query.seek(index);

    return Result {
        d->query.value(0).toString(),
        d->query.value(1).toDouble()
    };
}

QStringList ResultSet::_results_() const
{
    QStringList result;

    d->query.seek(-1);

    while (d->query.next()) {
        result << d->query.value(0).toString();
    }

    return result;
}

} // namespace Stats
} // namespace Experimental
} // namespace KActivities

#include "resultset_iterator.cpp"

