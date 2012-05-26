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

#ifndef _PIIOBJECTFACTORY_H
#define _PIIOBJECTFACTORY_H

#include <QObject>
#include "PiiGlobal.h"

/**
 * An interface for object factories that are able to create objects
 * based on a class name.
 */
class PII_CORE_EXPORT PiiObjectFactory
{
public:
  virtual ~PiiObjectFactory();
  
  /**
   * Create an instance of the named object.
   *
   * @param name the class name of the object (case-sensitive)
   *
   * @return a pointer to a newly created object. The caller is
   * responsible for deleting the object. If the object cannot be
   * created, 0 is returned.
   */
  virtual QObject* create(const QString& name) = 0;

  /**
   * List the class names this factory can create.
   *
   * @return the class names of known objects
   */
  virtual QStringList keys() const = 0;
};

#endif //_PIIOBJECTFACTORY_H
