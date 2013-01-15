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

#include <Soprano/Vocabulary/NAO>
#include <Soprano/QueryResultIterator>
#include <Soprano/Node>
#include <Soprano/Model>

#include <Nepomuk2/Vocabulary/NIE>
#include <Nepomuk2/Resource>
#include <Nepomuk2/ResourceManager>
#include <Nepomuk2/Variant>

#include "kao.h"

namespace Nepomuk = Nepomuk2;
using namespace KDE::Vocabulary;
using namespace Nepomuk::Vocabulary;
using namespace Soprano::Vocabulary;

void updateNepomukScore(const QString & activity, const QString & application, const QUrl & resource, qreal score);

QUrl resourceForUrl(const QUrl & url);

QUrl resourceForId(const QString & resourceId, const QUrl & type);

inline QString resN3(const QUrl & uri)
{
    return Soprano::Node::resourceToN3(uri);
}

inline Nepomuk::Resource activityResource(const QString & activity)
{
    Q_ASSERT(!activity.isEmpty());

    return Nepomuk::Resource(activity, KAO::Activity());
}

#endif // PLUGINS_SQLITE_NEPOMUK_COMMON_H

