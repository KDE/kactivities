/*
    SPDX-FileCopyrightText: 2008-2016 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KACTIVITIES_VERSION_BIN_H
#define KACTIVITIES_VERSION_BIN_H

/** @file version.h <KActivities/Version> */

#include "kactivities_export.h"
#include <kactivities_version.h>

#define KACTIVITIES_VERSION_RELEASE KACTIVITIES_VERSION_PATCH

/**
 * Namespace for everything in libkactivities
 */
namespace KActivities
{
/**
 * The runtime version of libkactivities
 */
KACTIVITIES_EXPORT unsigned int version();

/**
 * The runtime major version of libkactivities
 */
KACTIVITIES_EXPORT unsigned int versionMajor();

/**
 * The runtime major version of libkactivities
 */
KACTIVITIES_EXPORT unsigned int versionMinor();

/**
 * The runtime major version of libkactivities
 */
KACTIVITIES_EXPORT unsigned int versionRelease();

/**
 * The runtime version string of libkactivities
 */
KACTIVITIES_EXPORT const char *versionString();

} // KActivities namespace

#endif // multiple inclusion guard
