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

#ifndef _PIIDATABASEWRITER_H
#define _PIIDATABASEWRITER_H

#include "PiiDatabaseOperation.h"
#include <QSqlDatabase>

class QFile;
class QSqlQuery;

/**
 * An operation that writes rows into SQL databases and flat files. 
 * The operation has a user-configurable number of inputs that accept
 * primitive values and strings. Whenever all inputs have an incoming
 * object, a new row is created into a database table. The incoming
 * values are written to the table based on the names of the inputs as
 * specified by the @ref columnNames property.
 *
 * This operation adds "csv" as a supported connection scheme. See
 * PiiDatabaseOperation::databaseName for examples.
 *
 * @inputs
 *
 * @in inputX - input sockets. X is a zero-based index. Inputs can
 * also be accessed with the names given by the #columnNames property.
 *
 * @ingroup PiiDatabasePlugin
 */
class PiiDatabaseWriter : public PiiDatabaseOperation
{
  Q_OBJECT

  /**
   * The name of the table data will be written into. The table
   * description must match the column names set with #columnNames.
   */
  Q_PROPERTY(QString tableName READ tableName WRITE setTableName);

  /**
   * Database column names. When this property is set, an equivalent
   * number of input sockets will be created. The values read from
   * these input sockets are stored into the database table to the
   * named column.
   *
   * To create two inputs, do this:
   *
   * @code
   * writer->setProperty("columnNames", QStringList() << "filename" << "objects");
   * @endcode
   */
  Q_PROPERTY(QStringList columnNames READ columnNames WRITE setColumnNames);

  /**
   * A map that speficies default values for some columns. The
   * corresponding inputs will be flagged optional. If they are left
   * unconnected, the default values will always be substituted.
   *
   * @code
   * QVariantMap values;
   * values["filename"] = "N/A";
   * values["objects"] = 0;
   * writer->setProperty("defaultValues", values);
   * @endcode
   */
  Q_PROPERTY(QVariantMap defaultValues READ defaultValues WRITE setDefaultValues);

  /**
   * Turns database writing on and off.
   */
  Q_PROPERTY(bool writeEnabled READ writeEnabled WRITE setWriteEnabled);

  /**
   * The number of decimal digits written to CSV files. The other
   * database back-ends don't treat decimal numbers as text, and are
   * not affected by rounding.
   */
  Q_PROPERTY(int decimalsShown READ decimalsShown WRITE setDecimalsShown);
  
  PII_OPERATION_SERIALIZATION_FUNCTION

public:
  PiiDatabaseWriter();
  ~PiiDatabaseWriter();

  PiiInputSocket* input(const QString& name) const;
  
  void check(bool reset);

protected:
  void process();
  void aboutToChangeState(State state);

  QSqlDatabase* createDatabase(const QString& driver,
                               const QString& user,
                               const QString& password,
                               const QString& host,
                               int port,
                               const QString& database);

  QStringList columnNames() const;
  void setColumnNames(const QStringList& columnNames);

  QVariantMap defaultValues() const;
  void setDefaultValues(const QVariantMap& defaultValues);

  bool writeEnabled() const;
  void setWriteEnabled(bool writeEnabled);

  void setDecimalsShown(int decimalsShown);
  int decimalsShown() const;
  
  QString tableName() const;
  void setTableName(const QString& tableName);

private:
  void initializeDefaults();
  
	/// @internal
  class Data : public PiiDatabaseOperation::Data
  {
  public:
    Data();
    ~Data();
    
    QStringList lstColumnNames;
    QVariantMap mapDefaultValues;
    QVector<QVariant> vecDefaultValues;
    QString strTableName;
    bool bWriteEnabled;
    int iDecimalsShown;
    QSqlQuery* pQuery;
    QFile *pFile;
  };
  PII_D_FUNC;

  void createQuery();
};


#endif //_PIIDATABASEWRITER_H
