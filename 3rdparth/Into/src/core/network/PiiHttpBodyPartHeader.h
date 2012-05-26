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

#ifndef _PIIHTTPBODYPARTHEADER_H
#define _PIIHTTPBODYPARTHEADER_H

#include "PiiNetworkGlobal.h"
#include <QHttpHeader>
#include <QVariantMap>
#include <QByteArray>

/**
 * A class that stores header information for multipart messages as
 * defined in RFC 1341, 2045, and 2387. @p %PiiHttpBodyPartHeader
 * eases parsing of such headers by providing some useful functions
 * for querying values commonly used. Note that the default @p
 * Content-Type for a multipart message is @p text/plain.
 *
 * @ingroup PiiNetwork
 */
class PII_NETWORK_EXPORT PiiHttpBodyPartHeader : public QHttpHeader
{
public:
  /**
   * Create an empty (invalid) header.
   */
  PiiHttpBodyPartHeader();
  /**
   * Copy another header.
   */
  PiiHttpBodyPartHeader(const QHttpHeader& other);
  /**
   * Copy another header.
   */
  PiiHttpBodyPartHeader(const PiiHttpBodyPartHeader& other);
  /**
   * Parse a string containing header information. If the string is
   * not correctly formatted, the header will become invalid.
   */
  PiiHttpBodyPartHeader(const QString& str);

  ~PiiHttpBodyPartHeader();
  
  /**
   * See if the header describes a multipart message. The @p
   * Content-Type header field of a multipart message begins with @p
   * multipart/. The body of a multipart message consists of many
   * entities.
   */
  bool isMultipart() const;

  /**
   * Set the preamble.
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
   * Get the name of an uploaded file, if the entity is a file
   * uploaded from an HTML form. If there is no file name, an empty
   * string will be returned.
   */
  QString fileName() const;

  /**
   * Get the boundary string that separates entities in this multipart
   * message. If this header does not represent a multipart message,
   * an empty string will be returned.
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
  PiiHttpBodyPartHeader& operator= (const PiiHttpBodyPartHeader& other);

  int majorVersion() const;
  int minorVersion() const;

private:
  void parseDisposition();
  void setDefaultContentType();
  
  /// @internal
  class Data
  {
  public:
    Data();
    Data(const Data& other);
    QVariantMap mapDispositionParams;
    QByteArray aPreamble;
    QString strContentDisposition;
  } *d;
};

#endif //_PIIHTTPBODYPARTHEADER_H
