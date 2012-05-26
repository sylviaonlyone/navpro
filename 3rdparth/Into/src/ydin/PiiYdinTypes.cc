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

#include "PiiYdinTypes.h"
#include <PiiMatrixSerialization.h>
#include <PiiSerializableExport.h>
#include <PiiSerializationTraits.h>
#include <PiiSerialization.h>
#include <PiiColor.h>
#include <complex>
#include <QDate>
#include <QTime>
#include <QStringList>

// matrices
PII_REGISTER_VARIANT_TYPE(PiiMatrix<char>);
PII_REGISTER_VARIANT_TYPE(PiiMatrix<short>);
PII_REGISTER_VARIANT_TYPE(PiiMatrix<int>);
PII_REGISTER_VARIANT_TYPE(PiiMatrix<qint64>);
//PII_REGISTER_VARIANT_TYPE(PiiMatrix<long long>);
PII_REGISTER_VARIANT_TYPE(PiiMatrix<unsigned char>);
PII_REGISTER_VARIANT_TYPE(PiiMatrix<unsigned short>);
PII_REGISTER_VARIANT_TYPE(PiiMatrix<unsigned int>);
PII_REGISTER_VARIANT_TYPE(PiiMatrix<quint64>);
//PII_REGISTER_VARIANT_TYPE(PiiMatrix<unsigned long long>);
PII_REGISTER_VARIANT_TYPE(PiiMatrix<float>);
PII_REGISTER_VARIANT_TYPE(PiiMatrix<double>);
//PII_REGISTER_VARIANT_TYPE(PiiMatrix<long double>);
PII_REGISTER_VARIANT_TYPE(PiiMatrix<bool>);

// color images
PII_REGISTER_VARIANT_TYPE(PiiMatrix<PiiColor<unsigned char> >);
PII_REGISTER_VARIANT_TYPE(PiiMatrix<PiiColor4<unsigned char> >);
PII_REGISTER_VARIANT_TYPE(PiiMatrix<PiiColor<unsigned short> >);
PII_REGISTER_VARIANT_TYPE(PiiMatrix<PiiColor<float> >);

// complex matrices
PII_REGISTER_VARIANT_TYPE(PiiMatrix<std::complex<int> >);
PII_REGISTER_VARIANT_TYPE(PiiMatrix<std::complex<float> >);
PII_REGISTER_VARIANT_TYPE(PiiMatrix<std::complex<double> >);
//PII_REGISTER_VARIANT_TYPE(PiiMatrix<std::complex<long double> >);

// colors
PII_REGISTER_VARIANT_TYPE(PiiColor<unsigned char>);
PII_REGISTER_VARIANT_TYPE(PiiColor4<unsigned char>);
PII_REGISTER_VARIANT_TYPE(PiiColor<unsigned short>);
PII_REGISTER_VARIANT_TYPE(PiiColor<float>);

// complex numbers
PII_REGISTER_VARIANT_TYPE(std::complex<int>);
PII_REGISTER_VARIANT_TYPE(std::complex<float>);
PII_REGISTER_VARIANT_TYPE(std::complex<double>);
//PII_REGISTER_VARIANT_TYPE(std::complex<long double>);

// Qt classes
PII_REGISTER_VARIANT_TYPE(QString);
PII_REGISTER_VARIANT_TYPE(QImage);

PII_REGISTER_VARIANT_TYPE(PiiSocketState);


namespace PiiYdin
{
  QString convertToQString(const PiiVariant& variant)
  {
    QString strValue;
    switch (variant.type())
      {
      case QStringType:
        return variant.valueAs<QString>();
        PII_PRIMITIVE_CASES(return numberToQString, variant);
      }
    return QString();
  }  

  QString convertToQString(PiiInputSocket* input)
  {
    QString strValue(convertToQString(input->firstObject()));
    if (strValue.isNull())
      PII_THROW_UNKNOWN_TYPE(input);
    return strValue;
  }  
}
