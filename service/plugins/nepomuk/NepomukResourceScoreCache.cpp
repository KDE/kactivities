/*
 *   Copyright (C) 2011 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "NepomukResourceScoreCache.h"

#include <cmath>

#include <Soprano/Model>
#include <Soprano/QueryResultIterator>

#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Variant>

#include <KDebug>

#include "kext.h"
#include "nao.h"
#include <nepomuk/nuao.h>

#include "ActivityManager.h"

using namespace Nepomuk::Vocabulary;

#include "NepomukCommon.h"
#include "NepomukPlugin.h"

/**
 *
 */
class NepomukResourceScoreCachePrivate {
public:
    Nepomuk::Resource self;
    QString activity;
    QString application;
    QUrl resource;

    qreal timeFactor(int days) const
    {
        // Exp is falling rather quickly, we are slowing it 32 times
        return ::exp(- days / 32.0);
    }

    qreal timeFactor(QDateTime fromTime, QDateTime toTime = QDateTime::currentDateTime()) const
    {
        return timeFactor(fromTime.daysTo(toTime));
    }

};

NepomukResourceScoreCache::NepomukResourceScoreCache(const QString & activity, const QString & application, const QUrl & resource)
    : d(new NepomukResourceScoreCachePrivate())
{
    // kDebug() << "Cache for" << activity << application << resource << anyResource(resource).resourceUri();

    d->activity = activity;
    d->application = application;
    d->resource = resource;

    const QString query
        = QString::fromLatin1("select ?r where { "
                                  "?r a %1 . "
                                  "?r kext:usedActivity %2 . "
                                  "?r kext:initiatingAgent %3 . "
                                  "?r kext:targettedResource %4 . "
                                  "} LIMIT 1"
            ).arg(
                /* %1 */ resN3(KExt::ResourceScoreCache()),
                /* %2 */ resN3(currentActivityRes),
                /* %3 */ resN3(agentResource(application)),
                /* %4 */ resN3(anyResource(resource))
            );

    // kDebug() << query;

    Soprano::QueryResultIterator it
        = Nepomuk::ResourceManager::instance()->mainModel()->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    if (it.next()) {
        Nepomuk::Resource result(it[0].uri());
        it.close();

        d->self = result;

        // kDebug() << "Found an old cache" << d->self.resourceUri() << d->self.resourceType()
        //          << "With a score of" << d->self.property(KExt::cachedScore()) << d->self.property(NAO::score());

    } else {
        Nepomuk::Resource result(QUrl(), KExt::ResourceScoreCache());

        result.setProperty(
                KExt::targettedResource(),
                Nepomuk::Resource(resource)
            );
        result.setProperty(
                KExt::initiatingAgent(), agentResource(application)
            );
        result.setProperty(
                KExt::usedActivity(), activityResource(activity)
            );
        result.setProperty(KExt::cachedScore(), 0);

        d->self = result;

        // kDebug() << "Created a new cache resource" << d->self.resourceUri() << d->self.resourceType();

    }
}

NepomukResourceScoreCache::~NepomukResourceScoreCache()
{
    delete d;
}

void NepomukResourceScoreCache::updateScore()
{
    // kDebug() << "Updating the score for " << d->resource;
    // kDebug() << "Last modified as string" << d->self.property(NAO::lastModified());

    QDateTime lastModified = d->self.property(NAO::lastModified()).toDateTime();

    qreal score = d->self.property(KExt::cachedScore()).toDouble();

    if (lastModified.isValid()) {
        // Adjusting the score depending on the time that passed since the
        // last update

        // kDebug() << "Previous score:" << score;
        score *= d->timeFactor(lastModified);
        // kDebug() << "Adjusted score:" << score;

    } else {
        // If we haven't had previous calculation, set the score to 0
        score = 0;

    }

    // kDebug() << "Last modified timestamp is" << lastModified << lastModified.isValid();

    const QString query
        = QString::fromLatin1("select distinct ?r where { "
                                  "?r a nuao:DesktopEvent . "
                                  "?r %1 %2 . "
                                  "?r %3 %4 . "
                                  "?r %5 %6 . "
                                  "?r nuao:end ?end . "
                                  "FILTER(?end >= %7) ."
                                  " } "
            ).arg(
                /* %1 */ resN3(KExt::usedActivity()),
                /* %2 */ resN3(activityResource(d->activity)),
                /* %3 */ resN3(NUAO_targettedResource),
                /* %4 */ resN3(anyResource(d->resource)),
                /* %5 */ resN3(NUAO_initiatingAgent),
                /* %6 */ resN3(agentResource(d->application)),
                /* %7 */ litN3(lastModified.isValid() ? lastModified : QDateTime::fromMSecsSinceEpoch(0))
            );

    // kDebug() << query;

    Soprano::QueryResultIterator it
        = Nepomuk::ResourceManager::instance()->mainModel()->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    d->self.setProperty(NAO::lastModified(), QDateTime::currentDateTime());

    while (it.next()) {
        Nepomuk::Resource result(it[0].uri());
        QDateTime eventStart = result.property(NUAO::start()).toDateTime();
        QDateTime eventEnd = result.property(NUAO::end()).toDateTime();

        if (!eventStart.isValid()) continue;

        if (!eventEnd.isValid()) {
            // If the end was not saved, we are treating it as a simple
            // Accessed event
            eventEnd = eventStart;
        }

        int intervalLength = eventStart.secsTo(eventEnd);

        if (intervalLength == 0) {
            // We have an Accessed event - otherwise, this wouldn't be 0

            score += d->timeFactor(eventEnd); // like it is open for 1 minute

        } else if (intervalLength >= 4) {
            // Ignoring stuff that was open for less than 4 seconds

            score += d->timeFactor(eventEnd) * intervalLength / 60.0;
        }

        // kDebug() << result.resourceUri() << eventStart << eventEnd << intervalLength;

    }

    // kDebug() << "New calculated score:" << score << d->self.isValid();
    d->self.setProperty(KExt::cachedScore(), score);
    d->self.setProperty(NAO::score(), score);

    Rankings::self()->resourceScoreUpdated(d->activity, d->application, d->resource, score);
}
