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

#include "PiiDatabaseWriter.h"

#include <QSqlDriver>
#include <QMetaType>
#include <QSqlQuery>
#include <QFile>

using namespace PiiYdin;

PiiDatabaseWriter::Data::Data() :
  bWriteEnabled(true),
  iDecimalsShown(0),
  pQuery(0),
  pFile(0)
{
}

PiiDatabaseWriter::Data::~Data()
{
  delete pQuery;
  delete pFile;
}

PiiDatabaseWriter::PiiDatabaseWriter() :
  PiiDatabaseOperation(new Data)
{
  setProtectionLevel("columnNames", WriteWhenStoppedOrPaused);
  setProtectionLevel("defaultValues", WriteWhenStoppedOrPaused);
}

PiiDatabaseWriter::~PiiDatabaseWriter()
{
  closeConnection();
}

void PiiDatabaseWriter::aboutToChangeState(State state)
{
  PII_D;
  if (state == Stopped)
    {
      delete d->pFile, d->pFile = 0;
      delete d->pQuery, d->pQuery = 0;
    }
  PiiDatabaseOperation::aboutToChangeState(state);
}

void PiiDatabaseWriter::check(bool reset)
{
  PiiDefaultOperation::check(reset);
  
  for (int i=0; i<inputCount(); i++)
    if (inputAt(i)->isConnected())
      return;
  PII_THROW(PiiExecutionException, tr("At least one input must be connected."));
}

void PiiDatabaseWriter::createQuery()
{
  PII_D;
  // Prepare an SQL query
  QSqlDriver* pDriver = db()->driver();
  QString strQuery = QString("INSERT INTO %1 (").arg(pDriver->escapeIdentifier(d->strTableName, QSqlDriver::TableName));
  QString strValues;
  for (int i=0; i<d->lstColumnNames.size(); i++)
    {
      if (i)
        {
          strQuery.append(',');
          strValues.append(',');
        }
      strQuery.append(pDriver->escapeIdentifier(d->lstColumnNames[i], QSqlDriver::FieldName));
      strValues.append('?');
    }
  strQuery.append(QString(") VALUES (%1)").arg(strValues));
  d->pQuery = new QSqlQuery(*db());
  d->pQuery->prepare(strQuery);
}

QSqlDatabase* PiiDatabaseWriter::createDatabase(const QString& driver,
                                                const QString& user,
                                                const QString& password,
                                                const QString& host,
                                                int port,
                                                const QString& database)
{
  PII_D;
  delete d->pFile, d->pFile = 0;
  if (driver == "csv")
    {
      d->pFile = new QFile(d->strDatabaseName);
      if (!d->pFile->open(QIODevice::Append))
        {
          delete d->pFile, d->pFile = 0;
          error(tr("Could not open %1 for writing.").arg(d->strDatabaseName));
        }
      return 0;
    }
  return PiiDatabaseOperation::createDatabase(driver, user, password, host, port, database);
}


void PiiDatabaseWriter::process()
{
  PII_D;
  if (!d->bWriteEnabled)
    return;
  
  if (!isConnected() && d->pFile == 0)
    {
      delete d->pQuery, d->pQuery = 0;
      if (openConnection())
        createQuery();
    }
  
  QStringList row;
  for (int i=0; i<inputCount() && i<d->lstColumnNames.size(); i++)
    {
      QVariant value;
      // If the input is connected, we must convert its value to a QVariant

      if (inputAt(i)->isConnected())
        {
          PiiVariant obj = inputAt(i)->firstObject();
          switch (obj.type())
            {
              PII_PRIMITIVE_CASES(value = qVariantAs, obj);
            case PiiYdin::QStringType:
              value = obj.valueAs<QString>();
              break;
            default:
              PII_THROW_UNKNOWN_TYPE(inputAt(i));
            }
        }
      // If the input is not connected, we take a default value for it
      else
        value = d->vecDefaultValues[i];

      // Decimal numbers may need rounding
      if (d->iDecimalsShown > 0 && value.type() == QVariant::Double)
        row << QString().setNum(value.toDouble(), 'f', d->iDecimalsShown);
      else
        row << value.toString();

      if (d->pQuery != 0)
        // Bind the value to a prepared query
        d->pQuery->bindValue(i, value.toString());
    }

  if (d->pQuery != 0)
    {
      // Try to execute the query
      exec(*d->pQuery);
    }
  else if (d->pFile != 0)
    {
      for (int i=0; i<row.size(); i++)
        {
          if (i)
            d->pFile->putChar(',');
          QString value = row[i].replace('"', "\"\"");
          d->pFile->putChar('"');
          d->pFile->write(value.toUtf8());
          d->pFile->putChar('"');
        }
      d->pFile->putChar('\n');
      d->pFile->flush();
    }
}

PiiInputSocket* PiiDatabaseWriter::input(const QString& name) const
{
  const PII_D;
  int index = d->lstColumnNames.indexOf(name);
  if (index != -1)
    return inputAt(index);
  else
    return PiiDefaultOperation::input(name);
}

void PiiDatabaseWriter::setColumnNames(const QStringList& columnNames)
{
  PII_D;
  d->lstColumnNames = columnNames;
  setNumberedInputs(d->lstColumnNames.size());
  initializeDefaults();
}

void PiiDatabaseWriter::setDefaultValues(const QVariantMap& defaultValues)
{
  PII_D;
  d->mapDefaultValues = defaultValues;
  initializeDefaults();
}

void PiiDatabaseWriter::initializeDefaults()
{
  PII_D;
  int iInputs = inputCount();
  d->vecDefaultValues.resize(iInputs);

  for (int i=0; i<iInputs; ++i)
    {
      inputAt(i)->setOptional(d->mapDefaultValues.contains(d->lstColumnNames[i]));
      d->vecDefaultValues[i] = d->mapDefaultValues[d->lstColumnNames[i]];
    }
}

void PiiDatabaseWriter::setWriteEnabled(bool writeEnabled) { _d()->bWriteEnabled = writeEnabled; }
bool PiiDatabaseWriter::writeEnabled() const { return _d()->bWriteEnabled; }

void PiiDatabaseWriter::setDecimalsShown(int decimalsShown) { _d()->iDecimalsShown = decimalsShown; }
int PiiDatabaseWriter::decimalsShown() const { return _d()->iDecimalsShown; }

void PiiDatabaseWriter::setTableName(const QString& tableName) { _d()->strTableName = tableName; }
QString PiiDatabaseWriter::tableName() const { return _d()->strTableName; }

QStringList PiiDatabaseWriter::columnNames() const { return _d()->lstColumnNames; }
QVariantMap PiiDatabaseWriter::defaultValues() const { return _d()->mapDefaultValues; }
