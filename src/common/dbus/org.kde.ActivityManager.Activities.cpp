/*
 *   Copyright (C) 2010 - 2016 by Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.
 *   If not, see <http://www.gnu.org/licenses/>.
 */

#include "org.kde.ActivityManager.Activities.h"

#include <QMetaType>
#include <QDBusMetaType>

namespace details {

class ActivityInfoStaticInit {
public:
    ActivityInfoStaticInit()
    {
        qDBusRegisterMetaType<ActivityInfo>();
        qDBusRegisterMetaType<ActivityInfoList>();
    }

    static ActivityInfoStaticInit _instance;
};

ActivityInfoStaticInit ActivityInfoStaticInit::_instance;

} // namespace details

QDBusArgument &operator<<(QDBusArgument &arg, const ActivityInfo r)
{
    arg.beginStructure();

    arg << r.id;
    arg << r.name;
    arg << r.description;
    arg << r.icon;
    arg << r.state;

    arg.endStructure();

    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, ActivityInfo &r)
{
    arg.beginStructure();

    arg >> r.id;
    arg >> r.name;
    arg >> r.description;
    arg >> r.icon;
    arg >> r.state;

    arg.endStructure();

    return arg;
}

QDebug operator<<(QDebug dbg, const ActivityInfo &r)
{
    dbg << "ActivityInfo(" << r.id << r.name << ")";
    return dbg.space();
}
