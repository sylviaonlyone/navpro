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

#ifndef _PIIGENERICINPUTARCHIVE_H
#define _PIIGENERICINPUTARCHIVE_H

#include "PiiInputArchive.h"
#include "PiiArchiveMacros.h"

/**
 * A polymorphic implementation of the archive interface. Through this
 * interface, all input archive interfaces can masquerade as a single
 * archive. This is useful when one cannot know all archive types at
 * compile time.
 *
 * @ingroup Serialization
 */
class PII_SERIALIZATION_EXPORT PiiGenericInputArchive :
  public PiiInputArchive<PiiGenericInputArchive>
{
public:
  template <class Archive> class Impl;

  virtual ~PiiGenericInputArchive() {}
  
  virtual int majorVersion() const = 0;
  virtual int minorVersion() const = 0;
  
  virtual PiiGenericInputArchive& operator>>(unsigned char& value) = 0;
  virtual PiiGenericInputArchive& operator>>(unsigned short& value) = 0;
  virtual PiiGenericInputArchive& operator>>(unsigned int& value) = 0;
  virtual PiiGenericInputArchive& operator>>(unsigned long& value) = 0;
  virtual PiiGenericInputArchive& operator>>(unsigned long long& value) = 0;
  virtual PiiGenericInputArchive& operator>>(char& value) = 0;
  virtual PiiGenericInputArchive& operator>>(short& value) = 0;
  virtual PiiGenericInputArchive& operator>>(int& value) = 0;
  virtual PiiGenericInputArchive& operator>>(long& value) = 0;
  virtual PiiGenericInputArchive& operator>>(long long& value) = 0;
  virtual PiiGenericInputArchive& operator>>(float& value) = 0;
  virtual PiiGenericInputArchive& operator>>(double& value) = 0;
  //virtual PiiGenericInputArchive& operator>>(long double& value) = 0;
  virtual PiiGenericInputArchive& operator>>(bool& value) = 0;
  virtual PiiGenericInputArchive& operator>>(char*& value) = 0;
  virtual PiiGenericInputArchive& operator>>(QString& value) = 0;
  virtual void readRawData(void* ptr, unsigned int size) = 0;

  PII_DEFAULT_INPUT_OPERATORS(PiiGenericInputArchive)
private:
  virtual void startDelim() = 0;
  virtual void endDelim() = 0;
};

/**
 * Default implementation of the generic input archive interface. This
 * class takes the actual archive implementation as a template
 * parameter. For example, an implementation with a binary input
 * archive can be created simply with
 * PiiGenericInputArchive::Impl<PiiBinaryInputArchive>. The library
 * contains convenient typedefs for known archive types (e.g. 
 * PiiGenericBinaryInputArchive).
 *
 * @ingroup Serialization
 */
template <class Archive> class PiiGenericInputArchive::Impl :
  public PiiGenericInputArchive,
  private Archive
{
public:
  Impl(QIODevice* d) : Archive(d) {}

  int majorVersion() const { return Archive::majorVersion(); }
  int minorVersion() const { return Archive::minorVersion(); }
  
#define PII_STREAM_OP(Type) virtual PiiGenericInputArchive& operator>>(Type value) { Archive::operator>> (value); return *this; }
  PII_STREAM_OP(unsigned char&)
  PII_STREAM_OP(unsigned short&)
  PII_STREAM_OP(unsigned int&)
  PII_STREAM_OP(unsigned long&)
  PII_STREAM_OP(unsigned long long&)
  PII_STREAM_OP(char&)
  PII_STREAM_OP(short&)
  PII_STREAM_OP(int&)
  PII_STREAM_OP(long&)
  PII_STREAM_OP(long long&)
  PII_STREAM_OP(float&)
  PII_STREAM_OP(double&)
  //PII_STREAM_OP(long double&)
  PII_STREAM_OP(bool&)
  PII_STREAM_OP(char*&)
  PII_STREAM_OP(QString&)
#undef PII_STREAM_OP
  virtual void readRawData(void* ptr, unsigned int size) { Archive::readRawData(ptr, size); }

  PII_DEFAULT_INPUT_OPERATORS(PiiGenericInputArchive)

private:
  void startDelim() { Archive::startDelim(); }
  void endDelim() { Archive::endDelim(); }
};

#include "PiiSerializer.h"

PII_DECLARE_SERIALIZER(PiiGenericInputArchive);
PII_DECLARE_FACTORY_MAP(PiiGenericInputArchive);

#endif //_PIIGENERICINPUTARCHIVE_H
