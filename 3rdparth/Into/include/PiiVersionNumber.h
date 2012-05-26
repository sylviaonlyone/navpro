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

#ifndef _PIIVERSIONNUMBER_H
#define _PIIVERSIONNUMBER_H

#include "PiiGlobal.h"
#include <QVector>
#include <QString>
#include <QStringList>

/**
 * A class that can be used to compare version numbers composed of
 * multiple dot-separated parts. A version number string may
 * optionally contain a dash-separated revision name, which the class
 * recognizes.
 *
 * Typically, version numbers are composed of three numeric parts:
 * major, minor and patch, but the class does not prevent you from
 * using any number of parts.
 *
 * @code
 * PiiVersionNumber v1("1.0.0");
 * PiiVersionNumber v2("1.1-custom");
 * PiiVersionNumber v3(0, 9);
 * PiiVersionNumber v4(0, 9, 0, "custom");
 * PiiVersionNumber v5(0, 9, 0);
 * PiiVersionNumber v6("1.1-custom-2");
 * PiiVersionNumber v7("2.0.0-gamma");
 * PiiVersionNumber v8("2.0.0-delta");
 * QVERIFY(v1 < v2);
 * QVERIFY(v1 >= v3);
 * QVERIFY(v4 != v3);
 * QVERIFY(v4 < v3);
 * QVERIFY(v5 > v4);
 * QVERIFY(v6 > v2);
 * QVERIFY(v7 < v8);
 * QCOMPARE(v2.revision(), QString("custom"));
 * QCOMPARE(v6.revision(), QString("custom-2"));
 * @endcode
 *
 * @ingroup Core
 */
class PII_CORE_EXPORT PiiVersionNumber
{
public:
  /**
   * Create a new version number with the given @p major, @p minor, and
   * @p patch numbers and a @p revision name.
   */
  PiiVersionNumber(int major=0, int minor=0, int patch=-1, const QString& revision = "");
  /**
   * Initialize a version number from a string. Valid examples include
   * "1.0", "2.23.5.5", "0.9-beta", and even "-alpha".
   */
  PiiVersionNumber(const QString& versionString);
  /**
   * Copy a version number.
   */
  PiiVersionNumber(const PiiVersionNumber& other);

  ~PiiVersionNumber();
  
  /**
   * Parse a version number string and set the version number fields
   * accordingly.
   *
   * @return @p true iff the number was successfully parsed
   */
  bool setVersionString(const QString& versionString);

  /**
   * Get the @p indexth part of the version number. Index 0 is for the
   * major version, 1 for the minor version etc. If there is no such
   * part, 0 will be returned.
   */
  int part(int index) const;

  /**
   * Get the number of parts in the version.
   */
  int partCount() const;

  /**
   * See if the version contains a revision name.
   */
  bool hasRevision() const;

  /**
   * Get the revision name. If there is no revision name, an empty
   * string will be returned.
   */
  QString revision() const;

  /**
   * Create a string representation of the version number. Version
   * numbers will be separated with dots, and the revision name with a
   * dash.
   */
  QString toString() const;

  /**
   * Assign another version number to this.
   */
  PiiVersionNumber& operator= (const PiiVersionNumber& other);

  /**
   * Compare version numbers. The first differing decimal place
   * determines the ordering. If all decimal places are the same, the
   * revision will be checked. If the revision contains dashes (-),
   * each dash-separated part will be checked separately. The ordering
   * is determined by comparing the parts as integers, if they are
   * composed of digits only. Otherwise, their textual sort order
   * (case-insensitivive) determines the ordering. Names of Greek
   * letters are recognized as special cases. Alpha, beta, gamma,
   * delta ect. are sorted according to the greek alphabet. If only
   * one of the version numbers has a revision suffix, the type of the
   * suffix determines the ordering. A numeric suffix means a bigger
   * version whereas a textual suffix means a smaller version.
   * Therefore, 2.0.0-beta < 2.0.0 < 2.0.0-2.
   */
  bool operator< (const PiiVersionNumber& other) const { return compare(other) < 0; }
  /**
   * Compare version numbers. See operator<().
   */
  bool operator> (const PiiVersionNumber& other) const { return compare(other) > 0; }
  /**
   * Compare version numbers. See operator<().
   */
  bool operator<= (const PiiVersionNumber& other) const { return compare(other) <= 0; }
  /**
   * Compare version numbers. See operator<().
   */
  bool operator>= (const PiiVersionNumber& other) const { return compare(other) >= 0; }
  /**
   * Compare version numbers. Version numbers are equal if all of
   * their elements, including the revision name, are equal. "1.0" is
   * different from "1.0.0".
   */
  bool operator== (const PiiVersionNumber& other) const;
  /**
   * Compare version numbers. Version numbers are different if any of
   * their elements, including the revision name, are different.
   */
  bool operator!= (const PiiVersionNumber& other) const;

private:
  int compare(const PiiVersionNumber& other) const;

  class Data
  {
  public:
    Data();
    Data(const Data& other);
    
    QVector<int> vecParts;
    QString strRevision;
    QStringList lstRevisionParts;
  } *d;

  static const QStringList _lstGreekLetters;
};

#endif //_PIIVERSIONNUMBER_H
