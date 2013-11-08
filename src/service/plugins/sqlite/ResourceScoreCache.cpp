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

// Utils
#include <utils/d_ptr_implementation.h>

// Local
#include "Debug.h"
#include "StatsPlugin.h"
#include "DatabaseConnection.h"


/**
 *
 */
class ResourceScoreCache::Private {
public:
    QString activity;
    QString application;
    QString resource;
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

void ResourceScoreCache::updateScore()
{
    QDateTime lastUpdate;
    qreal score;

    DatabaseConnection::self()->getResourceScoreCache(
        d->activity, d->application, d->resource, score, lastUpdate);

    QMetaObject::invokeMethod(StatsPlugin::self(), "resourceScoreUpdated",
                              Q_ARG(QString, d->activity),
                              Q_ARG(QString, d->application),
                              Q_ARG(QString, d->resource),
                              Q_ARG(double, score));
}
