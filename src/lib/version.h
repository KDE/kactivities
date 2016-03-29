/*
 *   Copyright (C) 2008 - 2016 by Aaron Seigo <aseigo@kde.org>
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

#ifndef KACTIVITIES_VERSION_BIN_H
#define KACTIVITIES_VERSION_BIN_H

/** @file version.h <KActivities/Version> */

#include "kactivities_export.h"
#include <kactivities_version.h>

#define KACTIVITIES_VERSION_RELEASE KACTIVITIES_VERSION_PATCH

/**
 * Namespace for everything in libkactivities
 */
namespace KActivities {

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
