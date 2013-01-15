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

#ifndef EVENT_H
#define EVENT_H

#include <QString>
#include <QWidget>
#include <QDateTime>
#include <QMetaType>

/**
 *
 */
class Event {
public:

    enum Type {
        Accessed = 0,    ///< resource was accessed, but we don't know for how long it will be open/used

        Opened = 1,      ///< resource was opened
        Modified = 2,    ///< previously opened resource was modified
        Closed = 3,      ///< previously opened resource was closed

        FocussedIn = 4,  ///< resource get the keyboard focus
        FocussedOut = 5, ///< resource lost the focus

        LastEventType = 5,
        UserEventType = 32

    };

    // These events can't come outside of the activity manager daemon,
    // they are intended to provide some additional functionality
    // to the daemon plugins
    enum UserType {
        UpdateScore = UserEventType + 1

    };

    // TODO: Remove
    // Was introduced for better cooperation with Zeitgeist
    // We don't use it
    enum Reason {
        User = 0,
        Scheduled = 1,
        Heuristic = 2,
        System = 3,
        World = 4,

        LastEventReason = 4,
        UserEventReason = 32
    };

    Event();

    explicit Event(const QString & application, WId wid, const QString & uri,
            int type = Accessed, int reason = User);

    Event deriveWithType(Type type) const;

    bool operator == (const Event & other) const;

public:
    QString application;
    WId     wid;
    QString uri;
    int     type;
    int     reason;
    QDateTime timestamp;

    QString typeName() const;
};

QDebug operator << (QDebug dbg, const Event & e);

typedef QList<Event> EventList;

Q_DECLARE_METATYPE(Event)
Q_DECLARE_METATYPE(EventList)

#endif // EVENT_H

