/*
 *   Copyright (C) 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "NepomukCommon.h"

#include <utils/val.h>

QUrl resourceForUrl(const QUrl & url)
{
    static val & query = QString::fromLatin1(
            "select ?r where { "
                "?r nie:url %1 . "
            "} LIMIT 1");

    Soprano::QueryResultIterator it =
        Nepomuk::ResourceManager::instance()->mainModel()->executeQuery(
            query.arg(Soprano::Node::resourceToN3(url)), Soprano::Query::QueryLanguageSparql);

    if (it.next()) {
        return it[0].uri();

    } else {
        Nepomuk::Resource resource(url);
        resource.setProperty(NIE::url(), url);

        // Add more data to

        return resource.uri();
    }
}

QUrl resourceForId(const QString & resourceId, const QUrl & type)
{
    static val & _query = QString::fromLatin1(
            "select ?r where { "
                "?r a %1 . "
                "?r nao:identifier %2 . "
            "} LIMIT 1");

    val & query = _query.arg(
            /* %1 */ Soprano::Node::resourceToN3(type),
            /* %2 */ Soprano::Node::literalToN3(resourceId)
        );

    Soprano::QueryResultIterator it =
        Nepomuk::ResourceManager::instance()->mainModel()->executeQuery(
            query, Soprano::Query::QueryLanguageSparql);

    if (it.next()) {
        return it[0].uri();

    } else {
        Nepomuk::Resource agent(QUrl(), type);
        agent.setProperty(NAO::identifier(), resourceId);

        return agent.uri();
    }
}

void updateNepomukScore(const QString & activity, const QString & application, const QUrl & resource, qreal score)
{
#ifdef NEPOMUK_STORE_RESOURCE_SCORES
    Nepomuk::Resource scoreCache;

    // Selecting a ResourceScoreCache object that is assigned to the specified
    // (activity, application, resource) triple
    static val & _query = QString::fromLatin1("select ?r where { "
                                    "?r a %1 . "
                                    "?r kao:usedActivity %2 . "
                                    "?r kao:initiatingAgent %3 . "
                                    "?r kao:targettedResource %4 . "
                                    "} LIMIT 1"
            );

    val query = _query.arg(
                /* %1 */ resN3(KAO::ResourceScoreCache()),
                /* %2 */ resN3(resourceForId(activity, KAO::Activity())),
                /* %3 */ resN3(resourceForId(application, NAO::Agent())),
                /* %4 */ resN3(resourceForUrl(resource))
            );

    auto it = Nepomuk::ResourceManager::instance()->mainModel()->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    // If it already exists - lucky us
    // If it does not - we need to create a new one
    if (it.next()) {
        Nepomuk::Resource result(it[0].uri());
        it.close();

        scoreCache = result;

    } else {
        Nepomuk::Resource result(QUrl(), KAO::ResourceScoreCache());

        result.setProperty(KAO::targettedResource(), resourceForUrl(resource));
        result.setProperty(KAO::initiatingAgent(),   resourceForId(application, NAO::Agent()));
        result.setProperty(KAO::usedActivity(),      resourceForId(activity, KAO::Activity()));

        scoreCache = result;
    }

    // If the score is strictly positive, we are saving it in nepomuk,
    // otherwise we are deleting the score cache since the negative
    // and zero scores have no use.
    // This is (mis)used when clearing the usage history - the
    // Scoring object will send us a score smaller than zero
    if (score > 0) {
        scoreCache.removeProperty(NAO::score());
        scoreCache.removeProperty(KAO::cachedScore());
        scoreCache.setProperty(KAO::cachedScore(), score);

    } else {
        scoreCache.remove();

    }
#endif
}

