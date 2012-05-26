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

#include "PiiOperationCompound.h"

#include <PiiSerializationFactory.h>
#include <PiiUtil.h>
#include <PiiDelay.h>
#include <QTime>
#include <QtDebug>
#include <PiiYdinUtil.h>
#include <PiiYdinResources.h>
#include <PiiSerializableExport.h>


PII_DEFINE_VIRTUAL_METAOBJECT_FUNCTION(PiiOperationCompound);
PII_SERIALIZABLE_EXPORT(PiiOperationCompound);

static int iOperationCompoundMetaType = qRegisterMetaType<PiiOperationCompound*>("PiiOperationCompound*");

PiiOperationCompound::Data::Data() :
  state(PiiOperation::Stopped),
  bChecked(false),
  bWaiting(false)
{
}

PiiOperationCompound::Data::~Data()
{
}
  
PiiOperationCompound::PiiOperationCompound() :
  PiiOperation(new Data)
{
}

PiiOperationCompound::PiiOperationCompound(Data* data) :
  PiiOperation(data)
{
}

PiiOperationCompound::~PiiOperationCompound()
{
  unexposeAll();
}

void PiiOperationCompound::unexposeAll()
{
  PII_D;
  qDeleteAll(d->lstInputs);
  qDeleteAll(d->lstOutputs);
  d->lstInputs.clear();
  d->lstOutputs.clear();
}

void PiiOperationCompound::clear()
{
  PII_D;
  unexposeAll();
  qDeleteAll(d->lstOperations);
  d->lstOperations.clear();
}

void PiiOperationCompound::check(bool reset)
{
  PII_D;

  // Check for a parent if we haven't been derived.
  if (parent() == 0 &&
      PiiOperationCompound::metaObject() == metaObject())
    piiWarning(tr("%1 (objectName %2) has no parent.")
               .arg(metaObject()->className())
               .arg(objectName()));

  PiiCompoundExecutionException PII_MAKE_EXCEPTION(compoundEx, "");
  
  d->vecChildStates.resize(d->lstOperations.size());
  bool bError = false;
  for (int i=0; i<d->lstOperations.size(); ++i)
    {
      try
        {
          d->lstOperations[i]->check(d->lstOperations[i]->state() == Stopped);
        }
      catch (PiiExecutionException& ex)
        {
          QString strNewMessage = compoundEx.message();
          if (!strNewMessage.isEmpty())
            strNewMessage.append('\n');
          strNewMessage.append(ex.message());
          compoundEx.setMessage(strNewMessage);
          bError = true;
          compoundEx.addException(d->lstOperations[i], ex);
        }
      d->vecChildStates[i] = qMakePair(d->lstOperations[i]->state(), false);
    }
  if (bError)
    throw compoundEx;
  if (reset)
    resetProxies();
  d->bChecked = true;
}

void PiiOperationCompound::start()
{
  PII_D;
  if (!d->bChecked)
    return;
  QMutexLocker lock(&d->stateMutex);
  if (d->state == Stopped || d->state == Paused)
    {
      setState(Starting);
      commandChildren<Start>();
      d->bChecked = false;
    }
}

void PiiOperationCompound::pause()
{
  PII_D;
  QMutexLocker lock(&d->stateMutex);
  if (d->state == Running)
    {
      setState(Pausing);
      commandChildren<Pause>();
    }
}

void PiiOperationCompound::stop()
{
  PII_D;
  QMutexLocker lock(&d->stateMutex);
  if (d->state member_of (Starting, Running, Stopping))
    {
      setState(Stopping);
      commandChildren<Stop>();
    }
}

void PiiOperationCompound::interrupt()
{
  PII_D;
  // Since interrupt() is meant to really stop everything, we send the
  // signals even when the compound indicates it is already stopped.
  QMutexLocker lock(&d->stateMutex);
  if (d->state != Stopped)
    setState(Interrupted);
  
  commandChildren<Interrupt>();
}

void PiiOperationCompound::resetProxies()
{
  PII_D;
  // Reset all proxy inputs and outputs
  for (int i=0; i<d->lstInputs.size(); ++i)
    d->lstInputs[i]->reset();
  for (int i=0; i<d->lstOutputs.size(); ++i)
    d->lstOutputs[i]->reset();
}

/* Check if a child's state change to newState causes a state change
 * on the parent. The state of the parent changes through an
 * intermediate state. (e.g. Running -> Stopping -> Stopped)
 */
bool PiiOperationCompound::checkSteadyStateChange(State newState, State intermediateState, State steadyState)
{
  // If the operation's new state is {Starting,Pausing,Stopping},
  // change state respectively
  if (newState == intermediateState ||
      (steadyState == Stopped && newState == Interrupted))
    setState(newState);
  // If an operation directly changes to the target steady state
  else if (newState == steadyState)
    {
      // Go through the intermediate state
      setState(intermediateState);
      // Everybody is in the steady state, so am I
      if (checkChildStates(steadyState))
        setState(steadyState);
    }
  else
    return false;

  // State changed
  return true;
}

