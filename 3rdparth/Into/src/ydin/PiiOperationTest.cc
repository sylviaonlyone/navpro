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

#include "PiiOperationTest.h"
#include "PiiEngine.h"
#include "PiiProbeInput.h"
#include <PiiSerializableExport.h> // MSVC
#include <PiiDelay.h>
#include <QDebug>

PiiOperationTest::Data::Data() :
  pOperation(0)
{
}

PiiOperationTest::PiiOperationTest() :
  d(new Data)
{
}

PiiOperationTest::~PiiOperationTest()
{
  stop();
  qDeleteAll(d->mapProbes);
  delete d;
}

bool PiiOperationTest::createOperation(const char* plugin,
                                       const char* operation)
{
  try
    {
      PiiEngine::loadPlugin(plugin);
    }
  catch (PiiLoadException& ex)
    {
      qWarning("%s", qPrintable(ex.message()));
      return false;
    }
 
  d->pOperation = PiiSerializationFactory::create<PiiOperation>(operation);
  if (d->pOperation == 0)
    return false;
  d->pOperation->setParent(this);

  return true;
}

void PiiOperationTest::setOperation(PiiOperation* operation)
{
  d->pOperation = operation;
}

PiiOperation* PiiOperationTest::operation() const
{
  return d->pOperation;
}

void PiiOperationTest::cleanup()
{
  clearAllOutputValues();
}

bool PiiOperationTest::start(FailMode mode)
{
  if (d->pOperation == 0)
    {
      qDebug("Operation has not been set.");
      return false;
    }
  try
    {
      createProbes();
      d->pOperation->check(d->pOperation->state() == PiiOperation::Stopped);
      d->pOperation->start();
      if (mode == ExpectFail)
        return false;
      int iCount = 0;
      while (d->pOperation->state() != PiiOperation::Running && iCount < 5)
        {
          QCoreApplication::processEvents();
          PiiDelay::msleep(50);
          ++iCount;
        }
      if (iCount == 5)
        return false;
    }
  catch (PiiException& ex)
    {
      if (mode == ExpectSuccess)
        {
          qDebug("%s", ex.message().toLocal8Bit().constData());
          return false;
        }
    }
  return true;
}

bool PiiOperationTest::connectInput(const QString& name)
{
  PiiAbstractInputSocket* pInput = d->pOperation->input(name);
  if (pInput == 0)
    return false;

  connectInput(pInput);
  return true;
}

void PiiOperationTest::connectInput(PiiAbstractInputSocket* input)
{
  //qDebug("Connecting %s", qPrintable(input->objectName()));
  if (input->connectedOutput() != 0)
    return;
  
  PiiOutputSocket* pOutput = new PiiOutputSocket("");
  pOutput->setParent(this);
  pOutput->connectInput(input);
  // Delete output socket if the input is destroyed
  connect(input->socket(), SIGNAL(destroyed(QObject*)), SLOT(deleteOutput(QObject*)), Qt::DirectConnection);
}

void PiiOperationTest::deleteOutput(QObject* input)
{
  PiiAbstractInputSocket* pInput = dynamic_cast<PiiAbstractInputSocket*>(input);
  if (pInput != 0)
    delete pInput->connectedOutput();
}

void PiiOperationTest::connectAllInputs()
{
  QList<PiiAbstractInputSocket*> lstInputs = d->pOperation->inputs();
  for (int i=lstInputs.size(); i--; )
    connectInput(lstInputs[i]);
}

void PiiOperationTest::disconnectInput(const QString& name)
{
  PiiAbstractInputSocket* pInput = d->pOperation->input(name);
  if (pInput != 0)
    delete pInput->connectedOutput();
}

void PiiOperationTest::disconnectAllInputs()
{
  QList<PiiAbstractInputSocket*> lstInputs = d->pOperation->inputs();
  for (int i=lstInputs.size(); i--; )
    delete lstInputs[i]->connectedOutput();
}

