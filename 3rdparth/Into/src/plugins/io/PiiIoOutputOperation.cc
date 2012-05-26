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

#include "PiiIoOutputOperation.h"
#include <PiiYdinTypes.h>
#include <QtDebug>
#include <PiiUtil.h>
#include <PiiEngine.h>

PiiIoOutputOperation::Data::Data() :
  pIoDriver(0),
  strIoDriver(""),
  strIoUnit("0"),
  bDisabled(false)
{
}

PiiIoOutputOperation::PiiIoOutputOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("channel"));
  
  PiiInputSocket *socket = new PiiInputSocket("value");
  socket->setOptional(true);
  addSocket(socket);
}

PiiIoOutputOperation::~PiiIoOutputOperation()
{
  PII_D;
  if (d->pIoDriver != 0)
    delete d->pIoDriver;
}

void PiiIoOutputOperation::check(bool reset)
{
  PII_D;
  PiiDefaultOperation::check(reset);

  if (reset)
    {
      if (d->strIoDriver.isEmpty())
        piiWarning(tr("I/O driver has not been set"));
      else
        {
          PiiIoDriver* driver = qobject_cast<PiiIoDriver*>(PiiSerializationFactory::create<QObject>(qPrintable(d->strIoDriver)));

          if (driver != 0)
            {
              delete d->pIoDriver;
              d->pIoDriver = driver;
            }
          else
            PII_THROW(PiiExecutionException, tr("I/O driver (%1) has not been set or is not available.").arg(d->strIoDriver));
      
          if (d->pIoDriver)
            {
              d->pIoDriver->selectUnit(d->strIoUnit);
          
              if (!d->pIoDriver->initialize())
                PII_THROW(PiiExecutionException, tr("Cannot initialize I/O driver."));
            }
        }
    }
  
  //init channels
  d->lstChannels.clear();
  
  if (d->pIoDriver != 0)
    {
      //make channels
      for (int i=0; i<d->lstChannelConfigs.size(); ++i)
        {
          QVariantMap mapChannelConfig = d->lstChannelConfigs[i].toMap();
          d->lstChannels << d->pIoDriver->channel(mapChannelConfig.value("channel", -1).toInt());
          mapChannelConfig.remove("channel");
          if (d->lstChannels[i] != 0)
            Pii::setProperties(*d->lstChannels[i], mapChannelConfig);
        }
    }
}

void PiiIoOutputOperation::setChannelConfigs(const QVariantList& channelConfigs)
{
  PII_D;
  d->lstChannelConfigs = channelConfigs;
}

QVariantList PiiIoOutputOperation::channelConfigs() const
{
  const PII_D;
  return d->lstChannelConfigs;
}

void PiiIoOutputOperation::setChannelConfig(const QVariantMap& channelConfig)
{
  PII_D;
  int size = d->lstChannelConfigs.size();
  int index = channelConfig.value("index",0).toInt();
  if (index < 0)
    index = 0;
  
  if (size > 0 && index < size)
    d->lstChannelConfigs.replace(index,channelConfig);
  else
    {
      //make empty channelconfigs
      for (int i=size; i<index; i++)
        {
          QVariantMap config;
          config["index"] = i;
          config["channelMode"] = "NoMode";
          config["channel"] = -1;
          d->lstChannelConfigs << config;
        }

      d->lstChannelConfigs << channelConfig;
    }

}

void PiiIoOutputOperation::setIoDriver(const QString& ioDriver)
{
  PII_D;
  d->strIoDriver = ioDriver;
}

QString PiiIoOutputOperation::ioDriver() const
{
  const PII_D;
  return d->pIoDriver ? d->pIoDriver->metaObject()->className() : "";
}

void PiiIoOutputOperation::setIoUnit(const QString& ioUnit)
{
  PII_D;
  d->strIoUnit = ioUnit;
}

void PiiIoOutputOperation::process()
{
  PII_D;
  PiiVariant obj = readInput();

  int index = -1;
  switch (obj.type())
    {
      PII_PRIMITIVE_CASES(index = (int)PiiYdin::primitiveAs, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }

  //piiDebug("PiiIoOutputOperation::process(): %d", index);
  if (!d->bDisabled && index >= 0 && index < d->lstChannels.size() && d->lstChannels[index] != 0)
    d->lstChannels[index]->sendSignal();
}


QString PiiIoOutputOperation::ioUnit() const { return _d()->strIoUnit; }
void PiiIoOutputOperation::setDisabled(bool disabled) { _d()->bDisabled = disabled; }
bool PiiIoOutputOperation::disabled() const { return _d()->bDisabled; }
