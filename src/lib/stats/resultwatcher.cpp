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
#include <boost/optional.hpp>

// KActivities
#include <kactivities/consumer.h>

#include "resourceslinking_interface.h"
#include "resourcesscoring_interface.h"
#include "common/dbus/common.h"

namespace KActivities {
namespace Experimental {
namespace Stats {

// Main class

class ResultWatcher::Private {
public:
    Private(ResultWatcher *parent, Query query)
        : linking(new KAMD_DBUS_CLASS_INTERFACE(Resources/Linking, ResourcesLinking, Q_NULLPTR))
        , scoring(new KAMD_DBUS_CLASS_INTERFACE(Resources/Scoring, ResourcesScoring, Q_NULLPTR))
        , q(parent)
        , query(query)
    {
    }

    void linkResourceToActivity(const QString &agent, const QString &resource,
                                const QString &activity)
    {
        Q_UNUSED(activity)
        Q_UNUSED(agent)
        Q_UNUSED(resource)

        // The used resources do not really care about the linked ones
        if (query.selection() == Terms::UsedResources) return;
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
    }

    void updateResourceScore(const QString &activity, const QString &agent,
                             const QString &resource, double score)
    {
        Q_UNUSED(activity)
        Q_UNUSED(agent)
        Q_UNUSED(resource)
        Q_UNUSED(score)

        // The linked resources do not really care about the stats
        if (query.selection() == Terms::LinkedResources) return;

        emit q->resourceScoreChanged(resource, score);
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