QString PiiOperationCompound::fullName(QObject* operation)
{
  QStringList lstNames;
  while (operation)
    {
      QString strName = operation->metaObject()->className();
      if (!operation->objectName().isEmpty() &&
          operation->objectName() != operation->metaObject()->className())
        strName += "[" + operation->objectName() + "]";
      lstNames.prepend(strName);
      operation = operation->parent();
    }

  return lstNames.join("/");
}

void PiiOperationCompound::childStateChanged(int state)
{
  PII_D;
  QMutexLocker lock(&d->stateMutex);
  //qDebug("\n%-11s %s", stateName((State)state), qPrintable(fullName(sender())));
  //qDebug("%s %s, compound %s", sender()->metaObject()->className(), stateName((State)state), stateName(this->state()));

  int iIndex = d->lstOperations.indexOf(static_cast<PiiOperation*>(sender()));
  d->vecChildStates[iIndex].first = (State)state;
  if (state == Running)
    d->vecChildStates[iIndex].second = true;
  
  switch (d->state)
    {
    case Stopped:
      // Stopped can change to Running
      checkSteadyStateChange((State)state, Starting, Running);
      break;
    case Starting:
      // Starting can change to running ...
      if (checkChildStates(Running))
        {
          setState(Running);
          // Now, if any of the operations already stopped, we must
          // change state.
          for (int i=d->vecChildStates.size(); i--; )
            if (d->vecChildStates[i].first member_of (Stopping, Stopped))
              {
                checkSteadyStateChange(d->vecChildStates[i].first, Stopping, Stopped);
                break;
              }
        }
      // ... or stopped
      // If an operation that was already running stops, we won't
      // change state to Stopping yet.
      else if ((State)state not_member_of (Stopping, Stopped) ||
               !d->vecChildStates[iIndex].second)
        checkSteadyStateChange((State)state, Stopping, Stopped);
      break;
    case Running:
      // Running can change to Paused or Stopped
      if (!checkSteadyStateChange((State)state, Stopping, Stopped))
        checkSteadyStateChange((State)state, Pausing, Paused);
      break;
    case Pausing:
      // Pausing can change to Paused ...
      if (checkChildStates(Paused))
        setState(Paused);
      // ... or stopped
      else
        checkSteadyStateChange((State)state, Stopping, Stopped);
      break;
    case Paused:
      // Paused can change to Stopped and Running
      if (!checkSteadyStateChange((State)state, Stopping, Stopped))
        checkSteadyStateChange((State)state, Starting, Running);
      break;
    case Stopping:
    case Interrupted:
      // Stopping/Interrupted can only change to Stopped
      if (checkChildStates(Stopped))
        setState(Stopped);
      break;
    }
}

// Returns true if all children are in the given state.
bool PiiOperationCompound::checkChildStates(State state)
{
  PII_D;
  for (int i=d->vecChildStates.size(); i--; )
    // If the current state of a child is different from what we
    // expect, fail. But if we are checking for "Running", we need to
    // check if the child has been running.
    if (d->vecChildStates[i].first != state && !(state == Running && d->vecChildStates[i].second))
      return false;
  return true;
}

bool PiiOperationCompound::wait(unsigned long time)
{
  PII_D;
  QTime t;
  t.start();
  // Wait for all children
  bool allDone;
  do
    {
      allDone = true;
      for (int i=0; i<d->lstOperations.size(); ++i)
        {
          allDone = allDone && d->lstOperations[i]->wait(100);
          QCoreApplication::processEvents();
        }
    }
  while (!allDone &&
         (time == ULONG_MAX || static_cast<unsigned long>(t.elapsed()) < time));
  return allDone;
}

bool PiiOperationCompound::wait(State state, unsigned long time)
{
  PII_D;
  if (d->bWaiting)
    {
      if (state == d->state) return true;
      piiWarning(tr("Recursive call to PiiOperationCompound::wait() blocked."));
      return false;
    }
  d->bWaiting = true;
  QTime t;
  t.start();
  while (d->state != state && static_cast<unsigned long>(t.elapsed()) <= time)
    {
      QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents |
                                      QEventLoop::DeferredDeletion |
                                      QEventLoop::ExcludeUserInputEvents,
                                      10);
      PiiDelay::msleep(10);
    }
  d->bWaiting = false;
  return d->state == state;
}

