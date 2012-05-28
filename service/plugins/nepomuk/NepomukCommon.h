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

#ifndef PLUGINS_NEPOMUK_NEPOMUK_COMMON_H
#define PLUGINS_NEPOMUK_NEPOMUK_COMMON_H

#include <nepomuk/nie.h>
#include "NepomukPlugin.h"

#include <QHash>
#include <QString>

#include <Soprano/QueryResultIterator>
#include <Soprano/Node>
#include <Soprano/Model>

#include <Nepomuk/Variant>
#include <Nepomuk/ResourceManager>

#define NUAO_targettedResource KUrl(NUAO::nuaoNamespace().toString() + QLatin1String("targettedResource"))
#define NUAO_initiatingAgent   KUrl(NUAO::nuaoNamespace().toString() + QLatin1String("initiatingAgent"))
// #define NUAO_involvesActivity  KUrl(NUAO::nuaoNamespace().toString() + QLatin1String("involvesActivity"))

// #define activityResource(ID) Nepomuk::Resource(ID, KAO::Activity())
#define agentResource(ID)    Nepomuk::Resource(ID, NAO::Agent())
#define currentActivityId    NepomukPlugin::self()->sharedInfo()->currentActivity()
#define currentActivityRes   activityResource(currentActivityId)

// #define anyResource(ID) Nepomuk::Resource(KUrl(ID))

inline Nepomuk::Resource anyResource(const QUrl & uri)
{
    Nepomuk::Resource result(uri);

    // kDebug() << uri << "setting the URI" << result.isFile() << result.isValid();
    result.setProperty(Nepomuk::Vocabulary::NIE::url(), uri);
    // kDebug() << "set the URI" << result.isFile() << result.isValid();

    return result;
}

inline Nepomuk::Resource anyResource(const QString & uri)
{
    return anyResource(KUrl(uri));
}

#define litN3(A) Soprano::Node::literalToN3(A)

inline QString resN3(const QUrl & uri)
{
    return Soprano::Node::resourceToN3(uri);
}

inline QString resN3(const Nepomuk::Resource & resource)
{
    return Soprano::Node::resourceToN3(resource.resourceUri());
}

class NepomukPluginCommon {
public:
    static QHash < QString, Nepomuk::Resource * > activityResources;
};

Nepomuk::Resource & activityResource(const QString & id);

#endif // PLUGINS_NEPOMUK_NEPOMUK_COMMON_H
