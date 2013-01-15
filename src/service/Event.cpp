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

#include "Event.h"

#include <QDebug>
#include <common.h>


Event::Event()
    : wid(0), type(Accessed), reason(User), timestamp(QDateTime::currentDateTime())
{
}

Event::Event(const QString & vApplication, WId vWid, const QString & vUri, int vType, int vReason)
    : application(vApplication), wid(vWid), uri(vUri), type(vType), reason(vReason), timestamp(QDateTime::currentDateTime())
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

bool Event::operator == (const Event & other) const
{
    return
        application == other.application &&
        wid == other.wid &&
        uri == other.uri &&
        type == other.type &&
        reason == other.reason &&
        timestamp == other.timestamp;
}

QString Event::typeName() const
{
    switch (type) {
        case Accessed:    return "Accessed";
        case Opened:      return "Opened";
        case Modified:    return "Modified";
        case Closed:      return "Closed";
        case FocussedIn:  return "FocussedIn";
        case FocussedOut: return "FocussedOut";
        default:          return "Other";
    }

}

QDebug operator << (QDebug dbg, const Event & e)
{
#ifndef QT_NO_DEBUG_OUTPUT
    dbg << "Event(" << e.application << e.wid << e.typeName() << e.uri << ":" << e.timestamp << ")";
#else
    Q_UNUSED(e)
#endif
    return dbg.space();
}

