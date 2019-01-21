/*
   Copyright (c) 2009, 2012, Oracle and/or its affiliates. All rights reserved.

   The MySQL Connector/C++ is licensed under the terms of the GPLv2
   <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
   MySQL Connectors. There are special exceptions to the terms and
   conditions of the GPL as it is applied to this software, see the
   FLOSS License Exception
   <http://www.mysql.com/about/legal/licensing/foss-exception.html>.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

// libmysql defines HAVE_STRTOUL (on win), so we have to follow different pattern in definitions names
// to avoid annoying warnings.

#define HAVE_FUNCTION_STRTOLD 1
#define HAVE_FUNCTION_STRTOLL 1
#define HAVE_FUNCTION_STRTOL 1
#define HAVE_FUNCTION_STRTOULL 1

#define HAVE_FUNCTION_STRTOUL 1

#define HAVE_FUNCTION_STRTOIMAX 1
#define HAVE_FUNCTION_STRTOUMAX 1

#define HAVE_STDINT_H 1
#define HAVE_INTTYPES_H 1

#define HAVE_INT8_T   1
#define HAVE_UINT8_T  1
#define HAVE_INT16_T  1
#define HAVE_UINT16_T 1
#define HAVE_INT32_T  1
#define HAVE_UINT32_T 1
#define HAVE_INT32_T  1
#define HAVE_UINT32_T 1
#define HAVE_INT64_T  1
#define HAVE_UINT64_T 1
#define HAVE_MS_INT8     1
#define HAVE_MS_UINT8    1
#define HAVE_MS_INT16    1
#define HAVE_MS_UINT16   1
#define HAVE_MS_INT32    1
#define HAVE_MS_UINT32   1
#define HAVE_MS_INT64   1
#define HAVE_MS_UINT64   1


#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif


#if defined(HAVE_INTTYPES_H) && !defined(_WIN32)
#include <inttypes.h>
#endif

#if defined(_WIN32)
#ifndef CPPCONN_DONT_TYPEDEF_MS_TYPES_TO_C99_TYPES

#if _MSC_VER >= 1600

#include <stdint.h>

#else

#if !defined(HAVE_INT8_T) && defined(HAVE_MS_INT8)
typedef __int8			int8_t;
#endif

#ifdef HAVE_MS_UINT8
typedef unsigned __int8	uint8_t;
#endif
#ifdef HAVE_MS_INT16
typedef __int16			int16_t;
#endif

#ifdef HAVE_MS_UINT16
typedef unsigned __int16	uint16_t;
#endif

#ifdef HAVE_MS_INT32
typedef __int32			int32_t;
#endif

#ifdef HAVE_MS_UINT32
typedef unsigned __int32	uint32_t;
#endif

#ifdef HAVE_MS_INT64
typedef __int64			int64_t;
#endif
#ifdef HAVE_MS_UINT64
typedef unsigned __int64	uint64_t;
#endif

#endif  // _MSC_VER >= 1600
#endif	// CPPCONN_DONT_TYPEDEF_MS_TYPES_TO_C99_TYPES
#endif	//	_WIN32