QList<PiiAbstractInputSocket*> PiiOperationCompound::inputs() const
{
  const PII_D;
  QList<PiiAbstractInputSocket*> lstInputs;
  for (int i=0; i < d->lstInputs.size(); ++i)
    lstInputs << d->lstInputs[i]->input();
  return lstInputs;
}

QList<PiiAbstractOutputSocket*> PiiOperationCompound::outputs() const
{
  const PII_D;
  QList<PiiAbstractOutputSocket*> lstOutputs;
  for (int i=0; i < d->lstOutputs.size(); ++i)
    lstOutputs << d->lstOutputs[i]->output();
  return lstOutputs;
}

int PiiOperationCompound::inputCount() const
{
  return _d()->lstInputs.size();
}

int PiiOperationCompound::outputCount() const
{
  return _d()->lstOutputs.size();
}

QVariant PiiOperationCompound::socketProperty(PiiAbstractSocket* socket, const char* name) const
{
  const PII_D;
  QString alias;

  // Search outputs
  for (int i=0; i<d->lstOutputs.size(); ++i)
    if (d->lstOutputs[i]->socket() == socket)
      {
        if (PiiYdin::isNameProperty(name))
          return d->lstOutputs[i]->name();
        PiiAbstractOutputSocket* pRootOutput = d->lstOutputs[i]->output()->rootOutput();
        PiiOperation* pParentOp = qobject_cast<PiiOperation*>(pRootOutput->socket()->parent());
        return pParentOp->socketProperty(pRootOutput, name);
      }

  if (PiiYdin::isNameProperty(name))
    {
      // Search inputs
      for (int i=0; i<d->lstInputs.size(); ++i)
        if (d->lstInputs[i]->socket() == socket)
          return d->lstInputs[i]->name();

      // No match found in my aliases -> search child operations
  
      return internalName(socket);
    }
  return QVariant();
}

QString PiiOperationCompound::internalName(PiiAbstractSocket* socket) const
{
  if (socket == 0)
    return QString();
  
  const PII_D;
  QList<PiiOperation*> parents =
    Pii::findAllParents<PiiOperation*>(socket->socket());
  
  // Find a parent operation that is in our operation list 
  for (int i=parents.size(); i--; )
    {
      int childIndex = d->lstOperations.indexOf(parents[i]);
      // Yes, this parent is owned by us. Let it decide the name of the object.
      if (childIndex != -1)
        return d->lstOperations[childIndex]->objectName() + '.' +
          d->lstOperations[childIndex]->socketName(socket);
    }
  
  // Not in my children...
  return QString();
}

void PiiOperationCompound::createInputProxy(const QString& alias)
{
  PII_D;
  if (!d->lstInputs.contains(alias))
    {
      ExposedSocket *pSocket = new ExposedSocket(alias, this);
      pSocket->expose(static_cast<PiiAbstractInputSocket*>(0), ProxyConnection);
      d->lstInputs.append(pSocket);
    }
}

void PiiOperationCompound::createOutputProxy(const QString& alias)
{
  PII_D;
  if (!d->lstOutputs.contains(alias))
    {
      ExposedSocket *pSocket = new ExposedSocket(alias, this);
      pSocket->expose(static_cast<PiiAbstractOutputSocket*>(0), ProxyConnection);
      d->lstOutputs.append(pSocket);
    }
}

PiiProxySocket* PiiOperationCompound::inputProxy(const QString& alias) const
{
  ExposedSocket* pSocket = _d()->lstInputs[alias];
  if (pSocket != 0)
    return pSocket->proxy();

  return 0;
}

PiiProxySocket* PiiOperationCompound::outputProxy(const QString& alias) const
{
  ExposedSocket* pSocket = _d()->lstOutputs[alias];
  if (pSocket != 0)
    return pSocket->proxy();
  return 0;
}

void PiiOperationCompound::exposeInput(PiiAbstractInputSocket* socket,
                                       const QString& alias,
                                       ConnectionType connectionType)
{
  PII_D;
  // Make sure input is exposed only once
  unexposeInput(socket);
  ExposedSocket* pSocket = d->lstInputs[alias];
  if (pSocket == 0)
    {
      pSocket = new ExposedSocket(alias, this);
      d->lstInputs.append(pSocket);
    }
  pSocket->expose(socket, connectionType);

  // Break connection when the socket is destroyed.
  connect(socket->socket(),
          SIGNAL(destroyed(QObject*)),
          SLOT(removeInput(QObject*)),
          Qt::DirectConnection);
}

void PiiOperationCompound::exposeOutput(PiiAbstractOutputSocket* socket,
                                       const QString& alias,
                                       ConnectionType connectionType)
{
  PII_D;
  ExposedSocket* pSocket = d->lstOutputs[alias];
  if (pSocket == 0)
    {
      pSocket = new ExposedSocket(alias, this);
      d->lstOutputs.append(pSocket);
    }
  pSocket->expose(socket, connectionType);
  
  // Break connection when the socket is destroyed
  connect(socket->socket(),
          SIGNAL(destroyed(QObject*)),
          SLOT(removeOutput(QObject*)),
          Qt::DirectConnection);
}