bool PiiOperationTest::sendObject(const QString& name, const PiiVariant& value)
{
  if (d->pOperation->state() != PiiOperation::Running)
    {
      qDebug("Operation is not Running but %s", PiiOperation::stateName(d->pOperation->state()));
      return false;
    }
  PiiAbstractInputSocket* pInput = d->pOperation->input(name);
  return pInput != 0 && pInput->connectedOutput() != 0 &&
    pInput->controller()->tryToReceive(pInput, value);
}

bool PiiOperationTest::sendObject(const QString& name, const char* value)
{
  return sendObject(name, PiiVariant(QString(value)));
}

PiiProbeInput* PiiOperationTest::createProbe(PiiAbstractOutputSocket* output, const QString& name)
{
  PiiProbeInput* pProbe = new PiiProbeInput(name);
  output->connectInput(pProbe);
  connect(pProbe, SIGNAL(objectReceived(PiiVariant)), SLOT(emitObject(PiiVariant)), Qt::DirectConnection);
  connect(output->socket(), SIGNAL(destroyed(QObject*)), SLOT(deleteProbe(QObject*)), Qt::DirectConnection);
  return pProbe;
}

void PiiOperationTest::deleteProbe(QObject* output)
{
  PiiOutputSocket* pOutput = qobject_cast<PiiOutputSocket*>(output);
  if (pOutput != 0)
    {
      QString strName = d->pOperation->socketName(pOutput);
      delete d->mapProbes.take(strName);
    }
}

void PiiOperationTest::createProbes()
{
  qDeleteAll(d->mapProbes);
  d->mapProbes.clear();
  QList<PiiAbstractOutputSocket*> lstOutputs = d->pOperation->outputs();
  for (int i=lstOutputs.size(); i--; )
    {
      QString strName = d->pOperation->socketName(lstOutputs[i]);
      d->mapProbes[strName] = createProbe(lstOutputs[i], strName);
    }
}

bool PiiOperationTest::stop()
{
  if (d->pOperation != 0)
    {
      QList<PiiAbstractInputSocket*> lstInputs(d->pOperation->inputs());
      for (int i=0; i<lstInputs.size(); ++i)
        if (lstInputs[i]->controller() != 0)
          lstInputs[i]->controller()->tryToReceive(lstInputs[i], PiiYdin::createStopTag());

      if (!d->pOperation->wait(1000))
        {
          d->pOperation->interrupt();
          return d->pOperation->wait(1000);
        }
      return true;
    }
  return false;
}

void PiiOperationTest::emitObject(const PiiVariant& obj)
{
  if (sender() != 0)
    emit objectReceived(sender()->objectName(), obj);
}

QStringList PiiOperationTest::outputsWithData() const
{
  QStringList lstResult;
  for (ProbeMapType::const_iterator i=d->mapProbes.begin();
       i != d->mapProbes.end(); ++i)
    if (i.value()->hasSavedObject())
      lstResult << i.key();
  
  return lstResult;
}

PiiVariant PiiOperationTest::outputValue(const QString& name) const
{
  if (d->mapProbes.contains(name))
    return d->mapProbes[name]->savedObject();
  return PiiVariant();
}


bool PiiOperationTest::hasOutputValue(const QString& name) const
{
  return d->mapProbes.contains(name) && d->mapProbes[name]->hasSavedObject();
}

bool PiiOperationTest::hasOutputValue() const
{
  for (ProbeMapType::const_iterator i=d->mapProbes.begin();
       i != d->mapProbes.end(); ++i)
    if (i.value()->hasSavedObject())
      return true;
  return false;
}

void PiiOperationTest::clearOutputValue(const QString& name)
{
  if (d->mapProbes.contains(name))
    d->mapProbes[name]->setSavedObject(PiiVariant());
}

void PiiOperationTest::clearAllOutputValues()
{
  for (ProbeMapType::const_iterator i=d->mapProbes.begin();
       i != d->mapProbes.end(); ++i)
    i.value()->setSavedObject(PiiVariant());
}
