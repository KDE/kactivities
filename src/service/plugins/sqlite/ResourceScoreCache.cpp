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
#include <utils/qsqlquery.h>

// Local
#include "Debug.h"
#include "StatsPlugin.h"
#include "Database.h"


/**
 *
 */
class ResourceScoreCache::Private {
public:
    QString activity;
    QString application;
    QString resource;

    static const QString createResourceScoreCacheQuery;
    static const QString getResourceScoreCacheQuery;
    static const QString updateResourceScoreCacheQuery;
    static const QString getScoreAdditionQuery;

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

const QString ResourceScoreCache::Private::createResourceScoreCacheQuery
    = QStringLiteral("INSERT INTO kext_ResourceScoreCache VALUES("
                     "'%1', " // usedActivity
                     "'%2', " // initiatingAgent
                     "'%3', " // targettedResource
                     "0," // scoreType
                     "0.0," // cachedScore
                     "-1," // lastUpdate
                     "%4" // firstUpdate
                     ")");

const QString ResourceScoreCache::Private::getResourceScoreCacheQuery
    = QStringLiteral("SELECT cachedScore, lastUpdate FROM kext_ResourceScoreCache "
                     "WHERE "
                     "'%1' = usedActivity AND "
                     "'%2' = initiatingAgent AND "
                     "'%3' = targettedResource ");

const QString ResourceScoreCache::Private::updateResourceScoreCacheQuery
    = QStringLiteral("UPDATE kext_ResourceScoreCache SET "
                     "cachedScore = %4, "
                     "lastUpdate = %5 "
                     "WHERE "
                     "'%1' = usedActivity AND "
                     "'%2' = initiatingAgent AND "
                     "'%3' = targettedResource ");

const QString ResourceScoreCache::Private::getScoreAdditionQuery
    = QStringLiteral("SELECT start, end FROM nuao_DesktopEvent "
                     "WHERE "
                     "'%1' = usedActivity AND "
                     "'%2' = initiatingAgent AND "
                     "'%3' = targettedResource AND "
                     "start > %4");

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

    auto results = Database::self()->exec(
        // This can fail if we have the cache already made
        Private::createResourceScoreCacheQuery
            .arg(d->activity)
            .arg(d->application)
            .arg(d->resource)
            .arg(QDateTime::currentDateTime().toTime_t()),

        // Getting the old score
        Private::getResourceScoreCacheQuery
            .arg(d->activity)
            .arg(d->application)
            .arg(d->resource)
    );

    // Only and always one result

    for (const auto &result: results) {
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

    results = Database::self()->exec(
        Private::getScoreAdditionQuery
            .arg(d->activity)
            .arg(d->application)
            .arg(d->resource)
            .arg(lastUpdate.toTime_t()));

    long start = 0;

    for (const auto &result: results) {
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

    Database::self()->exec(
        Private::updateResourceScoreCacheQuery
            .arg(d->activity)
            .arg(d->application)
            .arg(d->resource)
            .arg(score)
            .arg(start));

    // Notifying the world

    QMetaObject::invokeMethod(StatsPlugin::self(),
                              "resourceScoreUpdated",
                              Qt::QueuedConnection,
                              Q_ARG(QString, d->activity),
                              Q_ARG(QString, d->application),
                              Q_ARG(QString, d->resource),
                              Q_ARG(double, score));
}