void PiiOperationCompound::exposeInput(const QString& fullName, const QString& alias, ConnectionType connectionType)
{
  PiiAbstractInputSocket* pInput = input(fullName);
  if (pInput)
    exposeInput(pInput, alias, connectionType);
  else
    piiWarning(tr("There is no \"%1\" input in %2.").arg(fullName).arg(metaObject()->className()));
}

void PiiOperationCompound::exposeInputs(const QStringList& fullNames, const QString& alias)
{
  for (int i=fullNames.size(); i--; )
    exposeInput(fullNames[i], alias);
}
  
void PiiOperationCompound::exposeOutput(const QString& fullName, const QString& alias, ConnectionType connectionType)
{
  PiiAbstractOutputSocket* pOutput = output(fullName);
  if (pOutput)
    exposeOutput(pOutput, alias, connectionType);
  else
    piiWarning(tr("There is no \"%1\" output in %2.").arg(fullName).arg(metaObject()->className()));
}

void PiiOperationCompound::unexposeInput(PiiAbstractInputSocket* input)
{
  if (input == 0) return;
  PII_D;
  for (int i=d->lstInputs.size(); i--; )
    {
      if (d->lstInputs[i]->isProxy())
        d->lstInputs[i]->proxy()->disconnectInput(input);
      else if (d->lstInputs[i]->input() == input)
        delete d->lstInputs.takeAt(i);
    }
}

void PiiOperationCompound::unexposeOutput(PiiAbstractOutputSocket* output)
{
  if (output == 0) return;
  PII_D;
  for (int i=d->lstOutputs.size(); i--; )
    {
      if (d->lstOutputs[i]->isProxy() && d->lstOutputs[i]->proxy()->connectedOutput() == output)
        d->lstOutputs[i]->proxy()->disconnectOutput();
      else if (d->lstOutputs[i]->output() == output)
        delete d->lstOutputs.takeAt(i);
    }
}

void PiiOperationCompound::unexposeInput(const QString& alias)
{
  delete _d()->lstInputs.take(alias);
}

void PiiOperationCompound::unexposeOutput(const QString& alias)
{
  delete _d()->lstOutputs.take(alias);
}

template <class Socket> void PiiOperationCompound::remove(QObject* socket, SocketList& sockets)
{
  for (int i=sockets.size(); i--; )
    {
      if (sockets[i]->qObject() == socket)
        {
          delete sockets.takeAt(i);
          return;
        }
    }
  piiCritical("Could not find aliased socket to be removed.");
}

void PiiOperationCompound::removeInput(QObject* socket)
{
  remove<PiiInputSocket>(socket, _d()->lstInputs);
}

void PiiOperationCompound::removeOutput(QObject* socket)
{
  remove<PiiOutputSocket>(socket, _d()->lstOutputs);
}

QList<PiiOperation*> PiiOperationCompound::childOperations() const
{
  return _d()->lstOperations;
}

int PiiOperationCompound::childCount() const
{
  return _d()->lstOperations.size();
}

void PiiOperationCompound::addOperation(PiiOperation* op)
{
  PII_D;
  QMutexLocker lock(&d->stateMutex);
  // Operations can only be added when the operation is stopped or
  // paused
  if (op == 0 || d->state not_member_of (Stopped, Paused))
    return;

  if (!d->lstOperations.contains(op))
    {
      // Don't allow empty names
      if (op->objectName().isEmpty())
        {
          QString className(op->metaObject()->className());
          int i = 1;
          QString name = className;
          // Find a unique objectName
          while (findChildOperation(name))
            name = className + QString::number(i++);
          op->setObjectName(name);
        }

      d->lstOperations.append(op);
      op->setParent(this);
      connect(op, SIGNAL(errorOccured(PiiOperation*,const QString&)), SLOT(handleError(PiiOperation*,const QString&)), Qt::QueuedConnection);
      connect(op, SIGNAL(stateChanged(int)), SLOT(childStateChanged(int)), Qt::QueuedConnection);
      connect(op, SIGNAL(destroyed(QObject*)), SLOT(childDestroyed(QObject*)), Qt::DirectConnection);
    }
}


void PiiOperationCompound::childDestroyed(QObject* op)
{
  PII_D;
  PiiOperation* child = static_cast<PiiOperation*>(op);
  if (child)
    {
      d->lstOperations.removeAll(child);
      if (d->state not_member_of (Stopped, Paused))
        emit errorOccured(child ? child : this, tr("An operation was destroyed while its parent was being executed."));
    }
}

