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

// Self
#include <kactivities-features.h>
#include "ResourceScoreCache.h"

// STD
#include <cmath>

// Utils
#include <utils/d_ptr_implementation.h>
#include <utils/qsqlquery_iterator.h>

// Local
#include "Debug.h"
#include "StatsPlugin.h"
#include "Database.h"
#include "Utils.h"


class ResourceScoreCache::Queries {
private:
    Queries()
        : createResourceScoreCacheQuery(resourcesDatabase().createQuery())
        , getResourceScoreCacheQuery(resourcesDatabase().createQuery())
        , updateResourceScoreCacheQuery(resourcesDatabase().createQuery())
        , getScoreAdditionQuery(resourcesDatabase().createQuery())
    {

        Utils::prepare(resourcesDatabase(),
            createResourceScoreCacheQuery, QStringLiteral(
            "INSERT INTO ResourceScoreCache "
            "VALUES (:usedActivity, :initiatingAgent, :targettedResource, "
                    "0, 0, -1, " // type, score, lastUpdate
                    ":firstUpdate)"
        ));

        Utils::prepare(resourcesDatabase(),
            getResourceScoreCacheQuery, QStringLiteral(
            "SELECT cachedScore, lastUpdate FROM ResourceScoreCache "
            "WHERE "
                ":usedActivity      = usedActivity AND "
                ":initiatingAgent   = initiatingAgent AND "
                ":targettedResource = targettedResource "
        ));

        Utils::prepare(resourcesDatabase(),
            updateResourceScoreCacheQuery, QStringLiteral(
            "UPDATE ResourceScoreCache SET "
                "cachedScore = :cachedScore, "
                "lastUpdate  = :lastUpdate "
            "WHERE "
                ":usedActivity      = usedActivity AND "
                ":initiatingAgent   = initiatingAgent AND "
                ":targettedResource = targettedResource "
        ));

        Utils::prepare(resourcesDatabase(),
            getScoreAdditionQuery, QStringLiteral(
            "SELECT start, end "
            "FROM ResourceEvent "
            "WHERE "
                ":usedActivity      = usedActivity AND "
                ":initiatingAgent   = initiatingAgent AND "
                ":targettedResource = targettedResource AND "
                "start > :start"
        ));
    }

public:
    QSqlQuery createResourceScoreCacheQuery;
    QSqlQuery getResourceScoreCacheQuery;
    QSqlQuery updateResourceScoreCacheQuery;
    QSqlQuery getScoreAdditionQuery;

    static Queries &self();

};

ResourceScoreCache::Queries &ResourceScoreCache::Queries::self()
{
    static Queries queries;
    return queries;
}


class ResourceScoreCache::Private {
public:
    QString activity;
    QString application;
    QString resource;

    inline qreal timeFactor(int days) const
    {
        // Exp is falling rather quickly, we are slowing it 32 times
        return std::exp(-days / 32.0);
    }

    inline qreal timeFactor(QDateTime fromTime,
                     QDateTime toTime = QDateTime::currentDateTime()) const
    {
        return timeFactor(fromTime.daysTo(toTime));
    }
};

ResourceScoreCache::ResourceScoreCache(const QString &activity,
                                       const QString &application,
                                       const QString &resource)
{
    d->activity = activity;
    d->application = application;
    d->resource = resource;
}

ResourceScoreCache::~ResourceScoreCache()
{
}

void ResourceScoreCache::update()
{
    QDateTime lastUpdate;
    qreal score;

    // This can fail if we have the cache already made
    Utils::exec(Queries::self().createResourceScoreCacheQuery,
        ":usedActivity", d->activity,
        ":initiatingAgent", d->application,
        ":targettedResource", d->resource,
        ":firstUpdate", QDateTime::currentDateTime().toTime_t()
    );

    // Getting the old score
    Utils::exec(Queries::self().getResourceScoreCacheQuery,
        ":usedActivity", d->activity,
        ":initiatingAgent", d->application,
        ":targettedResource", d->resource
    );

    // Only and always one result

    for (const auto &result: Queries::self().getResourceScoreCacheQuery) {
        const auto time = result[1].toLongLong();

        if (time < 0) {
            // If we haven't had the cache before, set the score to 0
            lastUpdate = QDateTime();
            score = 0;

        } else {
            // Adjusting the score depending on the time that passed since the
            // last update
            lastUpdate.setTime_t(time);

            score = result[0].toReal();
            score *= d->timeFactor(lastUpdate);
        }
    }

    // Calculating the updated score
    // We are processing all events since the last cache update

    Utils::exec(Queries::self().getScoreAdditionQuery,
        ":usedActivity", d->activity,
        ":initiatingAgent", d->application,
        ":targettedResource", d->resource,
        ":start", lastUpdate.toTime_t()
    );

    qlonglong start = 0;

    for (const auto &result: Queries::self().getScoreAdditionQuery) {
        start = result[0].toLongLong();

        const auto end = result[1].toLongLong();
        const auto intervalLength = end - start;

        if (intervalLength == 0) {
            // We have an Accessed event - otherwise, this wouldn't be 0
            score += d->timeFactor(QDateTime::fromTime_t(end)); // like it is open for 1 minute

        } else if (intervalLength >= 4) {
            // Ignoring stuff that was open for less than 4 seconds
            score += d->timeFactor(QDateTime::fromTime_t(end)) * intervalLength / 60.0;
        }
    }

    // Updating the score

    Utils::exec(Queries::self().updateResourceScoreCacheQuery,
        ":usedActivity", d->activity,
        ":initiatingAgent", d->application,
        ":targettedResource", d->resource,
        ":cachedScore", score,
        ":lastUpdate", start
    );

    // Notifying the world

    QMetaObject::invokeMethod(StatsPlugin::self(),
                              "resourceScoreUpdated",
                              Qt::QueuedConnection,
                              Q_ARG(QString, d->activity),
                              Q_ARG(QString, d->application),
                              Q_ARG(QString, d->resource),
                              Q_ARG(double, score));
}
