/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
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

#define ACTIVITY_MANAGER_SERVICE "org.kde.ActivityManager"
#define ACTIVITY_MANAGER_OBJECT_TYPE(A) ACTIVITY_MANAGER_SERVICE #A
#define ACTIVITY_MANAGER_OBJECT_PATH(A) "/ActivityManager/" #A

#include <qwindowdefs.h>

__inline int toInt(WId wid)
{
#ifdef Q_OS_WIN64 // krazy:skip
    return (int)((__int64)wid);
#else
    return (int)wid;
#endif
}

