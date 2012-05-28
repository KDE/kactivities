/*
 *   Copyright (C) 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "ActivityManager.h"
#include "ActivityManager_p.h"

#include <KWindowSystem>

#include "EventProcessor.h"

// Private

// copied from kdelibs\kdeui\notifications\kstatusnotifieritemdbus_p.cpp
// if there is a common place for such definitions please move
#ifdef Q_OS_WIN64 // krazy:skip
__inline int toInt(WId wid)
{
    return (int)((__int64)wid);
}

#else
__inline int toInt(WId wid)
{
    return (int)wid;
}
#endif

void ActivityManagerPrivate::windowClosed(WId windowId)
{
    // kDebug() << "Window closed..." << windowId
    //          << "one of ours?" << windows.contains(windowId);

    if (!windows.contains(windowId)) {
        return;
    }

    foreach (const KUrl & uri, windows[windowId].resources) {
        q->RegisterResourceEvent(windows[windowId].application,
                toInt(windowId), uri.url(), Event::Closed, resources[uri].reason);
    }
}

void ActivityManagerPrivate::activeWindowChanged(WId windowId)
{
    Q_UNUSED(windowId)
    // kDebug() << "Window focussed..." << windowId
    //          << "one of ours?" << windows.contains(windowId);

}

