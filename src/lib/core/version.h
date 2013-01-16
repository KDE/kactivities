/*
 *   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KACTIVITIES_VERSION_H
#define KACTIVITIES_VERSION_H

/** @file version.h <KActivities/Version> */

#include "kactivities_export.h"

/**
 * String version of libkactivities version, suitable for use in
 * file formats or network protocols
 */
#define KACTIVITIES_VERSION_STRING "6.2.0"

/// @brief Major version of libkactivities, at compile time
#define KACTIVITIES_VERSION_MAJOR 6
/// @brief Minor version of libkactivities, at compile time
#define KACTIVITIES_VERSION_MINOR 2
/// @brief Release version of libkactivities, at compile time
#define KACTIVITIES_VERSION_RELEASE 0

#define KACTIVITIES_MAKE_VERSION(a,b,c) (((a) << 16) | ((b) << 8) | (c))

/**
 * Compile time macro for the version number of libkactivities
 */
#define KACTIVITIES_VERSION \
    KACTIVITIES_MAKE_VERSION(KACTIVITIES_VERSION_MAJOR, KACTIVITIES_VERSION_MINOR, KACTIVITIES_VERSION_RELEASE)

/**
 * Compile-time macro for checking the kactivities version. Not useful for
 * detecting the version of libkactivities at runtime.
 */
#define KACTIVITIES_IS_VERSION(a,b,c) (KACTIVITIES_VERSION >= KACTIVITIES_MAKE_VERSION(a,b,c))

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
