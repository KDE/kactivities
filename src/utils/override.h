/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation,3 Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef UTILS_OVERRIDE_MACRO_H
#define UTILS_OVERRIDE_MACRO_H

#include <config-features.h>

#if HAVE_CXX11_OVERRIDE
    #define _override override
#elif defined(HAVE_CXX_OVERRIDE_ATTR)
    #warning "The override keyword is not supported by the compiler. Trying the override attribute."
    #define _override __attribute__((override))
#else
    #warning "This compiler can not check for overriden methods."
    #define _override // nothing
#endif

#endif // UTILS_OVERRIDE_MACRO_H
