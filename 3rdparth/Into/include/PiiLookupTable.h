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

#ifndef _PIILOOKUPTABLE_H
#define _PIILOOKUPTABLE_H

#include <PiiDefaultOperation.h>

/**
 * An operation that maps integers into arbitrary data.
 *
 * @inputs
 *
 * @in index - a zero-based index into the look-up table. If there is
 * no @ref #defaultValue "default value", overflows and underflows
 * will cause a run-time exception. Any primitive type is be accepted.
 * 
 * @outputs
 *
 * @out outputX - any number of outputs that emit arbitrary data. For
 * each incoming @p index, the corresponding look-up table entry will
 * be emitted.
 *
 * @ingroup PiiBasePlugin
 */
class PiiLookupTable : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The look-up table. The look-up table is represented as a list of
   * variants. Each entry may be either a PiiVariant or a
   * QVariantList. In the first case, <tt>table[index]</tt> will be
   * emitted through all outputs whenever @p index is received. If the
   * list elements are QVariantLists, <tt>table[0][index]</tt> will be
   * emitted through @p output0 etc.
   *
   * @code
   * PiiOperation* lut = engine.createOperation("PiiLookupTable");
   * // Map 0 to true and 1 to false. Emit the boolean value through all outputs.
   * lut->setProperty("table", QVariantList() << Pii::createQVariant(true) << Pii::createQVariant(false));
   *
   * QVariantList tbl;
   * // Map 0 to 0 and 1 to -1 in output0
   * tbl.append(QVariantList() << Pii::createQVariant(0) << Pii::createQVariant(-1));
   * // Map 0 to "zero" and 1 to "one" in output1
   * tbl.append(QVariantList() << Pii::createQVariant(QString("zero")) << Pii::createQVariant(QString("one")));
   * lut->setProperty("table", tbl);
   * @endcode
   *
   * If the number of look-up lists is less than the number of outputs
   * (see #outputCount), the last valid value will be used for all
   * extra outputs. If multiple lists are provided, each must be of
   * the same length. Invalid lists will be ignored. If list values
   * are not PiiVariants, they will be substituted with zeros. This
   * will most likely cause a run-time exception, so be careful.
   *
   * By default, the look-up table contains nothing.
   */
  Q_PROPERTY(QVariantList table READ table WRITE setTable);

  /**
   * The number of outputs. Must be greater than zero. The default
   * value is one.
   */
  Q_PROPERTY(int dynamicOutputCount READ dynamicOutputCount WRITE setDynamicOutputCount);

  /**
   * Default value to be used for index over/underflows. If there is
   * no look-up table entry for an input index, this value will be
   * substituted. If the default value is not set and an
   * over/underflow occurs, a run-time exception will be generated.
   */
  Q_PROPERTY(PiiVariant defaultValue READ defaultValue WRITE setDefaultValue);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiLookupTable();

  void check(bool reset);

  void setTable(const QVariantList& table);
  QVariantList table() const;
  void setDynamicOutputCount(int count);
  int dynamicOutputCount() const;
  void setDefaultValue(const PiiVariant& defaultValue);
  PiiVariant defaultValue() const;

protected:
  void process();

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    // A look-up list for all outputs
    QList<QList<PiiVariant> > lstOutputValues;
    // The actual property value
    QVariantList lstTable;
    // Last list index that overflows neither look-up list
    // (lstOutputValues) nor output list.
    int iMaxTableIndex;
    // Last index that doesn't overflow the look-up table itself
    // (lstOutputValues[0])
    int iMaxLookupIndex;
    PiiVariant varDefaultValue;
  };
  PII_D_FUNC;
};

#endif //_PIILOOKUPTABLE_H
