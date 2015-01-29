/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef RESOURCELINK_TABLE_H
#define RESOURCELINK_TABLE_H

#include <QString>

#include "common.h"

namespace ResourceLink {
    struct Item {
        QString usedActivity;
        QString initiatingAgent;
        QString targettedResource;

        inline std::tuple<const QString &, const QString &, const QString &>
        primaryKey() const
        {
            return std::tie(targettedResource, usedActivity, initiatingAgent);
        }
    };

    DECL_COLUMN(QString, usedActivity);
    DECL_COLUMN(QString, initiatingAgent);
    DECL_COLUMN(QString, targettedResource);

    template <typename Range>
    inline std::vector<Item> groupByResource(const Range &range)
    {
        return groupBy(range, &Item::targettedResource,
                       [](Item &acc, const Item &item) {
                           acc.usedActivity += item.usedActivity + ' ';
                           acc.initiatingAgent += item.initiatingAgent + ' ';
                       });
    }

} // namespace ResourceLink

#endif // RESOURCELINK_TABLE_H
