/*
 *   Copyright (C) 2011 Ivan Cukic <ivan.cukic(at)kde.org>
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


Event::Event(const QString & vApplication, WId vWid, const QString & vUri, int vType, int vReason)
    : application(vApplication), wid(vWid), uri(vUri), type(vType), reason(vReason), timestamp(QDateTime::currentDateTime())
{
}

bool Event::operator == (const Event & other) const
{
    return
        application == other.application &&
        wid         == other.wid         &&
        uri         == other.uri         &&
        type        == other.type        &&
        reason      == other.reason;
}
