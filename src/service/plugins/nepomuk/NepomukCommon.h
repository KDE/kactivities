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

#ifndef PLUGINS_SQLITE_NEPOMUK_COMMON_H
#define PLUGINS_SQLITE_NEPOMUK_COMMON_H

#include <config-features.h>

#ifdef HAVE_NEPOMUK

#include <Soprano/Vocabulary/NAO>
#include <Soprano/QueryResultIterator>
#include <Soprano/Node>
#include <Soprano/Model>

#include <Nepomuk2/Vocabulary/NIE>
#include <Nepomuk2/Resource>
#include <Nepomuk2/ResourceManager>
#include <Nepomuk2/Variant>

#include "kao.h"

#include <utils/val.h>

namespace Nepomuk = Nepomuk2;
using namespace KDE::Vocabulary;
using namespace Nepomuk::Vocabulary;
using namespace Soprano::Vocabulary;

inline QUrl resourceForUrl(const QUrl & url)
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

inline QUrl resourceForId(const QString & id, const QUrl & type)
{
    static val & _query = QString::fromLatin1(
            "select ?r where { "
                "?r a %1 . "
                "?r nao:identifier %2 . "
            "} LIMIT 1");

    val & query = _query.arg(
            /* %1 */ Soprano::Node::resourceToN3(type),
            /* %2 */ Soprano::Node::literalToN3(id)
        );

    Soprano::QueryResultIterator it =
        Nepomuk::ResourceManager::instance()->mainModel()->executeQuery(
            query, Soprano::Query::QueryLanguageSparql);

    if (it.next()) {
        return it[0].uri();

    } else {
        Nepomuk::Resource agent(QUrl(), type);
        agent.setProperty(NAO::identifier(), id);

        return agent.uri();
    }
}


inline QString resN3(const QUrl & uri)
{
    return Soprano::Node::resourceToN3(uri);
}

inline void updateNepomukScore(const QString & activity, const QString & application, const QUrl & resource, qreal score)
{
    Nepomuk::Resource scoreCache;

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

    scoreCache.removeProperty(NAO::score());
    scoreCache.removeProperty(KAO::cachedScore());
    scoreCache.setProperty(KAO::cachedScore(), score);
}

#endif // HAVE_NEPOMUK
#endif // PLUGINS_SQLITE_NEPOMUK_COMMON_H

