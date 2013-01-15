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

#include "ResourceScoreCache.h"
#include "StatsPlugin.h"
#include "DatabaseConnection.h"

#include <QDebug>

#include <config-features.h>

#include <utils/d_ptr_implementation.h>

/**
 *
 */
class ResourceScoreCache::Private {
public:
    QString activity;
    QString application;
    QUrl resource;

};

ResourceScoreCache::ResourceScoreCache(const QString & activity, const QString & application, const QUrl & resource)
    : d()
{
    qDebug() << "Going to update score for"
        << activity << application << resource;

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
            d->activity, d->application, d->resource,
            score, lastUpdate);

    qDebug() << "Sending resourceScoreUpdated event";
    QMetaObject::invokeMethod(StatsPlugin::self(), "resourceScoreUpdated",
            Q_ARG(QString, d->activity),
            Q_ARG(QString, d->application),
            Q_ARG(QString, d->resource.toString()),
            Q_ARG(double, score)
        );
}
