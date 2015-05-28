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
#include "Event.h"

// Local
#include "Debug.h"


Event::Event()
    : wid(0)
    , type(Accessed)
    , timestamp(QDateTime::currentDateTime())
{
}

Event::Event(const QString &vApplication, quintptr vWid, const QString &vUri, int vType)
    : application(vApplication)
    , wid(vWid)
    , uri(vUri)
    , type(vType)
    , timestamp(QDateTime::currentDateTime())
{
    Q_ASSERT(!vApplication.isEmpty());
    Q_ASSERT(!vUri.isEmpty());
}

Event Event::deriveWithType(Type type) const
{
    Event result(*this);
    result.type = type;
    return result;
}

bool Event::operator==(const Event &other) const
{
    return application == other.application && wid == other.wid && uri == other.uri && type == other.type && timestamp == other.timestamp;
}

QString Event::typeName() const
{
    switch (type) {
        case Accessed:
            return QStringLiteral("Accessed");
        case Opened:
            return QStringLiteral("Opened");
        case Modified:
            return QStringLiteral("Modified");
        case Closed:
            return QStringLiteral("Closed");
        case FocussedIn:
            return QStringLiteral("FocussedIn");
        case FocussedOut:
            return QStringLiteral("FocussedOut");
        default:
            return QStringLiteral("Other");
    }
}

QDebug operator<<(QDebug dbg, const Event &e)
{
#ifndef QT_NO_DEBUG_OUTPUT
    dbg << "Event(" << e.application << e.wid << e.typeName() << e.uri << ":" << e.timestamp << ")";
#else
    Q_UNUSED(e);
#endif
    return dbg.space();
}
