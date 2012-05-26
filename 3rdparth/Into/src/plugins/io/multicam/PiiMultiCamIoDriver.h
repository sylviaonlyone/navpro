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

#ifndef _PIIMULTICAMIODRIVER_H
#define _PIIMULTICAMIODRIVER_H

#include "PiiDefaultIoDriver.h"
#include "PiiMultiCamIoDriverGlobal.h"
#include <PiiMultiCamIoChannel.h>
#include <QVector>
#include <multicam.h>

class PII_MULTICAMIODRIVER_EXPORT PiiMultiCamIoDriver : public PiiDefaultIoDriver
{
  Q_OBJECT

public:
  PiiMultiCamIoDriver();
  ~PiiMultiCamIoDriver();
  
  bool selectUnit(const QString& unit);
  bool initialize();
  bool close();
  int channelCount() const;
  bool isOpen();

  PiiIoChannel* createChannel(int channel);
  
private:
  
  /// @internal
  class Data : public PiiDefaultIoDriver::Data
  {
  public:
    Data();

    int iBoard;
    bool bOpen;
  };
  PII_D_FUNC;
};

#endif //_PIIMULTICAMIODRIVER_H
