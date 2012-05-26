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

#include "PiiResourceStatement.h"



PiiResourceStatement::Data::Data() :
  bStringData(false),
  pSubject(0),
  pPredicate(0),
  pObject(0),
  type(InvalidType),
  id(-1)
{
}

PiiResourceStatement::Data::Data(const char* s,
                                 const char* p,
                                 const char* o,
                                 Type t,
                                 int i) :
  bStringData(false),
  pConstSubject(s),
  pConstPredicate(p),
  pConstObject(o),
  type(t),
  id(i)
{
}

PiiResourceStatement::Data::Data(const QString& s,
                                 const QString& p,
                                 const QString& o,
                                 Type t,
                                 int i) :
  bStringData(true),
  pSubject(new QString(s)),
  pPredicate(new QString(p)),
  pObject(new QString(o)),
  type(t),
  id(i)
{
}

PiiResourceStatement::Data::~Data()
{
  if (bStringData)
    deleteStrings();
}

void PiiResourceStatement::Data::deleteStrings()
{
  delete reinterpret_cast<QString*>(pSubject);
  delete reinterpret_cast<QString*>(pPredicate);
  delete reinterpret_cast<QString*>(pObject);
}

PiiResourceStatement::PiiResourceStatement(const char* s,
                                           const char* p,
                                           const char* o,
                                           Type t,
                                           int i) :
  d(new Data(s, p, o, t, i))
{
}

PiiResourceStatement::PiiResourceStatement(const QString& s,
                                           const QString& p,
                                           const QString& o,
                                           Type t,
                                           int i) :
  d(new Data(s, p, o, t, i))
{
}

PiiResourceStatement::PiiResourceStatement(int s,
                                           const QString& p,
                                           const QString& o,
                                           Type t,
                                           int i) :
  d(new Data(QString("#%1").arg(s), p, o, t, i))
{
}

PiiResourceStatement::~PiiResourceStatement()
{
  delete d;
}

PiiResourceStatement::PiiResourceStatement(const PiiResourceStatement& other) :
  d(other.d->bStringData ?
    new Data(other.subject(), other.predicate(), other.object(), other.type(), other.id()) :
    new Data(reinterpret_cast<char*>(other.d->pSubject),
             reinterpret_cast<char*>(other.d->pPredicate),
             reinterpret_cast<char*>(other.d->pObject),
             other.type(), other.id()))
{
}

PiiResourceStatement& PiiResourceStatement::operator= (const PiiResourceStatement& other)
{
  if (&other != this)
    {
      if (other.d->bStringData)
        {
          d->deleteStrings();
          d->pSubject = new QString(other.subject());
          d->pPredicate = new QString(other.predicate());
          d->pObject = new QString(other.object());
          d->bStringData = true;
        }
      else
        {
          d->pSubject = other.d->pSubject;
          d->pPredicate = other.d->pPredicate;
          d->pObject = other.d->pObject;
          d->bStringData = false;
        }
      d->type = other.d->type;
      d->id = other.d->id;
    }
  return *this;
}

bool PiiResourceStatement::isValid() const
{
  return !subject().isEmpty() && !object().isEmpty() && d->type != InvalidType;
}

QString PiiResourceStatement::subject() const
{
  return d->bStringData ? *reinterpret_cast<QString*>(d->pSubject) : reinterpret_cast<char*>(d->pSubject);
}

QString PiiResourceStatement::predicate() const
{
  return d->bStringData ? *reinterpret_cast<QString*>(d->pPredicate) : reinterpret_cast<char*>(d->pPredicate);
}

QString PiiResourceStatement::object() const
{
  return d->bStringData ? *reinterpret_cast<QString*>(d->pObject) : reinterpret_cast<char*>(d->pObject);
}

PiiResourceStatement::Type PiiResourceStatement::type() const
{
  return d->type;
}

int PiiResourceStatement::id() const
{
  return d->id;
}

void PiiResourceStatement::setId(int id)
{
  d->id = id;
}
