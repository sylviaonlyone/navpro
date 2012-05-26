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

#ifndef _PIIREGEXPMATCHER_H
#define _PIIREGEXPMATCHER_H

#include <PiiDefaultOperation.h>
#include <QRegExp>

/**
 * An operation that splits strings into pieces based on a regular
 * expression.
 *
 * @inputs
 *
 * @in input - input text (QString)
 *
 * @outputs
 *
 * @out outputX - matched texts. X is a zero-based index. The first
 * output, @p output0 will emit the full matched text. The other
 * outputs will emit the matched subexpressions, if any. The number of
 * outputs matches the number of subexpressions in #pattern, plus one.
 *
 * @code
 * PiiOperation* pRe = engine.createOperation("PiiRegExpMatcher");
 * // Separate path from file name. Output1 will emit the path
 * // without a trailing slash, and output2 the file name part.
 * pRe->setProperty("pattern", "(^.*)/([^/]+$)
 * @endcode
 *
 * @ingroup PiiBasePlugin
 */
class PiiRegExpMatcher : public PiiDefaultOperation
{
  Q_OBJECT
  /**
   * The regular expression to be matched against @p input. See
   * QRegExp for syntax of valid patterns. Setting this property
   * changes the number of outputs to match the number of
   * subexpressions (plus one).
   */
  Q_PROPERTY(QString pattern READ pattern WRITE setPattern);
   
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiRegExpMatcher();

  void setPattern(const QString& pattern);
  QString pattern() const;

protected:
  void process();

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    QRegExp re;
  };
  PII_D_FUNC;
};

#endif //_PIIREGEXPMATCHER_H
