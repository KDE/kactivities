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

#include "resultwatcher.h"

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
#include <limits>
#include <boost/optional.hpp>

// KActivities
#include <kactivities/consumer.h>

#include "resourceslinking_interface.h"
#include "resourcesscoring_interface.h"
#include "common/dbus/common.h"
#include "common/specialvalues.h"
#include "activitiessync_p.h"
#include "utils/lazy_val.h"
#include "utils/qsqlquery_iterator.h"

#include <boost/algorithm/cxx11/any_of.hpp>

namespace KActivities {
namespace Experimental {
namespace Stats {

using boost::algorithm::any_of;

// Main class

class ResultWatcher::Private {
public:
    mutable ActivitiesSync::ConsumerPtr activities;

    Private(ResultWatcher *parent, Query query)
        : linking(new KAMD_DBUS_CLASS_INTERFACE(Resources/Linking, ResourcesLinking, Q_NULLPTR))
        , scoring(new KAMD_DBUS_CLASS_INTERFACE(Resources/Scoring, ResourcesScoring, Q_NULLPTR))
        , q(parent)
        , query(query)
    {
    }

    // Processing the list of activities as specified by the query.
    // If it contains :any, we are returning true, otherwise
    // we want to match a specific activity (be it the current
    // activity or not). The :global special value is not special here
    bool activityMatches(const QString &activity) const
    {
        return any_of(query.activities(), [&] (const QString &matcher) {
            return (matcher == ANY_ACTIVITY_TAG)     ? true :
                   (matcher == CURRENT_ACTIVITY_TAG) ? activity == ActivitiesSync::currentActivity(activities) :
                                                       activity == matcher;
        });
    }

    // Same as above, but for agents
    bool agentMatches(const QString &agent) const
    {
        return any_of(query.agents(), [&] (const QString &matcher) {
            return (matcher == ANY_AGENT_TAG)     ? true :
                   (matcher == CURRENT_AGENT_TAG) ? agent == QCoreApplication::applicationName() :
                                                    agent == matcher;
        });
    }

    bool typeMatches(const QString &resource) const
    {
        // We don't necessarily need to retrieve the type from
        // the database. If we do, get it only once
        auto type = make_lazy_val([&] () -> QString {
            using Common::Database;

            auto query
                = Database::instance(Database::ResourcesDatabase,
                                     Database::ReadOnly)
                      ->execQuery("SELECT mimetype FROM ResourceInfo WHERE "
                                  "targettedResource = '" + resource + "'");

            for (const auto &item : query) {
                return item[0].toString();
            }

            return QString();
        });

        return any_of(query.types(), [&] (const QString &matcher) {
            return matcher == ANY_TYPE_TAG || matcher == type;
        });
    }

    bool eventMatches(const QString &agent, const QString &resource,
                      const QString &activity) const
    {
        // The order of checks is not arbitrary, it is sorted
        // from the cheapest, to the most expensive
        return agentMatches(agent)
               && activityMatches(activity)
               && typeMatches(resource);
    }

    void linkResourceToActivity(const QString &agent, const QString &resource,
                                const QString &activity)
    {
        Q_UNUSED(activity)
        Q_UNUSED(agent)
        Q_UNUSED(resource)

        // The used resources do not really care about the linked ones
        if (query.selection() == Terms::UsedResources) return;

        if (!eventMatches(agent, resource, activity)) return;

        emit q->resultAdded(resource, std::numeric_limits<double>::infinity());
    }

    void unlinkResourceFromActivity(const QString &agent,
                                    const QString &resource,
                                    const QString &activity)
    {
        Q_UNUSED(activity)
        Q_UNUSED(agent)
        Q_UNUSED(resource)

        // The used resources do not really care about the linked ones
        if (query.selection() == Terms::UsedResources) return;

        if (!eventMatches(agent, resource, activity)) return;

        emit q->resultRemoved(resource);
    }

    void updateResourceScore(const QString &activity, const QString &agent,
                             const QString &resource, double score)
    {
        Q_ASSERT_X(activity == "00000000-0000-0000-0000-000000000000" ||
                   !QUuid(activity).isNull(),
                   "ResultWatcher::updateResourceScore",
                   "The activity should be always specified here, no magic values");

        // The linked resources do not really care about the stats
        if (query.selection() == Terms::LinkedResources) return;

        if (!eventMatches(agent, resource, activity)) return;

        emit q->resultAdded(resource, score);
    }


    void deleteEarlierStats(QString, int)
    {
        // The linked resources do not really care about the stats
        if (query.selection() == Terms::LinkedResources) return;

        emit q->resultsInvalidated();
    }

    void deleteRecentStats(QString, int, QString)
    {
        // The linked resources do not really care about the stats
        if (query.selection() == Terms::LinkedResources) return;

        emit q->resultsInvalidated();
    }

    QScopedPointer<org::kde::ActivityManager::ResourcesLinking> linking;
    QScopedPointer<org::kde::ActivityManager::ResourcesScoring> scoring;

    ResultWatcher * const q;
    Query query;
};

ResultWatcher::ResultWatcher(Query query)
    : d(new Private(this, query))
{
    using namespace org::kde::ActivityManager;
    using namespace std::placeholders;

    // There is no need for private slots, when we have bind

    // Connecting the linking service
    QObject::connect(
        d->linking.data(), &ResourcesLinking::ResourceLinkedToActivity,
        this, std::bind(&Private::linkResourceToActivity, d, _1, _2, _3));
    QObject::connect(
        d->linking.data(), &ResourcesLinking::ResourceUnlinkedFromActivity,
        this, std::bind(&Private::unlinkResourceFromActivity, d, _1, _2, _3));

    // Connecting the scoring service
    QObject::connect(
        d->scoring.data(), &ResourcesScoring::ResourceScoreUpdated,
        this, std::bind(&Private::updateResourceScore, d, _1, _2, _3, _4));
    QObject::connect(
        d->scoring.data(), &ResourcesScoring::RecentStatsDeleted,
        this, std::bind(&Private::deleteRecentStats, d, _1, _2, _3));
    QObject::connect(
        d->scoring.data(), &ResourcesScoring::EarlierStatsDeleted,
        this, std::bind(&Private::deleteEarlierStats, d, _1, _2));

}

ResultWatcher::~ResultWatcher()
{
    delete d;
}


} // namespace Stats
} // namespace Experimental
} // namespace KActivities

