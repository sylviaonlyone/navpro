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

#ifndef _PIIDATABASE_H
#define _PIIDATABASE_H

#include "PiiDatabaseGlobal.h"
#include <QString>

namespace PiiDatabase
{
  /**
   * Create a database connection to @p databaseUri.
   *
   * @param databaseUri the URI of the database. The general syntax is
   * @p driver://user:password\@host:port/database. E.g. @p
   * psql://me:secret\@localhost/mybase
   *
   * @param connectionName the name of the connection. The database
   * can be later retrieved with QSqlDatabase::database() with the
   * given name. If this parameter is omitted, the new database
   * connection becomes the default connection.
   *
   * @code
   * PiiDatabase::createDb("mysql://localhost/test", "myConnection");
   * QSqlDatabase db = QSqlDatabase::database("myConnection");
   * if (db.isValid())
   *   doSomething();
   * @endcode
   */
  bool PII_DATABASE_EXPORT createDb(const QString& databaseUri, const QString& connectionName = "");
}

#endif //_PIIDATABASE_H
