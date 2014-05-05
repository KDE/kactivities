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

// Qt
#include <QString>

// Utils
#include <utils/d_ptr.h>

/**
 * ResourceScoreCache handles the persistence of the usage ratings for
 * the resources.
 *
 * It contains the logic to update the score of a single resource.
 */
class ResourceScoreCache {
public:
    ResourceScoreCache(const QString &activity, const QString &application,
                       const QString &resource);
    virtual ~ResourceScoreCache();

    void update();

private:
    D_PTR;
    class Queries;
};

#endif // PLUGINS_SQLITE_RESOURCE_SCORE_CACHE_H
