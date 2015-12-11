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
                    "0, 0, " // type, score
                    ":firstUpdate, " // lastUpdate
                    ":firstUpdate)"
        ));

        Utils::prepare(resourcesDatabase(),
            getResourceScoreCacheQuery, QStringLiteral(
            "SELECT cachedScore, lastUpdate, firstUpdate FROM ResourceScoreCache "
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
                "start > :start "
            "ORDER BY "
                "start ASC"
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

    inline qreal timeFactor(QDateTime fromTime, QDateTime toTime) const
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

    Q_ASSERT_X(!d->application.isEmpty(),
               "ResourceScoreCache::constructor",
               "Agent shoud not be empty");
    Q_ASSERT_X(!d->activity.isEmpty(),
               "ResourceScoreCache::constructor",
               "Activity shoud not be empty");
    Q_ASSERT_X(!d->resource.isEmpty(),
               "ResourceScoreCache::constructor",
               "Resource shoud not be empty");

}

ResourceScoreCache::~ResourceScoreCache()
{
}

void ResourceScoreCache::update()
{
    QDateTime lastUpdate;
    QDateTime firstUpdate;
    QDateTime currentTime = QDateTime::currentDateTime();
    qreal score = 0;

    DATABASE_TRANSACTION(resourcesDatabase());

    qDebug() << "Creating the cache for: " << d->resource;

    // This can fail if we have the cache already made
    auto isCacheNew = Utils::exec(
        Utils::IgnoreError, Queries::self().createResourceScoreCacheQuery,
        ":usedActivity", d->activity,
        ":initiatingAgent", d->application,
        ":targettedResource", d->resource,
        ":firstUpdate", currentTime.toTime_t()
    );

    // Getting the old score
    Utils::exec(
        Utils::FailOnError, Queries::self().getResourceScoreCacheQuery,
        ":usedActivity", d->activity,
        ":initiatingAgent", d->application,
        ":targettedResource", d->resource
    );

    // Only and always one result
    for (const auto &result: Queries::self().getResourceScoreCacheQuery) {

        lastUpdate.setTime_t(result["lastUpdate"].toUInt());
        firstUpdate.setTime_t(result["firstUpdate"].toUInt());

        qDebug() << "Already in database? " << (!isCacheNew);
        qDebug() << "      First update : " << firstUpdate;
        qDebug() << "       Last update : " << lastUpdate;

        if (isCacheNew) {
            // If we haven't had the cache before, set the score to 0
            firstUpdate = currentTime;
            score = 0;

        } else {
            // Adjusting the score depending on the time that passed since the
            // last update
            score = result["cachedScore"].toReal();
            score *= d->timeFactor(lastUpdate, currentTime);
        }
    }

    // Calculating the updated score
    // We are processing all events since the last cache update

    qDebug() << "After the adjustment";
    qDebug() << "     Current score : " << score;
    qDebug() << "      First update : " << firstUpdate;
    qDebug() << "       Last update : " << lastUpdate;

    Utils::exec(Utils::FailOnError, Queries::self().getScoreAdditionQuery,
        ":usedActivity", d->activity,
        ":initiatingAgent", d->application,
        ":targettedResource", d->resource,
        ":start", lastUpdate.toTime_t()
    );

    uint lastEventStart = currentTime.toTime_t();

    for (const auto &result: Queries::self().getScoreAdditionQuery) {
        lastEventStart = result["start"].toUInt();

        const auto end = result["end"].toUInt();
        const auto intervalLength = end - lastEventStart;

        qDebug() << "Interval length is " << intervalLength;

        if (intervalLength == 0) {
            // We have an Accessed event - otherwise, this wouldn't be 0
            score += d->timeFactor(QDateTime::fromTime_t(end), currentTime); // like it is open for 1 minute

        } else {
            score += d->timeFactor(QDateTime::fromTime_t(end), currentTime) * intervalLength / 60.0;

        }
    }

    qDebug() << "         New score : " << score;

    // Updating the score

    Utils::exec(Utils::FailOnError, Queries::self().updateResourceScoreCacheQuery,
        ":usedActivity", d->activity,
        ":initiatingAgent", d->application,
        ":targettedResource", d->resource,
        ":cachedScore", score,
        ":lastUpdate", lastEventStart
    );

    // Notifying the world
    emit QMetaObject::invokeMethod(StatsPlugin::self(),
                                   "ResourceScoreUpdated",
                                   Qt::QueuedConnection,
                                   Q_ARG(QString, d->activity),
                                   Q_ARG(QString, d->application),
                                   Q_ARG(QString, d->resource),
                                   Q_ARG(double, score),
                                   Q_ARG(uint, lastEventStart),
                                   Q_ARG(uint, firstUpdate.toTime_t())
                                   );
}
