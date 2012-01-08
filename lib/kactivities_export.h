/****************************************************************************************
 * Copyright (c) 2012 Patrick von Reth <patrick.vonreth@gmail.com>                      *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#ifndef KACTIVITIES_EXPORT_H
#define KACTIVITIES_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef KACTIVITIES_EXPORT
# ifdef MAKE_KACTIVITIES_LIB 
   /* We are building this library */ 
#  define KACTIVITIES_EXPORT KDE_EXPORT

#  if defined(DEBUG)
#    define KACTIVITIES_EXPORT_TESTS KDE_EXPORT
#  else
#    define KACTIVITIES_EXPORT_TESTS
#  endif

# else
   /* We are using this library */ 
#  define KACTIVITIES_EXPORT KDE_IMPORT

#  if defined(DEBUG)
#    define KACTIVITIES_EXPORT_TESTS KDE_IMPORT
#  else
#    define KACTIVITIES_EXPORT_TESTS
#  endif

# endif//MAKE_KACTIVITIES_LIB 
#endif// KACTIVITIES_EXPORT

# ifndef KACTIVITIES_EXPORT_DEPRECATED
#  define KACTIVITIES_EXPORT_DEPRECATED KDE_DEPRECATED KACTIVITIES_EXPORT
# endif

#endif

