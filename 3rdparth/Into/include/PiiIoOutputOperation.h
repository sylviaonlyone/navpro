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

#ifndef _PIIIOOUTPUTOPERATION_H
#define _PIIIOOUTPUTOPERATION_H

#include <PiiDefaultOperation.h>
#include <PiiIoChannel.h>
#include <PiiIoDriver.h>

/**
 * PiiIoOutputOperation description
 *
 * @par Configuring channels
 *
 * Channels are configured with property maps (QVariantMap). See other
 * recognized property values from PiiIoChannel.
 *
 * All channels can be configured at once with the #channelConfigs
 * property. To configure a single channel, use the #channelConfig
 * property.
 *
 * @code
 * // Create a new output signal operation
 * PiiOperation* handler = engine.createOperation("PiiIoOutputOperation");
 *
 * // Configure channel 13
 * QVariantMap props;
 * props["pulseWidth"] = 300;
 * props["pulseDelay"] = 0;
 * props["channel"] = 13;
 * props["index"] = 0;
 * props["channelMode"] = "PulseOutput";
 * props["electricalConnection"] = "IttlConnection";
 *
 * handler->setProperty("channelConfig", props);
 *
 *
 * // To configure channels(14,15,16) at once, do this:
 * QVariantMap props1;
 * props["pulseWidth"] = 300;
 * props["pulseDelay"] = 0;
 * props["channel"] = 14;
 * props["channelMode"] = "PulseOutput";
 * props["electricalConnection"] = "IttlConnection";
 *
 * QVariantMap props2;
 * props["pulseWidth"] = 300;
 * props["pulseDelay"] = 0;
 * props["channel"] = 15;
 * props["channelMode"] = "PulseOutput";
 * props["electricalConnection"] = "IttlConnection";
 *
 * QVariantMap props3;
 * props["pulseWidth"] = 300;
 * props["pulseDelay"] = 0;
 * props["channel"] = 16;
 * props["channelMode"] = "PulseOutput";
 * props["electricalConnection"] = "IttlConnection";
 *
 * QVariantList configs;
 * configs << props1 << props2 << props3;
 * handler->setProperty("channelConfigs", configs);
 * @endcode
 *
 * @inputs
 *
 * @in channel - integer.
 *
 * @in value - double.
 * 
 * @ingroup PiiIoPlugin
 */
class PiiIoOutputOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Configuration values for several channels.
   */
  Q_PROPERTY(QVariantList channelConfigs READ channelConfigs WRITE setChannelConfigs);

  /**
   * Configure a single channel.
   */
  Q_PROPERTY(QVariantMap channelConfig WRITE setChannelConfig STORED false);

  /**
   * ioDriver description
   */
  Q_PROPERTY(QString ioDriver READ ioDriver WRITE setIoDriver);

  /**
   * ioUnit description
   */
  Q_PROPERTY(QString ioUnit READ ioUnit WRITE setIoUnit);

  /**
   * disabled description
   */
  Q_PROPERTY(bool disabled READ disabled WRITE setDisabled);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiIoOutputOperation();
  ~PiiIoOutputOperation();

  void setChannelConfigs(const QVariantList& channelConfigs);
  QVariantList channelConfigs() const;
  
  void setChannelConfig(const QVariantMap& channelConfig);

  void setIoDriver(const QString& ioDriver);
  QString ioDriver() const;

  void setIoUnit(const QString& ioUnit);
  QString ioUnit() const;

  void setDisabled(bool disabled);
  bool disabled() const;

  void check(bool reset);

protected:
  void process();

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();

    QVariantList lstChannelConfigs;
    QList<PiiIoChannel*> lstChannels;
    PiiIoDriver *pIoDriver;
    QString strIoDriver;
    QString strIoUnit;
    bool bDisabled;
  };
  PII_D_FUNC;
};


#endif //_PIIIOOUTPUTOPERATION_H
