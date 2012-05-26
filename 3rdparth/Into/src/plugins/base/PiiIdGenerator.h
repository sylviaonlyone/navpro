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

#ifndef _PIIIDGENERATOR_H
#define _PIIIDGENERATOR_H

#include <PiiDefaultOperation.h>
#include <QString>

/**
 * On operation for generating running IDs. The operation can be used
 * for example for generating file names. The generated id has a format @p
 * [PREFIX]INDEX[SUFFIX], where [PREFIX] is a string set by the
 * property #prefix, INDEX is a running index converted to string
 * generated internally in the operation and [SUFFIX] is a string set
 * by the property #suffix. @p INDEX has at least the width set by the
 * property @p #indexWidth. The extra zeroes are added to fullfill this
 * requirement. The default value for the width is 6. For example if
 * the value of the property #prefix were "image",  value of the
 * property #suffix ".jpg" and the current value of the internal index
 * is 11, the generated id will be "image000011.jpg"
 * 
 *
 * @inputs
 *
 * @in trigger - any object. Informs that a new id must be generated.
 *
 * @outputs
 *
 * @out id - generated id as a string having a format [PREFIX]INDEX[SUFFIX] 
 *
 * @ingroup PiiBasePlugin
 */

class PiiIdGenerator : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Prefix for the id.
   */
  Q_PROPERTY(QString prefix READ prefix WRITE setPrefix);

  /**
   * Suffix for the id.
   */
  Q_PROPERTY(QString suffix READ suffix WRITE setSuffix);

  /**
   * Defines the next index for the id. The default value is zero. The
   * value is not zeroed, when the operation is interrupted, and re-executed.
   * executing the operation after it has been interrupted, doesn't
   * zero it.
   */
  Q_PROPERTY(int nextIndex READ nextIndex WRITE setNextIndex);

  /**
   * Defines the minimum width for the INDEX section of the id. Extra
   * zeroes  are added before other numbers in order to fullfill the
   * minimum width requirement. If for example the value of @p
   * indexWidth were 4, and the value of the current index were 11,
   * the value of the property #prefix "image" and value of the
   * property #suffix ".jpg" the generated id would be
   * "image0011.jpg". The default value is 6. If the value is zero, no
   * extra zeroes are added in the index section. The negative values
   * can cause indeterminate results and should be avoided.
   */
  Q_PROPERTY(int indexWidth READ indexWidth WRITE setIndexWidth);
 
  PII_OPERATION_SERIALIZATION_FUNCTION

public:
  PiiIdGenerator();

  void setPrefix(const QString& prefix);
  QString prefix() const;
  void setSuffix(const QString& suffix);
  QString suffix() const;
  void setNextIndex(int nextIndex);
  int nextIndex() const;
  void setIndexWidth(int indexWidth);
  int indexWidth() const;
  
protected:
  void process();
  void check(bool reset);
  
private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    QString strPrefix;
    QString strSuffix;
    int iNextIndex;
    PiiInputSocket *pTriggerInput;
    PiiOutputSocket *pIdOutput;
    int iIndexWidth;
  };
  PII_D_FUNC;  
};

#endif //_PIIIDGENERATOR_H
