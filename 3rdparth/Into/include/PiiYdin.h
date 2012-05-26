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

#ifndef _PIIYDIN_H
#define _PIIYDIN_H

#include <PiiGlobal.h>
#include <PiiResourceDatabase.h>

#ifdef PII_BUILD_YDIN
#  define PII_YDIN_EXPORT PII_DECL_EXPORT
#  define PII_BUILDING_YDIN 1
#else
#  define PII_YDIN_EXPORT PII_DECL_IMPORT
#  define PII_BUILDING_YDIN 0
#endif

/**
 * @defgroup Ydin Ydin execution engine
 *
 * The Ydin module contains classes that are needed for building @ref
 * PiiOperation "operations" and connecting them into an @ref
 * PiiEngine "execution engine" to form a functional application.
 */

namespace PiiYdin
{ 
  /**
   * Get a pointer to an application-wide @ref PiiResourceDatabase
   * "resource database". This database is used to record
   * relationships between components in Into. The structure of the
   * resource database makes it possible to also store
   * application-specific information about plug-ins, operations, and
   * other classes. The following predicates have a special meaning:
   *
   * @lip pii:parent - specifies a parent-child relationship between
   * two resources. This parent-child relationship has nothing to do
   * with class hierarchies or QObject's object tree. It exists just
   * as a general way of grouping things together. A resource may have
   * many parents. By convention, all plug-ins should specify a
   * parent-child relationship between the plug-in resource and the
   * registered operations. That is, the resource database should
   * contain a statement with "pii:parent" as the predicate for each
   * registered operation like so:
   * <tt>(MyOperation,pii:parent,MyPlugin)</tt>.
   *
   * @lip pii:class - specify a superclass for an instantiable
   * resource. A resource may point to any number of superclass
   * resources, which may also have superclasses. The class attribute
   * is used by PiiYdin::createResource() to (recursively) check that
   * the type of the class being instantiated matches the type
   * requested. All operations should specify at least PiiOperation as
   * their superclass like so:
   * <tt>(MyOperation,pii:class,PiiOperation)</tt>.
   *
   * @lip pii:offset - used in reified statements to specify the byte
   * offset between the start address of the class instance and that
   * of the specified superclass. This information is used to perform
   * cross casts between unrelated superclasses in multiple
   * inheritance cases. The offset of the first superclass in
   * inheritance order always has an offset of zero, and can be
   * omitted. Let us assume @p MyOperation implements @p MyInterface. 
   * This would be stored into the resource database like so:
   * <tt>(MyOperation,pii:class,MyInterface)</tt>. Consequently, the
   * resource can be instantiated as a PiiOperation. But a pointer to
   * a PiiOperation cannot be directly used as a pointer to a @p
   * MyInterface; an offset must be provided. If the previous
   * statement got 314 as the id, the offset would be stated as
   * follows: <tt>(#314,pii:offset,16)</tt>, where 16 stands for the
   * number of bytes a pointer to a PiiOperation (or a QObject) must
   * be offset to get a pointer to @p MyInterface. See @ref
   * PII_REGISTER_CLASS and @ref PII_REGISTER_SUPERCLASS.
   *
   * @lip pii:display - marks the object of a statement as a "display"
   * GUI for the subject.  For example, the statement
   * <tt>(PiiVisualTrainer,pii:display,PiiVisualTrainerWidget)</tt>
   * specifies that PiiVisualTrainerWidget can work as a display GUI
   * for PiiVisualTrainer.
   *
   * @lip pii:connector - used in reified statements to specify a
   * PiiResourceConnector object that binds two resource instances
   * together. Into uses connector objects to bind together things
   * like operations and their user interfaces, classifiers and
   * distance measures etc. A connector object is attached to a
   * statement about a relationsip between resources. Let us assume
   * that the statement in the previous example got a resource id of
   * #123. The connector that binds the operation and its GUI together
   * would then be specified as
   * <tt>(#123,pii:connector,PiiVisualTrainerConnector)</tt>. The
   * object (PiiVisualTrainerConnector) must be a class that
   * implements the PiiResourceConnector
   * interface. PiiYdin::connectResources() uses this information to
   * bind resource instances together. See @ref
   * PII_REGISTER_CONNECTION.
   *
   * The resource database is the main source of information related
   * to plug-ins, operations, and their interconnections. A couple of
   * useful searches are shown in the example below:
   *
   * @code
   * PiiResourceDatabase* db = PiiYdin::resourceDatabase();
   * using namespace Pii;
   * // Find all operations
   * QList<QString> lstOperations;
   * lstOperations = db->select(subject,
   *                            attribute("pii:class") == "PiiOperation");
   *
   * // Find all operations in the image plug-in
   * lstOperations = db->select(subject,
   *                            attribute("pii:class") == "PiiOperation") &&
   *                 db->select(subject,
   *                            attribute("pii:parent") == "PiiImagePlugin");
   * @endcode
   *
   * @note To save some bytes of memory, use Pii::classPredicate,
   * Pii::parentPredicate etc. instead of repeating the string
   * literals "pii:class", "pii:parent" etc.
   */
  PII_YDIN_EXPORT PiiResourceDatabase* resourceDatabase();

  /**
   * Returns @p true if @a propertyName equals "name", and @p false
   * otherwise. This utility function is used in overridden
   * implementations of PiiOperation::socketProperty().
   */
  PII_YDIN_EXPORT bool isNameProperty(const char* propertyName);

  /**
   * A pointer to the string literal "pii:class". Use this instead of
   * repeating the string literal to save memory.
   */
  extern PII_YDIN_EXPORT const char* classPredicate;
  /**
   * A pointer to the string literal "pii:parent". Use this instead of
   * repeating the string literal to save memory.
   */
  extern PII_YDIN_EXPORT const char* parentPredicate;
  /**
   * A pointer to the string literal "pii:connector". Use this instead
   * of repeating the string literal to save memory.
   */
  extern PII_YDIN_EXPORT const char* connectorPredicate;
  /**
   * A pointer to the string literal "pii:offset". Use this instead of
   * repeating the string literal to save memory.
   */
  extern PII_YDIN_EXPORT const char* offsetPredicate;
}

#endif //_PIIYDIN_H
