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

#include "PiiYdinUtil.h"
#include "PiiOperation.h"
#include "PiiYdinTypes.h"

namespace PiiYdin
{
  static QString dumpInputs(PiiAbstractOutputSocket* socket)
  {
    QString strResult;
    QList<PiiAbstractInputSocket*> lstInputs = socket->connectedInputs();
    for (int i=0; i<lstInputs.size(); ++i)
      {
        if (i > 0)
          strResult += " ";
        PiiOperation* pParent = Pii::findFirstParent<PiiOperation*>(lstInputs[i]->socket());
        if (pParent != 0)
          strResult += QString("%1.%2").arg(pParent->objectName(),
                                            pParent->socketName(lstInputs[i]));
        else
          strResult += "<unknown>." + lstInputs[i]->socket()->objectName();
      }
    return strResult;
  }

  QString illustrateOperation(PiiOperation* op, IllustrationFlags flags)
  {
    QList<PiiAbstractInputSocket*> inputs = op->inputs();
    QList<PiiAbstractOutputSocket*> outputs = op->outputs();

    QString strTitle(op->metaObject()->className());
    QString strName;
    if (!op->objectName().isEmpty() && strTitle != op->objectName())
      strName = QString("(%1)").arg(op->objectName());

    int iMaxSize = qMax(strTitle.size(), strName.size());
    int iMaxQueueLength = 0;
    for (int i=0; i<inputs.size(); i++)
      {
        PiiInputSocket* pSocket = static_cast<PiiInputSocket*>(inputs[i]->socket());
        QString name = op->socketName(inputs[i]);
        if (name.size() > iMaxSize)
          iMaxSize = name.size();
        if (pSocket != 0 && pSocket->queueLength() > iMaxQueueLength)
          iMaxQueueLength = pSocket->queueLength();
      }
    for (int i=0; i<outputs.size(); i++)
      {
        QString name = op->socketName(outputs[i]);
        if (name.size() > iMaxSize)
          iMaxSize = name.size();
      }

    QString result;
    if (flags & ShowState)
      {
        QString strState = PiiOperation::stateName(op->state());
        if (strState.size() > iMaxSize)
          iMaxSize = strState.size();
        result = QString("%0%1\n").arg("", iMaxQueueLength+2).arg(strState);
      }

    result += QString("%0+%1+\n%0| %2 |\n").
      arg("", iMaxQueueLength).
      arg('-', iMaxSize+3, QChar('-')).
      arg(strTitle, -(iMaxSize+1));

    if (strName.size() > 0)
      result += QString("%0| %1 |\n").arg("", iMaxQueueLength).arg(strName, -(iMaxSize+1));

    result += QString("%0+%1+\n").arg("", iMaxQueueLength).arg('-', iMaxSize+3, QChar('-'));

    for (int i=0; i<inputs.size(); i++)
      {
        PiiInputSocket* pSocket = static_cast<PiiInputSocket*>(inputs[i]->socket());
        QString name = op->socketName(inputs[i]);
        if ((flags & ShowInputQueues) &&
            pSocket != 0 &&
            pSocket->isConnected())
          {
            for (int j=pSocket->queueLength(); j<iMaxQueueLength; ++j)
              result += ' ';
            for (int j=pSocket->queueLength(); j--; )
              {
                const PiiVariant& obj = pSocket->objectAt(j);
                if (!obj.isValid())
                  {
                    result += '0';
                    continue;
                  }
                switch (obj.type())
                  {
                  case PiiYdin::SynchronizationTagType:
                    result += obj.valueAs<int>() > 0 ? '>' : '<';
                    break;
                  case PiiYdin::StopTagType: result += 'S'; break;
                  case PiiYdin::PauseTagType: result += 'P'; break;
                  case PiiYdin::ResumeTagType: result += 'R'; break;
                  default:
                    result += '.';
                  }
              }
          }
        else
          result += QString("%1").arg("", iMaxQueueLength);
        result += QString("|> %1 |\n").arg(name, -iMaxSize);
      }
    for (int i=0; i<outputs.size(); i++)
      {
        QString name = op->socketName(outputs[i]);
        result += QString("%0| %1 >|").arg("", iMaxQueueLength).arg(name, iMaxSize);
        if (flags & ShowOutputStates)
          result += " " + dumpInputs(outputs[i]);
        result += '\n';
      }
    result += QString("%0+%1+\n").arg("", iMaxQueueLength).arg('-', iMaxSize+3, QChar('-'));
    return result;
  }
  
  void dumpOperation(PiiOperation *op,  int level, IllustrationFlags flags)
  {
    PiiOperationCompound* compound = qobject_cast<PiiOperationCompound*>(op);
    QString padding;
    padding.fill(' ', level*2);
    if (compound != 0)
      {
        qDebug("%s******** [ %s (%s) ] ********\n", qPrintable(padding), compound->metaObject()->className(), qPrintable(compound->objectName()));
        QList<PiiOperation*> kids = compound->childOperations();
        for (int i=0; i<kids.size(); ++i)
          dumpOperation(kids[i], level + 1, flags);
        qDebug("%s******** [/%s (%s) ] ********\n", qPrintable(padding), compound->metaObject()->className(), qPrintable(compound->objectName()));
      }
    else
      {
        QString strIllustration = illustrateOperation(op, flags);
        strIllustration = padding + strIllustration.replace("\n", "\n" + padding);
        qDebug("%s", strIllustration.toUtf8().constData());
      }
  }
}
