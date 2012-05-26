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

#ifndef _PIIDATABASEOPERATION_H
#define _PIIDATABASEOPERATION_H

#include <PiiDefaultOperation.h>
#include "PiiDatabaseGlobal.h"

class QSqlDatabase;
class QSqlQuery;
class QSqlDriver;

/**
 * An abstract superclass for operations that read/write databases. 
 * This class provides functionality for maintaining the database
 * connection.
 *
 * @note Due to limitations of the low-level SQL drivers, database
 * queries must be always made from the thread that initiated the
 * driver. Therefore, %PiiDatabaseOperation disallows changing its
 * @ref PiiDefaultOperation::processingMode "processing mode".
 *
 * @ingroup PiiDatabasePlugin
 */
class PII_DATABASE_EXPORT PiiDatabaseOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Database URL specified the details necessary for connecting to a
   * database. The format is
   * scheme://[user:password@]host[:port][/database]. Bracketed parts
   * are optional. "scheme" tells the database type. Valid values are:
   *
   * @lip db2 - IBM DB2, v7.1 and higher
   * @lip ibase - Borland InterBase
   * @lip mysql - MySQL
   * @lip oci - Oracle
   * @lip odbc - ODBC (includes Microsoft SQL Server)
   * @lip psql - PostgreSQL 7.3 and above
   * @lip sqlite - SQLite version 3 or above
   * @lip sqlite2 - SQLite version 2
   * @lip tds - Sybase Adaptive Server
   * @lip qt - use a connection created by
   * QSqlDatabase::addDatabase(). If the database name is empty, the
   * default database connection will be used.
   * @lip null - no output (default value)
   *
   * Some examples:
   *
   * @lip mysql://user:password\@10.10.10.2/dumpdb
   * @lip null://
   * @lip qt://main
   *
   * @see #databaseName
   */
  Q_PROPERTY(QString databaseUrl READ databaseUrl WRITE setDatabaseUrl);

  /**
   * The name of the database. Database naming depends on the database
   * engine. For Oracle, use the TNS service name. For ODBC, use
   * either a DSN, a DSN file name (must end with .dsn), or a
   * connection string.
   *
   * @code
   * PiiOperation* pDb = engine.createOperation("PiiDatabaseWriter");
   *
   * // Open a Microsoft Access database
   * pDb->setProperty("databaseUrl", "odbc://");
   * pDb->setProperty("databaseName",
   *                  "Driver={Microsoft Access Driver (*.mdb)};"
   *                  "Dbq=accessfile.mdb;"
   *                  "Uid=Admin;Pwd=;");.
   *
   * // Append to or read from a CSV file
   * pDb->setProperty("databaseUrl", "csv://");
   * pDb->setProperty("databaseName", "outputfile.csv");
   * @endcode
   *
   * The database name can also be omitted. In such a case you can set
   * it directly with @p databaseUrl.
   */
  Q_PROPERTY(QString databaseName READ databaseName WRITE setDatabaseName);

  /**
   * By default, all errors cause a run-time exception and stop the
   * operation. Setting this flag to @p true suppresses the exceptions
   * and allows the operation to continue running. A log message will
   * be generated instead.
   */
  Q_PROPERTY(bool ignoreErrors READ ignoreErrors WRITE setIgnoreErrors);

  /**
   * The number of times to retry a failed query before giving up. If
   * #ignoreErrors is @p false, the operation will stop after this
   * many unsuccessful retry attempts. Default is 0. Only
   * connection-related errors (not SQL errors) may cause a retrial.
   */
  Q_PROPERTY(int retryCount READ retryCount WRITE setRetryCount);

  /**
   * The number of milliseconds to wait after a failed query. Default
   * is 50.
   */
  Q_PROPERTY(int retryDelay READ retryDelay WRITE setRetryDelay);

public:
  PiiDatabaseOperation();
  ~PiiDatabaseOperation();

protected:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    
    QString strConnectionId;
    QString strDatabaseUrl, strDatabaseName;

    QSqlDatabase* pDb;

    int iRetryDelay;
    int iRetryCount;
    bool bConnected, bIgnoreErrors;
  };
  PII_D_FUNC;

  /// @internal
  PiiDatabaseOperation(Data* d);

  QString databaseUrl() const;
  void setDatabaseUrl(const QString& databaseUrl);

  QString databaseName() const;
  void setDatabaseName(const QString& databaseName);

  bool ignoreErrors() const;
  void setIgnoreErrors(bool ignoreErrors);

  int retryDelay() const;
  void setRetryDelay(int retryDelay);

  int retryCount() const;
  void setRetryCount(int retryCount);

  /**
   * Opens a database connection to the given #databaseUrl. This
   * function must be called from the process() function to ensure the
   * database is always accessed from the same thread. This is a
   * limitation of the low-level SQL drivers.
   *
   * @return @p true if the connection was successfully opened, @p
   * false otherwise.
   *
   * @exception PiiExecutionException& if the connection cannot be
   * opened and #ignoreErrors is @p false.
   */
  bool openConnection();
  
  /**
   * Closes the database connection.
   */
  void closeConnection();

  /**
   * Returns a pointer to the internal database connection, or a null
   * pointer if no connection is open.
   */
  QSqlDatabase* db();

  /**
   * Shorthand for db()->driver().
   */
  QSqlDriver* driver();

  /**
   * Returns @p true if the database connection is open and @p false
   * otherwise.
   */
  bool isConnected() const;

  /**
   * Returns a pointer to a newly allocated QSqlDatabase object. 
   * Subclasses may override this function to provide new connection
   * schemes and to perform additional initialization.
   */
  virtual QSqlDatabase* createDatabase(const QString& driver,
                                       const QString& user,
                                       const QString& password,
                                       const QString& host,
                                       int port,
                                       const QString& database);

  /**
   * Throws a PiiExecutionException with the given @a message, if
   * #ignoreErrors is @p false. Otherwise, writes @a message to log.
   */
  void error(const QString& message);

  /**
   * Executes @a query. If the query fails with a connection-related
   * error, retries #retryCount times.
   */
  bool exec(QSqlQuery& query);
  /**
   * Checks an executed @a query for errors. Returns @p true if the
   * query was successfully executed, @p false otherwise. Throws a
   * PiiExecutionException if the query failed and #ignoreErrors is @p
   * false.
   */
  bool checkQuery(QSqlQuery& query);
  /**
   * Closes the database connection if @a state is @p Stopped.
   */
  void aboutToChangeState(State state);

private:
  int defaultPort(const QString& driver);
  static int _iConnectionIndex;
};


#endif //_PIIDATABASEOPERATION_H

