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

#ifndef _PIIBINARYINPUTARCHIVE_H
#define _PIIBINARYINPUTARCHIVE_H

#include <QDataStream>
#include <cstring>
#include "PiiArchive.h"
#include "PiiInputArchive.h"
#include "PiiArchiveMacros.h"
#include "PiiBinaryArchive.h"

/**
 * %PiiBinaryInputArchive reads raw binary data. The binary format is
 * platform-dependent.
 *
 * @ingroup Serialization
 */
class PII_SERIALIZATION_EXPORT PiiBinaryInputArchive :
  public PiiInputArchive<PiiBinaryInputArchive>,
  public PiiArchive,
  private QDataStream
{
public:
  /**
   * Construct a new binary input archive that reads the given I/O
   * device. The device must be open.
   *
   * @exception PiiSerializationException& if the device is not open,
   * or it cannot be read from, or the archive format is unknown
   */
  PiiBinaryInputArchive(QIODevice* d);
  
  void readRawData(void* ptr, unsigned int size);
  
  PiiBinaryInputArchive& operator>> (QString& value);

  PiiBinaryInputArchive& operator>> (char*& value);

  PiiBinaryInputArchive& operator>> (char& value) { return operator>> ((signed char&)value); }
  PiiBinaryInputArchive& operator>> (unsigned char& value) { QDataStream::operator>>(value); return *this; }

  PII_PRIMITIVE_INPUT_OPERATORS(PiiBinaryInputArchive, QDataStream)
  PII_PRIMITIVE_OPERATOR(PiiBinaryInputArchive, signed char, QDataStream, >>, &)
  PII_DEFAULT_INPUT_OPERATORS(PiiBinaryInputArchive)

protected:
  void startDelim() {}
  void endDelim() {}
};

PII_DECLARE_SERIALIZER(PiiBinaryInputArchive);
PII_DECLARE_FACTORY_MAP(PiiBinaryInputArchive);

#endif //_PIIBINARYINPUTARCHIVE_H