void PiiOperationCompound::handleError(PiiOperation* sender, const QString& msg)
{
  piiCritical("An error occured with %s (%s): %s",
              sender->metaObject()->className(),
              piiPrintable(sender->objectName()),
              piiPrintable(msg));
  emit errorOccured(sender, msg);
  interrupt();
}

bool PiiOperationCompound::replaceOperation(PiiOperation *oldOp, PiiOperation* newOp)
{
  PII_D;
  QMutexLocker lock(&d->stateMutex);
  // Operations can only be replaced when the operation is stopped or
  // paused
  if (d->state not_member_of (Stopped, Paused) ||
      !d->lstOperations.contains(oldOp) ||
      d->lstOperations.contains(newOp))
    return false;

  if (newOp != 0)
    {
      addOperation(newOp);
  
      //get all inputs and outputs from the old operation
      QList<PiiAbstractInputSocket*> lstOldInputs = oldOp->inputs();
      QList<PiiAbstractOutputSocket*> lstOldOutputs = oldOp->outputs();
  
      //get all inputs and outputs from the new operation
      QList<PiiAbstractInputSocket*> lstNewInputs = newOp->inputs();
      QList<PiiAbstractOutputSocket*> lstNewOutputs = newOp->outputs();

      //disconnect all inputs and outputs from the new operation
      for (int i=0; i<lstNewInputs.size(); ++i)
        lstNewInputs[i]->disconnectOutput();
      for (int i=0; i<lstNewOutputs.size(); ++i)
        lstNewOutputs[i]->disconnectInput();
  
      //connect all inputs
      for (int i=0; i<lstOldInputs.size(); ++i)
        {
          PiiAbstractOutputSocket *pOutput = lstOldInputs[i]->connectedOutput();
          // This input is not connected
          if (pOutput == 0)
            continue;
          
          // Try to find a socket with a matching name.
          PiiAbstractInputSocket* pInput = newOp->input(oldOp->socketName(lstOldInputs[i]));
          if (pInput != 0)
            pOutput->connectInput(pInput);
          else if (i < lstNewInputs.size())
            pOutput->connectInput(lstNewInputs[i]);

          lstOldInputs[i]->disconnectOutput();
        }

      //connect all outputs
      for (int i=0; i<lstOldOutputs.size(); ++i)
        {
          QList<PiiAbstractInputSocket*> lstInputs = lstOldOutputs[i]->connectedInputs();
          if (lstInputs.isEmpty())
            continue;
          lstOldOutputs[i]->disconnectInput();
          
          PiiAbstractOutputSocket* pOutput = newOp->output(oldOp->socketName(lstOldOutputs[i]));
          if (pOutput != 0 && i < lstNewOutputs.size())
            pOutput = lstNewOutputs[i];
          if (pOutput != 0)
            for (int j=0; j<lstInputs.size(); ++j)
              pOutput->connectInput(lstInputs[j]);
        }
    }

  //remove the old operation
  d->lstOperations.removeAll(oldOp);
  oldOp->disconnect(this);
  oldOp->setParent(0);
  
  return true;
}

void PiiOperationCompound::removeOperation(PiiOperation* op)
{
  PII_D;
  QMutexLocker lock(&d->stateMutex);
  // Operations can only be removed when the operation is stopped or paused
  if (op == 0 || d->state not_member_of (Stopped, Paused))
    return;

  op->disconnectAllInputs();
  op->disconnectAllOutputs();
  d->lstOperations.removeAll(op);
  op->disconnect(this);
  op->setParent(0);
}

struct PiiOperationCompound::InputFinder
{
  typedef PiiAbstractInputSocket* Type;
  InputFinder(const SocketList& inputs) : lstInputs(inputs) {}
  Type find(PiiOperation* op, const QString& path) const { return op->input(path); }
  Type get(const QString& name) const
  {
    ExposedSocket* s = lstInputs[name];
    return s ? s->input() : 0;
  }
  const SocketList& lstInputs;
};

struct PiiOperationCompound::OutputFinder
{
  typedef PiiAbstractOutputSocket* Type;
  OutputFinder(const SocketList& outputs) : lstOutputs(outputs) {}
  Type find(PiiOperation* op, const QString& path) const { return op->output(path); }
  Type get(const QString& name) const
  {
    ExposedSocket* s = lstOutputs[name];
    return s ? s->output() : 0;
  }
  const SocketList& lstOutputs;
};

struct PiiOperationCompound::SetPropertyFinder
{
  typedef bool Type;
  SetPropertyFinder(PiiOperation* self, const QVariant& value) : _self(self), _value(value) {}
  Type find(PiiOperation* op, const QString& path) const { return op->setProperty(qPrintable(path), _value); }
  Type get(const QString& name) const { return _self->PiiOperation::setProperty(qPrintable(name), _value); }
private:
  PiiOperation* _self;
  const QVariant& _value;
};

