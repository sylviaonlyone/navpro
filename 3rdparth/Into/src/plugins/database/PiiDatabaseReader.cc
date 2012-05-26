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

#include "PiiDatabaseReader.h"

#include <PiiYdinTypes.h>

#include <QSqlQuery>
#include <QSqlDriver>
#include <QFile>

PiiDatabaseReader::Data::Data() :
  pQuery(0),
  pFile(0)
{
}

PiiDatabaseReader::Data::~Data()
{
  delete pQuery;
  delete pFile;
}

PiiDatabaseReader::PiiDatabaseReader() : PiiDatabaseOperation(new Data)
{
  setProtectionLevel("columnNames", WriteWhenStoppedOrPaused);
  setProtectionLevel("defaultValues", WriteWhenStoppedOrPaused);
}

void PiiDatabaseReader::aboutToChangeState(State state)
{
  PII_D;
  if (state == Stopped)
    delete d->pFile, d->pFile = 0;
  PiiDatabaseOperation::aboutToChangeState(state);
}

PiiOutputSocket* PiiDatabaseReader::output(const QString& name) const
{
  const PII_D;
  int index = d->lstColumnNames.indexOf(name);
  if (index != -1)
    return outputAt(index);
  else
    return PiiDefaultOperation::output(name);
}

QSqlDatabase* PiiDatabaseReader::createDatabase(const QString& driver,
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
      if (!d->pFile->open(QIODevice::ReadOnly))
        {
          delete d->pFile, d->pFile = 0;
          error(tr("Could not open %1 for reading.").arg(d->strDatabaseName));
        }
      return 0;
    }
  return PiiDatabaseOperation::createDatabase(driver, user, password, host, port, database);
}

void PiiDatabaseReader::createQuery()
{
  PII_D;
  // PENDING
  QSqlDriver* pDriver = db()->driver();
  QString strQuery("SELECT ");
  for (int i=0; i<d->lstColumnNames.size(); ++i)
    {
      if (i > 0)
        strQuery.append(',');
      strQuery.append(pDriver->escapeIdentifier(d->lstColumnNames[i], QSqlDriver::FieldName));
    }
  strQuery.append(" FROM ");
  strQuery.append(pDriver->escapeIdentifier(d->strTableName, QSqlDriver::TableName));
  d->pQuery = new QSqlQuery(*db());
  d->pQuery->prepare(strQuery);
}

void PiiDatabaseReader::process()
{
  PII_D;
  if (!isConnected() && d->pFile == 0)
    {
      delete d->pQuery, d->pQuery = 0;
      if (openConnection())
        createQuery();
    }
  else if (d->pFile != 0)
    {
      QString strLine(d->pFile->readLine());
      if (!strLine.isEmpty() && strLine[strLine.size()-1] == '\n')
        strLine.chop(1);
      if (!strLine.isEmpty() && strLine[strLine.size()-1] == '\r')
        strLine.chop(1);
      if (strLine.isEmpty())
        operationStopped(); // throws
      QStringList lstParts = Pii::splitQuoted(strLine, ';');
      if (lstParts.size() != d->lstColumnNames.size())
        PII_THROW(PiiExecutionException,
                  tr("CSV file has %1 data fields, expected %2.")
                  .arg(lstParts.size())
                  .arg(d->lstColumnNames.size()));
      for (int i=0; i<lstParts.size(); ++i)
        {
          QString strPart(lstParts[i]);
          switch (d->vecDefaultValues[i].type())
            {
            case PiiVariant::IntType:
              emitObject(strPart.isEmpty() ? d->vecDefaultValues[i] : PiiVariant(strPart.toInt()), i);
              break;
            case PiiVariant::DoubleType:
              emitObject(strPart.isEmpty() ? d->vecDefaultValues[i] : PiiVariant(strPart.toDouble()), i);
              break;
            default:
              emitObject(strPart.isEmpty() ? d->vecDefaultValues[i] : PiiVariant(strPart), i);
              break;
            }
        }
    }
}

void PiiDatabaseReader::setColumnNames(const QStringList& columnNames)
{
  PII_D;
  d->lstColumnNames = columnNames;
  setNumberedOutputs(d->lstColumnNames.size());
  initializeDefaults();
}

QStringList PiiDatabaseReader::columnNames() const { return _d()->lstColumnNames; }

void PiiDatabaseReader::setTableName(const QString& tableName) { _d()->strTableName = tableName; }
QString PiiDatabaseReader::tableName() const { return _d()->strTableName; }

void PiiDatabaseReader::setDefaultValues(const QVariantMap& defaultValues)
{
  PII_D;
  d->mapDefaultValues = defaultValues;
  initializeDefaults();
}

void PiiDatabaseReader::initializeDefaults()
{
  PII_D;
  int iOutputs = outputCount();
  d->vecDefaultValues.resize(iOutputs);

  for (int i=0; i<iOutputs; ++i)
    {
      QVariant varDefault = d->mapDefaultValues[d->lstColumnNames[i]];
      switch (varDefault.type())
        {
        case QVariant::String:
          d->vecDefaultValues[i] = PiiVariant(varDefault.toString());
          break;
        case QVariant::Int:
          d->vecDefaultValues[i] = PiiVariant(varDefault.toInt());
          break;
        case QVariant::Double:
          d->vecDefaultValues[i] = PiiVariant(varDefault.toDouble());
          break;
        default:
          d->vecDefaultValues[i] = PiiVariant();
        }
    }
}

QVariantMap PiiDatabaseReader::defaultValues() const { return _d()->mapDefaultValues; }
