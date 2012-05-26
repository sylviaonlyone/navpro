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

#ifndef _PIIOPERATIONCOMPOUND_H
# error "Never use <PiiOperationCompound-templates.h> directly; include <PiiOperationCompound.h> instead."
#endif

template <class Archive> void PiiOperationCompound::save(Archive& archive, const unsigned int /*version*/)
{
  PII_D;
  PII_SERIALIZE_BASE(archive, PiiOperation);

  // Save child operations
  PII_SERIALIZE_NAMED(archive, d->lstOperations, "operations");

  {
    // Save input aliases
    int inputCnt = d->lstInputs.size();
    PII_SERIALIZE(archive, inputCnt);

    for (int i=0; i<inputCnt; ++i)
      {
        ExposedSocket* pSocket = d->lstInputs[i];
        // For each input, its name is first stored
        QString name = pSocket->name();
        PII_SERIALIZE(archive, name);

        // Is it a proxy or not?
        bool proxy = pSocket->isProxy();
        PII_SERIALIZE(archive, proxy);

        // Store connected inputs
        EndPointListType inputs = proxy ?
          buildEndPointList(pSocket->output()) :
          EndPointListType() << locateSocket(pSocket->input());

        PII_SERIALIZE(archive, inputs);
      }
  }

  {
    // Save output aliases
    int outputCnt = d->lstOutputs.size();
    PII_SERIALIZE(archive, outputCnt);

    for (int i=0; i<outputCnt; ++i)
      {
        ExposedSocket* pSocket = d->lstOutputs[i];
        // For each output, its name is first stored
        QString name = pSocket->name();
        PII_SERIALIZE(archive, name);

        // Is it a proxy?
        bool proxy = pSocket->isProxy();
        PII_SERIALIZE(archive, proxy);

        EndPointType output = locateSocket(pSocket->isProxy() ?
                                           pSocket->input()->connectedOutput() :
                                           pSocket->output());
        PII_SERIALIZE(archive, output);
      }
  }
  
  for (int i=0; i<d->lstOperations.size(); i++)
    {
      // Find all output sockets
      QList<PiiAbstractOutputSocket*> outputs = d->lstOperations[i]->outputs();
      
      // Store the number of outputs
      int cnt = outputs.size();
      PII_SERIALIZE(archive, cnt);

      // Store all connections starting from this operation
      for (int j=0; j<outputs.size(); j++)
        {
          // Store the name of the start point
          QString name = d->lstOperations[i]->socketName(outputs[j]);
          PII_SERIALIZE(archive, name);

          // Store all end points
          EndPointListType inputs = buildEndPointList(outputs[j]);
          PII_SERIALIZE(archive, inputs);
        }
    }

  // Save properties if this is the most derived class
  if (PiiOperationCompound::metaObject() == metaObject())
    PiiSerialization::saveProperties(archive, *this);
}

template <class Archive> void PiiOperationCompound::load(Archive& archive, const unsigned int version)
{
  PII_D;
  PII_SERIALIZE_BASE(archive, PiiOperation);

  // Restore operations
  QList<PiiOperation*> operations;
  PII_SERIALIZE_NAMED(archive, operations, "operations");
  for (int i=0; i<operations.size(); i++)
    addOperation(operations[i]);

  // Load input aliases
  {
    int inputCnt;
    PII_SERIALIZE(archive, inputCnt);

    while (inputCnt-- > 0)
      {
        QString name;
        PII_SERIALIZE(archive, name);
        bool proxy;
        PII_SERIALIZE(archive, proxy);
        EndPointListType inputs;
        PII_SERIALIZE(archive, inputs);
        for (int i=0; i<inputs.size(); ++i)
          {
            if (inputs[i].first)
              {
                PiiAbstractInputSocket* s = inputs[i].first->input(inputs[i].second);
                if (s)
                  exposeInput(s, name, proxy ? ProxyConnection : AliasConnection);
              }
          }
      }
  }
  // Load output aliases
  {
    int outputCnt;
    PII_SERIALIZE(archive, outputCnt);
    
    while (outputCnt-- > 0)
      {
        QString name;
        PII_SERIALIZE(archive, name);
        bool proxy;
        PII_SERIALIZE(archive, proxy);
        EndPointType output;
        PII_SERIALIZE(archive, output);
        if (output.first)
          {
            PiiAbstractOutputSocket* s = output.first->output(output.second);
            if (s)
              exposeOutput(s, name, proxy ? ProxyConnection : AliasConnection);
          }
      }
  }

  // Restore connections for each operation
  for (int i=0; i<d->lstOperations.size(); i++)
    {
      // Read the number of stored outputs
      int cnt;
      PII_SERIALIZE(archive, cnt);

      // Read connection specs
      while (cnt-- > 0)
        {
          // Read starting point name
          QString name;
          PII_SERIALIZE(archive, name);

          // Read end points
          EndPointListType inputs;
          PII_SERIALIZE(archive, inputs);

          PiiAbstractOutputSocket* source = d->lstOperations[i]->output(name);
          if (!source)
            continue;

          // Connect to each end point
          for (int j=0; j<inputs.size(); j++)
            {
              PiiAbstractInputSocket* target = inputs[j].first->input(inputs[j].second);
              if (target != 0)
                source->connectInput(target);
            }
        }
    }

  // Load properties if this is not the most derived class
  if (PiiOperationCompound::metaObject() == metaObject() && version > 0)
    PiiSerialization::loadProperties(archive, *this);
}