struct PiiOperationCompound::GetPropertyFinder
{
  typedef QVariant Type;
  GetPropertyFinder(PiiOperation* self) : _self(self) {}
  Type find(PiiOperation* op, const QString& path) const { return op->property(qPrintable(path)); }
  Type get(const QString& name) const { return _self->PiiOperation::property(qPrintable(name)); }
private:
  PiiOperation* _self;
};

PiiAbstractInputSocket* PiiOperationCompound::input(const QString& path) const
{
  return find(InputFinder(_d()->lstInputs), path);
}

PiiAbstractOutputSocket* PiiOperationCompound::output(const QString& path) const
{
  return find(OutputFinder(_d()->lstOutputs), path);
}

bool PiiOperationCompound::setProperty(const char* name, const QVariant& value)
{
  return find(SetPropertyFinder(this, value), name);
}

QVariant PiiOperationCompound::property(const char* name)
{
  return find(GetPropertyFinder(this), name);
}

template <class Finder> typename Finder::Type PiiOperationCompound::find(Finder f, const QString& path) const
{
  // First dot-separated part is the child name
  int dotIndex = path.indexOf('.');
  // There are no dots -> must be leaf node
  if (dotIndex == -1)
    return f.get(path);

  QString childName = path.mid(0, dotIndex);

  // Store the rest of the name
  QString rest;
  if (dotIndex < path.size()-1)
    rest = path.mid(dotIndex+1);

  // Special name for the parent operation -> we are not responsible
  // for the rest of the name.
  if (childName == "parent")
    {
      PiiOperation* parentOp = qobject_cast<PiiOperation*>(parent());
      if (parentOp)
        return f.find(parentOp, rest);
      return 0;
    }
  
  // Try to find the named (or indexed) child in our children
  PiiOperation* op = findChildOperation(childName);
  
  if (op != 0) // we found the child -> let it parse the rest
    return f.find(op, rest);

  return 0;
}

PiiOperation* PiiOperationCompound::findChildOperation(const QString& childName) const
{
  const PII_D;
  // Loop through all child operations to find the named operation
  for (int i=d->lstOperations.size(); i--; )
    if (d->lstOperations[i]->objectName() == childName)
      return d->lstOperations[i];
  return 0;
}

void PiiOperationCompound::setState(State state)
{
  PII_D;
  if (d->state != state)
    {
      aboutToChangeState(state);
      d->state = state;
      emit stateChanged(state);
    }
}

void PiiOperationCompound::aboutToChangeState(State /*newState*/)
{}

PiiOperation::State PiiOperationCompound::state() const
{
  return _d()->state;
}

PiiOperation* PiiOperationCompound::createOperation(const QString& className, const QString& objectName)
{
  PiiOperation* op = PiiYdin::createResource<PiiOperation>(qPrintable(className));

  // We got the pointer -> set className and add to operation list
  if (op != 0)
    {
      if (!objectName.isEmpty())
        op->setObjectName(objectName);
      addOperation(op);
    }
  return op;
}


PiiOperationCompound::EndPointType PiiOperationCompound::locateSocket(PiiAbstractSocket* socket,
                                                                      const PiiOperationCompound* context) const
{
  PiiSocket* pSocket = socket->socket();
  if (context == 0)
    {
      PiiOperation* parent = Pii::findFirstParent<PiiOperation*>(pSocket);
      return EndPointType(parent, parent->socketName(socket));
    }
  else
    {
      QList<QObject*> lstParents = Pii::findAllParents(pSocket);
      
      if (lstParents.indexOf(const_cast<PiiOperationCompound*>(context)) != -1)
        return EndPointType(const_cast<PiiOperationCompound*>(context), context->internalName(socket));
      else
        return EndPointType(0, pSocket->objectName());
    }
}

PiiOperationCompound::EndPointListType
PiiOperationCompound::buildEndPointList(PiiAbstractOutputSocket* output,
                                        const PiiOperationCompound* context) const
{
  EndPointListType result;

  /*
    Store all connection end points for the given output socket
   
    We only look for internal connections between operations here. If
    the output socket is aliased, its external connections will be
    cloned/serialized by the parent operation.
   
    If the output is exposed through a proxy, the connections to the
    proxy's inputs need not to be returned because proxies take care
    of their connections.
   */ 
  QList<PiiAbstractInputSocket*> lstInputs = output->connectedInputs();
  for (int i=0; i<lstInputs.size(); ++i)
    {
      PiiSocket* pInput = lstInputs[i]->socket();
      // I'm the direct father of my own output proxies ...
      // The input is not owned by any of my child operations ...
      if (pInput->parent() == this ||
          !Pii::isParent(this, pInput))
        continue; // skip this input
      
      result << locateSocket(lstInputs[i], context);
    }
  return result;
}

