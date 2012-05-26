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

#include "PiiHttpBodyPartHeader.h"
#include <PiiUtil.h>

PiiHttpBodyPartHeader::Data::Data()
{}

PiiHttpBodyPartHeader::Data::Data(const Data& other) :
  mapDispositionParams(other.mapDispositionParams),
  aPreamble(other.aPreamble)
{}

PiiHttpBodyPartHeader::PiiHttpBodyPartHeader() :
  d(new Data)
{}

PiiHttpBodyPartHeader::PiiHttpBodyPartHeader(const QHttpHeader& other) :
  QHttpHeader(other),
  d(new Data)
{
  parseDisposition();
  setDefaultContentType();
}

PiiHttpBodyPartHeader::PiiHttpBodyPartHeader(const PiiHttpBodyPartHeader& other) :
  QHttpHeader(other),
  d(new Data(*other.d))
{
  parseDisposition();
  setDefaultContentType();
}

PiiHttpBodyPartHeader::PiiHttpBodyPartHeader(const QString& str) :
  QHttpHeader(str),
  d(new Data)
{
  parseDisposition();
  setDefaultContentType();
}

PiiHttpBodyPartHeader::~PiiHttpBodyPartHeader()
{
}

void PiiHttpBodyPartHeader::setPreamble(const QByteArray& preamble)
{
  d->aPreamble = preamble;
}
  
QByteArray PiiHttpBodyPartHeader::preamble() const
{
  return d->aPreamble;
}


void PiiHttpBodyPartHeader::setDefaultContentType()
{
  if (!hasContentType())
    setContentType("text/plain");
}

void PiiHttpBodyPartHeader::parseDisposition()
{
  QString strDisposition = value("Content-Disposition");
  int semicolonIndex = strDisposition.indexOf(';');
  d->strContentDisposition = strDisposition.left(semicolonIndex).trimmed();
  if (semicolonIndex != -1)
    {
      d->mapDispositionParams = Pii::decodeProperties(strDisposition.mid(semicolonIndex + 1),
                                                      ';', '=', '\\',
                                                      Pii::TrimPropertyName |
                                                      Pii::TrimPropertyValue |
                                                      Pii::RemoveQuotes |
                                                      Pii::DowncasePropertyName);
    }
}

QString PiiHttpBodyPartHeader::contentDisposition() const
{
  return d->strContentDisposition;
}

bool PiiHttpBodyPartHeader::isMultipart() const
{
  return contentType().startsWith("multipart/");
}

bool PiiHttpBodyPartHeader::isUploadedFile() const
{
  return contentDisposition() == "form-data" &&
    d->mapDispositionParams.contains("filename");
}

QString PiiHttpBodyPartHeader::controlName() const
{
  return d->mapDispositionParams["name"].toString();
}

QString PiiHttpBodyPartHeader::fileName() const
{
  return d->mapDispositionParams["filename"].toString();
}

QString PiiHttpBodyPartHeader::boundary() const
{
  QRegExp reBoundary("boundary=(\".[^\"]+\"|[^\r\n]+)", Qt::CaseInsensitive);
  if (reBoundary.indexIn(contentType()) != -1)
    {
      // The first subexpression
      QString strBoundary = reBoundary.cap(1);
      // Get rid of quotation marks
      if (strBoundary[0] == '"')
        return strBoundary.mid(1, strBoundary.size()-2);
      return strBoundary;
    }
  return QString();
}

PiiHttpBodyPartHeader& PiiHttpBodyPartHeader::operator= (const PiiHttpBodyPartHeader& other)
{
  QHttpHeader::operator= (other);
  d->mapDispositionParams = other.d->mapDispositionParams;
  d->aPreamble = other.d->aPreamble;
  return *this;
}

int PiiHttpBodyPartHeader::majorVersion() const { return 1; }
int PiiHttpBodyPartHeader::minorVersion() const { return 1; }
