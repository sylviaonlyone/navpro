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

#ifndef _PIIRESOURCESTATEMENT_H
#define _PIIRESOURCESTATEMENT_H

#include "PiiGlobal.h"
#include <QString>

/**
 * A class used by PiiResourceDatabase to store statements about
 * resources. Each statement is composed of a subject, a predicate and
 * an object, which are stored as strings. The subject always refers
 * to a resource about which the statement is being made. The
 * predicate describes which trait or aspect of the subject is being
 * described, and object specifies the value of the aspect. The value
 * may be either a string literal or a reference to another resource.
 *
 * @ingroup Core
 */
class PII_CORE_EXPORT PiiResourceStatement
{
public:
  /**
   * Possible types of the object of the statement.
   *
   * @li InvalidType - the statement is invalid.
   *
   * @li LiteralType - the object is a string literal.
   *
   * @li ResourceType - the object is a resource id and references
   * another resource.
   */
  enum Type { InvalidType, LiteralType, ResourceType };

  /**
   * Create an invalid statement.
   */
  PiiResourceStatement();
  
  /**
   * Create a new statement.
   */
  PiiResourceStatement(const char* subject,
                       const char* predicate,
                       const char* object,
                       Type type = LiteralType,
                       int id = -1);

  /**
   * Create a new statement.
   */
  PiiResourceStatement(const QString& subject,
                       const QString& predicate,
                       const QString& object,
                       Type type = LiteralType,
                       int id = -1);

  /**
   * Create a new statement. This constructor creates a statement that
   * refers another statement. The subject will be converted to
   * "#subject". The following two statements are equal:
   *
   * @code
   * PiiResourceStatement s1("#123", "pii:connector", "MyConnector");
   * PiiResourceStatement s2(123, "pii:connector", "MyConnector");
   * @endcode
   */
  PiiResourceStatement(int subject,
                       const QString& predicate,
                       const QString& object,
                       Type type = LiteralType,
                       int id = -1);

  ~PiiResourceStatement();
  
  /**
   * Copy another statement.
   */
  PiiResourceStatement(const PiiResourceStatement& other);

  /**
   * Assign the contents of @p other to this.
   */
  PiiResourceStatement& operator= (const PiiResourceStatement& other);
    
  /**
   * See if this is a valid statement. A statement is valid if and
   * only if both subject and object are non-empty, and the type of
   * the object is not @p InvalidType. The predicate may be an empty
   * string.
   */
  bool isValid() const;

  /**
   * Returns the subject of the statement. The subject is always a
   * resource identifier.
   */
  QString subject() const;
  /**
   * Returns the predicate of the statement. The predicate is always a
   * string literal.
   */
  QString predicate() const;
  /**
   * Returns the object of the statement. The object is either a
   * string literal or a resource identifier, depending on #type().
   */
  QString object() const;
  /**
   * Returns the type of the statement. The type specifies how the
   * object should be interpreted.
   */
  Type type() const;
  /**
   * Returns the id number of the statement. PiiResourceDatabase
   * automatically assigns id numbers to inserted statements.
   */
  int id() const;
  /**
   * Set the resource id.
   */
  void setId(int id);

private:
  class Data
  {
  public:
    Data();
    Data(const char* s,
         const char* p,
         const char* o,
         Type t,
         int i);
    Data(const QString& s,
         const QString& p,
         const QString& o,
         Type t,
         int i);
    ~Data();
    void deleteStrings();
  
    bool bStringData;
    union { void* pSubject; const void* pConstSubject; };
    union { void* pPredicate; const void* pConstPredicate; };
    union { void* pObject; const void* pConstObject; };
    Type type;
    int id;
  } *d;
};

#endif //_PIIRESOURCESTATEMENT_H
