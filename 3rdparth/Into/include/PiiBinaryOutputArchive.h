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

#ifndef _PIIBINARYOUTPUTARCHIVE_H
#define _PIIBINARYOUTPUTARCHIVE_H

#include <QDataStream>
#include <cstring>
#include "PiiArchive.h"
#include "PiiOutputArchive.h"
#include "PiiArchiveMacros.h"
#include "PiiBinaryArchive.h"

/**
 * Binary output archive stores data in a raw binary format. The
 * binary format is platform-dependent.
 *
 * @ingroup Serialization
 */
class PII_SERIALIZATION_EXPORT PiiBinaryOutputArchive :
  public PiiOutputArchive<PiiBinaryOutputArchive>,
  public PiiArchive,
  private QDataStream
{
public:
  /**
   * Construct a new binary output archive that writes data to the
   * given I/O device. The device must be open.
   *
   * @exception PiiSerializationException& if the stream is not open
   * or cannot be written to.
   */
  PiiBinaryOutputArchive(QIODevice* d);
  
  void writeRawData(const void* ptr, unsigned int size);

  PiiBinaryOutputArchive& operator<< (const QString& value);
  
  PiiBinaryOutputArchive& operator<< (QString& value)
  {
    return operator<<(const_cast<const QString&>(value));
  }

  PiiBinaryOutputArchive& operator<< (const char* value);
  
  PII_PRIMITIVE_OUTPUT_OPERATORS(PiiBinaryOutputArchive, QDataStream)
  PII_PRIMITIVE_OPERATOR(PiiBinaryOutputArchive, char, QDataStream, <<, )
  PII_PRIMITIVE_OPERATOR(PiiBinaryOutputArchive, unsigned char, QDataStream, <<, )
  PII_DEFAULT_OUTPUT_OPERATORS(PiiBinaryOutputArchive)

protected:
  void startDelim() {}
  void endDelim() {}
};

PII_DECLARE_SERIALIZER(PiiBinaryOutputArchive);
PII_DECLARE_FACTORY_MAP(PiiBinaryOutputArchive);

#endif //_PIIBINARYOUTPUTARCHIVE_H
