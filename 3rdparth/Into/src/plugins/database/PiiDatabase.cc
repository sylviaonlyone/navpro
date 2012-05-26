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

#include "PiiDatabase.h"
#include <QUrl>
#include <QSqlDatabase>

namespace PiiDatabase
{
  bool createDb(const QString& databaseUri, const QString& connectionName)
  {
    QUrl dbUrl(databaseUri);
    if (!dbUrl.isValid())
      {
        piiWarning("Database URI is incorrectly formatted.");
        return false;
      }
  
    QString driverName = QString("Q%1").arg(dbUrl.scheme().toUpper());
    if (!QSqlDatabase::isDriverAvailable(driverName))
      {
        piiWarning("There is no database driver available for %s.", qPrintable(dbUrl.scheme()));
        return false;
      }
    // Get rid of "/"
    QString databaseName = dbUrl.path().remove(0,1);
  
    QSqlDatabase db = QSqlDatabase(QSqlDatabase::addDatabase(driverName, connectionName));

    if (!db.isValid())
      return false;
  
    db.setHostName(dbUrl.host());
    db.setDatabaseName(databaseName);
    db.setUserName(dbUrl.userName());
    db.setPassword(dbUrl.password());
    if (dbUrl.port() != -1)
      db.setPort(dbUrl.port());

    return true;
  }
}
