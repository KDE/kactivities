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

#ifndef PLUGINS_SQLITE_NEPOMUK_COMMON_H
#define PLUGINS_SQLITE_NEPOMUK_COMMON_H

#include <config-features.h>

#ifdef HAVE_NEPOMUK

#include <nepomuk/nie.h>
#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>
#include <Nepomuk/Variant>

#include "kao.h"
#include <Soprano/Vocabulary/NAO>
#include <Soprano/QueryResultIterator>
#include <Soprano/Node>
#include <Soprano/Model>

using namespace Nepomuk::Vocabulary;
using namespace Soprano::Vocabulary;

#define agentResource(ID)    Nepomuk::Resource(ID, NAO::Agent())

inline Nepomuk::Resource anyResource(const QUrl & uri)
{
    Nepomuk::Resource result(uri);
    result.setProperty(NIE::url(), uri);
    return result;
}

inline Nepomuk::Resource anyResource(const QString & uri)
{
    return anyResource(KUrl(uri));
}

// #define litN3(A) Soprano::Node::literalToN3(A)

inline QString resN3(const QUrl & uri)
{
    return Soprano::Node::resourceToN3(uri);
}

inline QString resN3(const Nepomuk::Resource & resource)
{
    return Soprano::Node::resourceToN3(resource.resourceUri());
}

inline void updateNepomukScore(const QString & activity, const QString & application, const QUrl & resource, qreal score)
{
    Nepomuk::Resource scoreCache;

    const QString query
        = QString::fromLatin1("select ?r where { "
                                  "?r a %1 . "
                                  "?r kao:usedActivity %2 . "
                                  "?r kao:initiatingAgent %3 . "
                                  "?r kao:targettedResource %4 . "
                                  "} LIMIT 1"
            ).arg(
                /* %1 */ resN3(KAO::ResourceScoreCache()),
                /* %2 */ resN3(Nepomuk::Resource(activity, KAO::Activity())),
                /* %3 */ resN3(agentResource(application)),
                /* %4 */ resN3(anyResource(resource))
            );

    Soprano::QueryResultIterator it
        = Nepomuk::ResourceManager::instance()->mainModel()->executeQuery(query, Soprano::Query::QueryLanguageSparql);

    if (it.next()) {
        Nepomuk::Resource result(it[0].uri());
        it.close();

        scoreCache = result;

    } else {
        Nepomuk::Resource result(QUrl(), KAO::ResourceScoreCache());

        result.setProperty(KAO::targettedResource(), Nepomuk::Resource(resource));
        result.setProperty(KAO::initiatingAgent(),   agentResource(application));
        result.setProperty(KAO::usedActivity(),      Nepomuk::Resource(activity, KAO::Activity()));

        scoreCache = result;
    }

    scoreCache.setProperty(KAO::cachedScore(), score);


}

#endif // HAVE_NEPOMUK

#endif // PLUGINS_SQLITE_NEPOMUK_COMMON_H

