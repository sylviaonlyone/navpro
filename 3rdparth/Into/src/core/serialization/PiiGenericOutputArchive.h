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

#ifndef _PIIGENERICOUTPUTARCHIVE_H
#define _PIIGENERICOUTPUTARCHIVE_H

#include "PiiOutputArchive.h"
#include "PiiArchiveMacros.h"
#include <QIODevice>

/**
 * A polymorphic implementation of the archive interface. Through this
 * interface, all output archive interfaces can masquerade as a single
 * archive. This is useful when one cannot know all archive types at
 * compile time.
 *
 * @ingroup Serialization
 */
class PII_SERIALIZATION_EXPORT PiiGenericOutputArchive :
  public PiiOutputArchive<PiiGenericOutputArchive>
{
public:
  template <class Archive> class Impl;

  virtual ~PiiGenericOutputArchive() {}

  virtual int majorVersion() const = 0;
  virtual int minorVersion() const = 0;
  
  virtual PiiGenericOutputArchive& operator<<(unsigned char value) = 0;
  virtual PiiGenericOutputArchive& operator<<(unsigned short value) = 0;
  virtual PiiGenericOutputArchive& operator<<(unsigned int value) = 0;
  virtual PiiGenericOutputArchive& operator<<(unsigned long value) = 0;
  virtual PiiGenericOutputArchive& operator<<(unsigned long long value) = 0;
  virtual PiiGenericOutputArchive& operator<<(char value) = 0;
  virtual PiiGenericOutputArchive& operator<<(short value) = 0;
  virtual PiiGenericOutputArchive& operator<<(int value) = 0;
  virtual PiiGenericOutputArchive& operator<<(long value) = 0;
  virtual PiiGenericOutputArchive& operator<<(long long value) = 0;
  virtual PiiGenericOutputArchive& operator<<(float value) = 0;
  virtual PiiGenericOutputArchive& operator<<(double value) = 0;
  //virtual PiiGenericOutputArchive& operator<<(long double value) = 0;
  virtual PiiGenericOutputArchive& operator<<(bool value) = 0;
  virtual PiiGenericOutputArchive& operator<<(const char* value) = 0;
  virtual PiiGenericOutputArchive& operator<<(const QString& value) = 0;
  virtual void writeRawData(const void* ptr, unsigned int size) = 0;

  PII_DEFAULT_OUTPUT_OPERATORS(PiiGenericOutputArchive)

private:
  virtual void startDelim() = 0;
  virtual void endDelim() = 0;
};

/**
 * Default implementation of the generic output archive interface. 
 * This class takes the actual archive implementation as a template
 * parameter. For example, an implementation with a binary output
 * archive can be created simply with
 * PiiGenericOutputArchive::Impl<PiiBinaryOutputArchive>. The library
 * contains convenient typedefs for known archive types (e.g. 
 * PiiGenericBinaryOutputArchive).
 *
 * @ingroup Serialization
 */
template <class Archive> class PiiGenericOutputArchive::Impl :
  public PiiGenericOutputArchive,
  private Archive
{
public:
  Impl(QIODevice* d) : Archive(d) {}

  int majorVersion() const { return Archive::majorVersion(); }
  int minorVersion() const { return Archive::minorVersion(); }
  
#define PII_STREAM_OP(Type) PiiGenericOutputArchive& operator<<(Type value) { Archive::operator<< (value); return *this; }
  PII_STREAM_OP(unsigned char)
  PII_STREAM_OP(unsigned short)
  PII_STREAM_OP(unsigned int)
  PII_STREAM_OP(unsigned long)
  PII_STREAM_OP(unsigned long long)
  PII_STREAM_OP(char)
  PII_STREAM_OP(short)
  PII_STREAM_OP(int)
  PII_STREAM_OP(long)
  PII_STREAM_OP(long long)
  PII_STREAM_OP(float)
  PII_STREAM_OP(double)
  //PII_STREAM_OP(long double)
  PII_STREAM_OP(bool)
  PII_STREAM_OP(const char*)
  PII_STREAM_OP(const QString&)
#undef PII_STREAM_OP
  virtual void writeRawData(const void* ptr, unsigned int size) { Archive::writeRawData(ptr, size); }

  PII_DEFAULT_OUTPUT_OPERATORS(PiiGenericOutputArchive)

private:
  void startDelim() { Archive::startDelim(); }
  void endDelim() { Archive::endDelim(); }
};

PII_DECLARE_SERIALIZER(PiiGenericOutputArchive);
PII_DECLARE_FACTORY_MAP(PiiGenericOutputArchive);

#endif //_PIIGENERICOUTPUTARCHIVE_H
