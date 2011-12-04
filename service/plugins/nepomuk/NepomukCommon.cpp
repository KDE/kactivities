/*
 *   Copyright (C) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include <KDebug>

#include <Soprano/Vocabulary/NAO>
#include "kext.h"

using namespace Nepomuk::Vocabulary;
using namespace Soprano::Vocabulary;

QHash < QString, Nepomuk::Resource* > NepomukPluginCommon::activityResources;

Nepomuk::Resource & activityResource(const QString & id)
{
    kDebug() << "Getting the resource of activity:" << id;

    if (!NepomukPluginCommon::activityResources.contains(id)) {
        kDebug() << "We don't have it in the cache";
        const QString & query = QString::fromLatin1(
                "select ?activity where { "
                "?activity a kext:Activity . "
                "?activity nao:identifier %1 ."
                "} LIMIT 1"
                ).arg(litN3(id));

        Soprano::QueryResultIterator it
            = Nepomuk::ResourceManager::instance()->mainModel()->executeQuery(query, Soprano::Query::QueryLanguageSparql);

        Nepomuk::Resource * resource = NULL;

        if (it.next()) {
            resource = new Nepomuk::Resource(it[0].uri());
            it.close();

        } else {
            kDebug() << "A very strange thing is happening - seems like" << id <<
                "activity doesn't exist stored in Nepomuk!";

            resource = new Nepomuk::Resource(id, KExt::Activity());
        }

        // Adding to cache and returning the value
        NepomukPluginCommon::activityResources[id] = resource;

    }

    // Otherwise
    Nepomuk::Resource * result = NepomukPluginCommon::activityResources[id];
    kDebug() << "Returning the resource object"
        << result->genericLabel()
        << result->property(NAO::identifier());
    return *result;
}
