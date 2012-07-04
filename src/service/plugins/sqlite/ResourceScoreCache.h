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

#ifndef PLUGINS_SQLITE_RESOURCE_SCORE_CACHE_H
#define PLUGINS_SQLITE_RESOURCE_SCORE_CACHE_H

#include <QString>
#include <QUrl>

#include <utils/d_ptr.h>

class ResourceScoreCachePrivate;

/**
 *
 */
class ResourceScoreCache {
public:
    ResourceScoreCache(const QString & activity, const QString & application, const QUrl & resource);
    virtual ~ResourceScoreCache();

    void updateScore();

private:
    D_PTR;
};

#endif // PLUGINS_SQLITE_RESOURCE_SCORE_CACHE_H
