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

#include "PiiResourceDatabase.h"

namespace Pii
{
  Subject subject;
  Predicate predicate;
  Object object;
  Attribute attribute("");
  ResourceType resourceType;
  StatementId statementId;
}

PiiResourceDatabase::PiiResourceDatabase() :
  d(new Data)
{
}

PiiResourceDatabase::~PiiResourceDatabase()
{
  delete d;
}

int PiiResourceDatabase::generateId()
{
  // PENDING overflow handling
  return d->lstStatements.size() > 0 ? d->lstStatements.last().id() + 1 : 0;
}

int PiiResourceDatabase::addStatement(const PiiResourceStatement& statement)
{
  PiiResourceStatement copy(statement);
  int id = generateId();
  copy.setId(id);
  d->lstStatements << copy;
  return id;
}

int PiiResourceDatabase::addStatement(const char* subject,
                                      const char* predicate,
                                      const char* object,
                                      PiiResourceStatement::Type type)
{
  int id = generateId();
  d->lstStatements << PiiResourceStatement(subject, predicate, object, type, id);
  return id;
}

int PiiResourceDatabase::addStatement(const QString& subject,
                                      const QString& predicate,
                                      const QString& object,
                                      PiiResourceStatement::Type type)
{
  int id = generateId();
  d->lstStatements << PiiResourceStatement(subject, predicate, object, type, id);
  return id;
}

int PiiResourceDatabase::addStatement(int subject,
                                      const QString& predicate,
                                      const QString& object,
                                      PiiResourceStatement::Type type)
{
  return addStatement(QString("#%1").arg(subject), predicate, object, type);
}

QList<int> PiiResourceDatabase::addStatements(const QList<PiiResourceStatement>& statements)
{
  QList<int> lstIds;
  int id = -1;
  for (int i=0; i<statements.size(); ++i)
    {
      const PiiResourceStatement& s = statements[i];
      // "#" refers to the id of the previous statement (that is not a
      // reification of another statement)
      if (s.subject() == "#" && i != 0)
        lstIds << addStatement(id, s.predicate(), s.object(), s.type());
      else
        {
          id = addStatement(s);
          lstIds << id;
        }
    }
  return lstIds;
}

void PiiResourceDatabase::removeStatements(const QList<int>& ids)
{
  for (int i=0; i<ids.size(); ++i)
    removeStatement(ids[i]);
}

void PiiResourceDatabase::removeStatement(int id)
{
  for (int i=0; i<d->lstStatements.size(); ++i)
    {
      if (d->lstStatements[i].id() == id)
        {
          d->lstStatements.removeAt(i);
          return;
        }
    }
}

QList<PiiResourceStatement> PiiResourceDatabase::statements() const
{
  return d->lstStatements;
}

int PiiResourceDatabase::statementCount() const
{
  return d->lstStatements.size();
}

void PiiResourceDatabase::dump() const
{
  for (int i=0; i<d->lstStatements.size(); ++i)
    {
      QString strStatement("(%1, %2, %4) #%3");
      strStatement = strStatement
        .arg(d->lstStatements[i].subject(),
             d->lstStatements[i].predicate())
        .arg(d->lstStatements[i].id());
      if (d->lstStatements[i].type() == PiiResourceStatement::LiteralType)
        strStatement = strStatement.arg(QString("\"%1\"").arg(d->lstStatements[i].object()));
      else
        strStatement = strStatement.arg(d->lstStatements[i].object());
      qDebug("%s", strStatement.toLocal8Bit().constData());
    }
}
