/* This file is part of Into.
 * Copyright (C) 2003-2011 Intopii.
 * All rights reserved.
 *
 * IMPORTANT LICENSING INFORMATION
 *
 * Into is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License version 3
 * as published by the Free Software Foundation.
 *
 * Into is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef _PIIGLOBAL_H
#define _PIIGLOBAL_H

/**
 * @file
 *
 * Export/import macros and other global definitions such as d-pointer
 * stuff.
 */

/**
 * @defgroup Core Core module
 *
 * The Core module contains data structures, classes, definitions and
 * utilities for many common tasks that are not dependent on the Ydin
 * execution engine.
 */

// Disable documentation
/// @cond null

#define INTO_VERSION_STR "2.0.0-beta"

#include <QtGlobal>
#include <QAtomicInt>

#ifdef _MSC_VER
/*
 * Expands to __declspec(dllexport) on Windows (MSVC) and to nothing
 * on Linux.
 */
#  define PII_DECL_EXPORT __declspec(dllexport)
#else
#  define PII_DECL_EXPORT
#endif

#ifdef _MSC_VER
/*
 * Expands to __declspec(dllimport) on Windows (MSVC) and to nothing
 * on Linux.
 */
#  define PII_DECL_IMPORT __declspec(dllimport)
#else
#  define PII_DECL_IMPORT
#endif

#ifdef PII_BUILD_CORE
#  define PII_CORE_EXPORT PII_DECL_EXPORT
#  define PII_BUILDING_CORE 1
#else
#  define PII_CORE_EXPORT PII_DECL_IMPORT
#  define PII_BUILDING_CORE 0
#endif

#if defined(Q_CC_GNU) && !defined(__INSURE__)
#  define PII_PRINTF_ATTR(first,rest)  __attribute__ ((format (printf, first, rest)))
#else
#  define PII_PRINTF_ATTR(first,rest)
#endif

#define PII_D_FUNC \
  inline Data* _d() { return static_cast<Data*>(this->d); } \
  inline const Data* _d() const { return static_cast<const Data*>(this->d); } \
  friend class Data

// reinterpret_cast must be used instead of (the safer) static_cast if
// the Data class is defined outside of the containing class. In this
// case the compiler does not know if static_cast can be performed.
#define PII_UNSAFE_D_FUNC \
  inline Data* _d() { return reinterpret_cast<Data*>(this->d); } \
  inline const Data* _d() const { return reinterpret_cast<const Data*>(this->d); } \
  friend class Data

#define PII_Q_FUNC(CLASS) \
  inline CLASS* owner() { return static_cast<CLASS*>(this->q); } \
  inline const CLASS* owner() const { return static_cast<const CLASS *>(this->q); } \
  friend class CLASS

#define PII_UNSAFE_Q_FUNC(CLASS) \
  inline CLASS* owner() { return reinterpret_cast<CLASS*>(this->q); } \
  inline const CLASS* owner() const { return reinterpret_cast<const CLASS *>(this->q); } \
  friend class CLASS

#define PII_D Data* const d = _d()

#define PII_Q(CLASS) CLASS* const q = owner()

#define PII_DISABLE_COPY(CLASS) private: CLASS(const CLASS&); CLASS& operator= (const CLASS& other)

#ifdef PII_CXX0X
#  define PII_MOVE std::move
#else
#  define PII_MOVE
#endif

#define PII_TYPEMAP(NAME) namespace NAME
#define PII_MAP_PUT_DEFAULT(TYPE) template <class _T_> struct Mapper { typedef TYPE Type; }
#define PII_MAP_PUT_NO_DEFAULT template <class _T_> struct Mapper
#define PII_MAP_PUT_SELF_DEFAULT PII_MAP_PUT_DEFAULT(_T_)
#define PII_MAP_PUT_TEMPLATE(FROM, TO) template <class _T_> struct Mapper<FROM<_T_> > { typedef TO Type; }
#define PII_MAP_PUT(FROM, TO) template <> struct Mapper<FROM> { typedef TO Type; }
#define PII_MAP_PUT_SELF(TYPE) PII_MAP_PUT(TYPE, TYPE)

#define PII_MAP_TYPE(MAP, TYPE) MAP::Mapper<TYPE>::Type

template <class Derived> struct PiiSharedD
{
  PiiSharedD() : iRefCount(1) {}

  Derived* self() { return static_cast<Derived*>(this); }
  const Derived* self() const { return static_cast<const Derived*>(this); }

  void assignTo(Derived*& d) { reserve(); d->release(); d = self(); }
  void reserve() { iRefCount.ref(); }
  void release() { if (!iRefCount.deref()) delete self(); }

  Derived* detach()
  {
    if (iRefCount != 1)
      {
        Derived* pData = new Derived(*self());
        release();
        return pData;
      }
    return self();
  }
    
  QAtomicInt iRefCount;
};

#include "PiiValueSet.h"
#include "PiiLog.h"
#include "PiiSynchronized.h"
/// @endcond

#endif //_PIIGLOBAL_H
