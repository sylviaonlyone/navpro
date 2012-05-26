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

#ifndef _PIIMIMEHEADER_H
#define _PIIMIMEHEADER_H

#include "PiiNetworkGlobal.h"
#include "PiiMimeException.h"
#include <QHttpHeader>
#include <QVariantMap>
#include <QByteArray>

/**
 * A class that stores header information for multipart messages as
 * defined in RFC 1341, 2045, and 2387. @p %PiiMimeHeader
 * eases parsing of such headers by providing some useful functions
 * for querying values commonly used. Note that the default @p
 * Content-Type for a multipart message is @p text/plain.
 *
 * @ingroup PiiNetwork
 */
class PII_NETWORK_EXPORT PiiMimeHeader : public QHttpHeader
{
public:
  /**
   * Create an empty (invalid) header.
   */
  PiiMimeHeader();
  /**
   * Copy another header.
   */
  PiiMimeHeader(const QHttpHeader& other);
  /**
   * Copy another header.
   */
  PiiMimeHeader(const PiiMimeHeader& other);
  /**
   * Parse a string containing header information. If the string is
   * not correctly formatted, the header will become invalid.
   */
  PiiMimeHeader(const QString& str);

  ~PiiMimeHeader();
  
  /**
   * See if the header describes a multipart message. The @p
   * Content-Type header field of a multipart message begins with @p
   * multipart/. The body of a multipart message consists of many
   * entities.
   */
  bool isMultipart() const;

  /**
   * Sets the preamble.
   */
  void setPreamble(const QByteArray& preamble);
  
  /**
   * Returns the "preamble" of a multipart message, if there is one. 
   * The preamble can be used to transfer additional information that
   * is not part of the entity itself.
   */
  QByteArray preamble() const;

  /**
   * See if the header describes a file uploaded from a HTML form.
   * Browsers use @p multipart/form-data encoding to send files from
   * HTML forms. If the @p Content-Disposition header contains a file
   * name, the entity can be treated as an uploaded file. Use the
   * #fileName() function to get the name of the file.
   */
  bool isUploadedFile() const;

  /**
   * Get the name of the HTML form control that sent the following
   * entity. If the header does not contain such information, an empty
   * string will be returned.
   *
   * @code
   * // Content-Disposition: form-data; name=control
   * QString name = header.controlName(); // returns "control"
   * @endcode
   */
  QString controlName() const;

  /**
   * Returns the name of an uploaded file, if the entity is a file
   * uploaded from an HTML form. If there is no file name, an empty
   * string will be returned.
   */
  QString fileName() const;

  /**
   * Returns the boundary string that separates entities in this
   * multipart message. If this header does not represent a multipart
   * message, an empty string will be returned.
   */
  QString boundary() const;

  /**
   * Get the value of the @p Content-Disposition header, without
   * parameters.
   *
   * @code
   * // Content-Disposition: form-data; name=control
   * QString disposition = header.contentDisposition(); // returns "form-data"
   * @endcode
   */
  QString contentDisposition() const;

  /**
   * Copy another header.
   */
  PiiMimeHeader& operator= (const PiiMimeHeader& other);

  int majorVersion() const;
  int minorVersion() const;

  /**
   * Reads a MIME header from @a device. This functions reads lines
   * from @a device until an empty line is found.
   *
   * @param device the input device
   *
   * @param maxLength the maximum number of bytes to read
   *
   * @param bytesRead a return-value parameter that stores the number
   * of bytes actually read from @a device. Note that this value may
   * be different from the size of the returned array.
   *
   * @return header data as a byte array, excluding the empty line at
   * the end.
   */
  static QByteArray readHeaderData(QIODevice* device, qint64 maxLength, qint64* bytesRead = 0);
  
private:
  void parse();

  static inline QString tr(const char* s) { return QCoreApplication::translate("PiiMimeHeader", s); }
  
  /// @internal
  class Data
  {
  public:
    Data();
    Data(const Data& other);
    
    QVariantMap mapDispositionParams;
    QByteArray aPreamble;
    QString strContentDisposition;
    QString strBoundary;
  } *d;
};

#endif //_PIIMIMEHEADER_H