PiiOperationCompound* PiiOperationCompound::clone() const
{
  const PII_D;
  // objectName() won't work for template classes, but we have the
  // serialization meta object...
  const char *className = PiiYdin::resourceName(this);
  
  // Use the serialization factory to create an instance of the most
  // derived class.
  PiiOperationCompound* pResult = PiiYdin::createResource<PiiOperationCompound>(className);
  
  if (pResult == 0) return 0;

  // Get rid of everything but the compound (sub-operations, aliases)
  pResult->clear();

  // Set properties if we are the most derived class.
  if (PiiOperationCompound::metaObject() == metaObject())
    Pii::setPropertyList(*pResult, Pii::propertyList(*this));
  
  // Clone sub-operations
  for (int i=0; i<d->lstOperations.size(); ++i)
    {
      PiiOperation* pChildClone = d->lstOperations[i]->clone();
      if (pChildClone == 0)
        {
          delete pResult;
          return 0;
        }
      pResult->addOperation(pChildClone);
    }
  
  // Clone input aliases
  for (int i=0; i<d->lstInputs.size(); ++i)
    {
      ExposedSocket* pSocket = d->lstInputs[i];
      
      // Expose inputs as they are exposed in this operation
      if (pSocket->isProxy())
        {
          pResult->createInputProxy(pSocket->name());
          EndPointListType lstExposedInputs = buildEndPointList(pSocket->output(), this);
          for (int j=0; j<lstExposedInputs.size(); ++j)
            pResult->exposeInput(lstExposedInputs[j].second,
                                 pSocket->name());
        }
      else
        {
          pResult->exposeInput(locateSocket(pSocket->input(), this).second,
                               pSocket->name(),
                               AliasConnection);
        }
    }

  // Clone output aliases
  for (int i=0; i<d->lstOutputs.size(); ++i)
    {
      ExposedSocket* pSocket = d->lstOutputs[i];

      if (pSocket->isProxy())
        {
          pResult->createOutputProxy(pSocket->name());
          PiiAbstractOutputSocket* pInternalOutput = pSocket->input()->connectedOutput();
          if (pInternalOutput != 0)
            pResult->exposeOutput(locateSocket(pInternalOutput, this).second,
                                  pSocket->name());
        }
      else
        pResult->exposeOutput(locateSocket(pSocket->output(), this).second,
                              pSocket->name(),
                              AliasConnection);
    }

  // Clone internal connections
  for (int i=0; i<d->lstOperations.size(); ++i)
    {
      // Find all output sockets
      QList<PiiAbstractOutputSocket*> lstOutputs = d->lstOperations[i]->outputs();
 
      // Clone all connections starting from this operation
      for (int j=0; j<lstOutputs.size(); ++j)
        {
          // Find all internal end points for this output socket.
          EndPointListType lstInputs = buildEndPointList(lstOutputs[j], this);
          
          for (int k=0; k<lstInputs.size(); ++k)
            {
              PiiAbstractInputSocket* target = pResult->input(lstInputs[k].second);

              // The index of the source operation is the same in the
              // clone's child list. The name of the output in the
              // context of our child operation is used to find the
              // output in the cloned operation.
              PiiAbstractOutputSocket *source = pResult->_d()->lstOperations[i]->output(d->lstOperations[i]->socketName(lstOutputs[j]));

              if (source != 0 && target != 0)
                source->connectInput(target);
            }
        }
    }
  return pResult;
}

PiiOperationCompound::ExposedSocket::ExposedSocket(const QString& name, QObject* parent) :
  _strName(name),
  _type(PiiSocket::Proxy),
  _pSocket(0),
  _pQObject(0),
  _pParent(parent)
{
}

PiiOperationCompound::ExposedSocket::~ExposedSocket()
{
  switch (_type)
    {
    case PiiSocket::Proxy:
      // Deleting a proxy automatically breaks all connections
      delete _pSocket;
      break;
    case PiiSocket::Output:
      {
        // Disconnect all outgoing connections
        QList<PiiAbstractInputSocket*> lstOutgoing = outgoingConnections();
        for (int i=lstOutgoing.size(); i--; )
          lstOutgoing[i]->disconnectOutput();
      }
      break;
    case PiiSocket::Input:
      input()->disconnectOutput();
    }
}

