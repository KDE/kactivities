/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "version.h"

namespace KActivities {

unsigned int version()
{
    return KACTIVITIES_VERSION;
}

unsigned int versionMajor()
{
    return KACTIVITIES_VERSION_MAJOR;
}

unsigned int versionMinor()
{
    return KACTIVITIES_VERSION_MINOR;
}

unsigned int versionRelease()
{
    return KACTIVITIES_VERSION_RELEASE;
}

const char *versionString()
{
    return KACTIVITIES_VERSION_STRING;
}

} // KActivities namespace
