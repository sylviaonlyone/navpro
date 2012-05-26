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

#include "PiiScript.h"

#include <QScriptValueIterator>

namespace PiiScript
{
  const char
    *pInstanceOfXRequired = QT_TR_NOOP("%1() can be applied to instances of %2 only."),
    *pTakesNoArguments = QT_TR_NOOP("%1() takes no arguments."),
    *pTakesOneArgument = QT_TR_NOOP("%1() takes exactly one argument."),
    *pTakesAtLeastOneArgument = QT_TR_NOOP("%1() takes at least one argument."),
    *pTakesNArguments = QT_TR_NOOP("%1() takes %2 arguments."),
    *pArgumentNMustBeX = QT_TR_NOOP("%1(): argument %2 must be an instance of %3.");

  const QScriptEngine::QObjectWrapOptions defaultWrapOptions =
    QScriptEngine::ExcludeDeleteLater |
    QScriptEngine::AutoCreateDynamicProperties;

  void setSuperclass(QScriptValue prototype, const char* superclass)
  {
    prototype.setPrototype(prototype.engine()->globalObject().property(superclass).property("prototype"));
  }

  void configure(QScriptValue target, QScriptValue source)
  {
    QScriptValueIterator it(source);
    while (it.hasNext())
      {
        it.next();
        target.setProperty(it.name(), it.value());
      }
  }

  QScriptValue configure(QScriptValue object, const QVariantMap& map)
  {
    for (QVariantMap::const_iterator i = map.begin(); i != map.end(); ++i)
      object.setProperty(i.key(), object.engine()->newVariant(i.value()));
    return object;
  }

  QVariantMap objectToMap(QScriptValue object)
  {
    QVariantMap mapResult;
    QScriptValueIterator it(object);
    while (it.hasNext())
      {
        it.next();
        mapResult[it.name()] = it.value().toVariant();
      }
    return mapResult;
  }
  
  QScriptValue mapToObject(QScriptEngine* engine, const QVariantMap& map)
  {
    QScriptValue object(engine->newObject());
    return configure(object, map);
  }
}
