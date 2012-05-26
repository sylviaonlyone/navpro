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

#ifndef _PIITEXTOUTPUTARCHIVE_H
#define _PIITEXTOUTPUTARCHIVE_H

#include <QTextStream>
#include <cstring>
#include "PiiArchive.h"
#include "PiiOutputArchive.h"  
#include "PiiArchiveMacros.h"
#include "PiiTextArchive.h"

/**
 * Text output archive stores data in a space-separated textual
 * format. The archive uses UTF-8 to encode non-ASCII characters.
 *
 * @ingroup Serialization
 */
class PII_SERIALIZATION_EXPORT PiiTextOutputArchive :
  public PiiOutputArchive<PiiTextOutputArchive>,
  public PiiArchive,
  private QTextStream
{
public:
  /**
   * Construct a new text output archive that writes data to the given
   * I/O device. The device must be open.
   *
   * @exception PiiSerializationException& if the stream is not open
   * or cannot be written to.
   */
  PiiTextOutputArchive(QIODevice* d);
  
  /**
   * Writes raw binary data to the text archive. The data is base64
   * encoded before writing.
   */
  void writeRawData(const void* ptr, unsigned int size);

  PiiTextOutputArchive& operator<< (const QString& value);

  PiiTextOutputArchive& operator<< (QString& value)
  {
    return operator<< (const_cast<const QString&>(value));
  }
  
  PiiTextOutputArchive& operator<< (const char* value);

  PiiTextOutputArchive& operator<< (char value) { return operator<<((short)value); }
  PiiTextOutputArchive& operator<< (unsigned char value) { return operator<<((unsigned short)value); }
  PII_PRIMITIVE_OUTPUT_OPERATORS(PiiTextOutputArchive, QTextStream)
  PII_DEFAULT_OUTPUT_OPERATORS(PiiTextOutputArchive)

protected:
  // Text archive separates each value by a single space.
  void startDelim() { QTextStream::operator<< (' '); }
  void endDelim() {}
};

PII_DECLARE_SERIALIZER(PiiTextOutputArchive);
PII_DECLARE_FACTORY_MAP(PiiTextOutputArchive);

#endif //_PIITEXTOUTPUTARCHIVE_H
