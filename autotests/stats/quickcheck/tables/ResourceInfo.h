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

#ifndef RESOURCEINFO_TABLE_H
#define RESOURCEINFO_TABLE_H

#include <QString>

#include "common.h"

namespace ResourceInfo {
    struct Item {
        QString targettedResource;
        QString title;
        QString mimetype;

        const QString &primaryKey() const
        {
            return targettedResource;
        }

    };

    DECL_COLUMN(QString, targettedResource);
    DECL_COLUMN(QString, title);
    DECL_COLUMN(QString, mimetype);

}

#endif // RESOURCEINFO_TABLE_H

