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

#ifndef _PIIMULTICAMIOCHANNEL_H
#define _PIIMULTICAMIOCHANNEL_H

#include "PiiDefaultIoChannel.h"
#include "PiiMultiCamIoDriverGlobal.h"
class PiiMultiCamIoDriver;

/**
 * I/O channel accessed through the MultiCam driver.
 *
 * @ingroup PiiIoPlugin
 */
class PII_MULTICAMIODRIVER_EXPORT PiiMultiCamIoChannel : public PiiDefaultIoChannel
{
  Q_OBJECT

  /**
   * electricalConnection description
   */
  Q_PROPERTY(ElectricalConnection electricalConnection READ electricalConnection WRITE setElectricalConnection);
  Q_ENUMS(ElectricalConnection);
  
public:
  enum ElectricalConnection
    {
      NoConnection,
      TtlConnection,
      IttlConnection,
      CmosConnection,
      ChannelLinkConnection,
      LvdsConnection,
      V5Connection,
      V12Connection,
      IV12Connection,
      SsRelayConnection,
      RelayConnection,
      AutoConnection,
      OcConnection,
      IocConnection,
      IoeConnection,
      OptoConnection
    };
  
  PiiMultiCamIoChannel(PiiMultiCamIoDriver *parent, int channel, int board);
  
  void setElectricalConnection(const ElectricalConnection& electricalConnection);
  ElectricalConnection electricalConnection() const;

  void changeOutputState(bool active);
  
protected:
  int currentState() const;
  void checkMode();

private:
  /// @internal
  class Data : public PiiDefaultIoChannel::Data
  {
  public:
    Data(int board);

    int iBoard;
    ElectricalConnection electricalConnection;
  };
  PII_D_FUNC;
};

#endif //_PIIMULTICAMIOCHANNEL_H