QList<PiiAbstractInputSocket*> PiiOperationCompound::ExposedSocket::outgoingConnections() const
{
  QList<PiiAbstractInputSocket*> lstOutgoing;
  if (output() != 0)
    {
      QList<PiiAbstractInputSocket*> lstAll = output()->connectedInputs();
      for (int i=lstAll.size(); i--; )
        if (!Pii::isParent(_pParent, lstAll[i]->socket()))
          lstOutgoing << lstAll[i];
    }
  return lstOutgoing;
}


void PiiOperationCompound::ExposedSocket::expose(PiiAbstractOutputSocket* outputSocket,
                                                 ConnectionType connectionType)
{
  // If the new connection is an alias, destroy a possible proxy and
  // store the aliased output.
  if (connectionType == AliasConnection)
    {
      // If a proxy exists, reconnect its existing inputs to the new
      // exposed output.
      QList<PiiAbstractInputSocket*> lstConnectedInputs;
      if (proxy() != 0)
        {
          lstConnectedInputs = proxy()->connectedInputs();
          delete _pSocket;
        }
      else if (output() != 0)
        lstConnectedInputs = output()->connectedInputs();
      
      for (int i=0; i<lstConnectedInputs.size(); ++i)
        outputSocket->connectInput(lstConnectedInputs[i]);

      _pSocket = outputSocket;
      _type = PiiSocket::Output;
      _pQObject = outputSocket->socket();
    }
  else
    {
      // Am I already a proxy?
      if (proxy() != 0) // yep, just reconnect the input
        outputSocket->connectInput(proxy());
      else // No, need to create a proxy
        {
          // Store old outgoing connections
          QList<PiiAbstractInputSocket*> lstOutgoing = outgoingConnections();

          // Create a proxy
          _pSocket = new PiiProxySocket;
          _type = PiiSocket::Proxy;

          // Reconnect old outgoing connections
          for (int i=0; i<lstOutgoing.size(); ++i)
            proxy()->connectInput(lstOutgoing[i]);

          // Connect the aliased output to the proxy
          if (outputSocket != 0)
            outputSocket->connectInput(proxy());

          proxy()->setParent(_pParent);
        }
      _pQObject = proxy()->socket();
    }
}

void PiiOperationCompound::ExposedSocket::expose(PiiAbstractInputSocket* inputSocket,
                                                 ConnectionType connectionType)
{
  // If the new connection is an alias, destroy a possible proxy and
  // store the aliased input.
  if (connectionType == AliasConnection)
    {
      // If a proxy exists, connect the output currently connected to
      // the proxy input to the new exposed input
      PiiAbstractOutputSocket *pOutput = 0;
      if (proxy() != 0)
        {
          pOutput = proxy()->connectedOutput();
          delete _pSocket;
        }
      else if (input() != 0)
        pOutput = input()->connectedOutput();

      inputSocket->connectOutput(pOutput);
      
      _pSocket = inputSocket;
      _type = PiiSocket::Input;
      _pQObject = inputSocket->socket();
    }
  else
    {
      // Am I already a proxy?
      if (proxy() != 0) // yep, just add a new internal connection
        {
          proxy()->connectInput(inputSocket);
        }
      else // No, need to create a proxy
        {
          // If the input is connected, store the connected output
          PiiAbstractOutputSocket* pConnectedOutput = 0;
          if (input() != 0)
            pConnectedOutput = input()->connectedOutput();

          _pSocket = new PiiProxySocket;
          _type = PiiSocket::Proxy;

          // Reconnect old connection
          if (pConnectedOutput)
            pConnectedOutput->connectInput(proxy());

          // Connect the aliased input to the proxy
          proxy()->connectInput(inputSocket);
          proxy()->setParent(_pParent);
        }
      _pQObject = proxy()->socket();
    }
}

PiiOperationCompound::ExposedSocket*
PiiOperationCompound::SocketList::operator[] (const QString& name) const
{
  for (int i=0; i<size(); ++i)
    if (operator[](i)->name() == name)
      return operator[](i);
  return 0;
}

PiiOperationCompound::ExposedSocket*
PiiOperationCompound::SocketList::operator[] (PiiAbstractOutputSocket* socket) const
{
  if (socket == 0) return 0;
  for (int i=0; i<size(); ++i)
    {
      ExposedSocket* pSocket = operator[](i);
      if ((pSocket->isProxy() && pSocket->proxy()->connectedOutput() == socket) ||
          (!pSocket->isProxy() && pSocket->output() == socket))
        return pSocket;
    }
  return 0;
}

bool PiiOperationCompound::SocketList::contains(const QString& name) const
{
  for (int i=0; i<size(); ++i)
    if (operator[](i)->name() == name)
      return true;
  return false;
}

PiiOperationCompound::ExposedSocket* PiiOperationCompound::SocketList::take(const QString& name)
{
  for (int i=0; i<size(); ++i)
    if (operator[](i)->name() == name)
      return takeAt(i);
  return 0;
}
