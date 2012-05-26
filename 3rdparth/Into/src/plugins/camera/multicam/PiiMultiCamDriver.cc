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

#include "PiiMultiCamDriver.h"
#include <PiiMetaConfigurationValue.h>
#include "../serial/PiiCameraLinkSerialDevice.h"
#include "../protocols/PiiCameraConfigurationProtocol.h"
#include <PiiWaitingIODevice.h>
#include <PiiEngine.h>
#include <QFileInfo>
#include <QSettings>
#include <QStringList>
#include <PiiFraction.h>
#include <QtDebug>
#include <cstring> // for strncmp(), strlen()

#define MULTICAM_THROW(MESSAGE, CODE) PII_THROW(PiiCameraDriverException, MESSAGE + tr(" Error code: %1").arg(CODE))

// Automatically deletes a MultiCam handle when destroyed. Used to
// protect allocated handles against exceptions.
struct AutoHandle
{
  AutoHandle(MCHANDLE handle = 0) : h(handle) {}
  ~AutoHandle() { if (h != 0) McDelete(h); }
  operator MCHANDLE() const { return h; }
  MCHANDLE release() { MCHANDLE result = h; h = 0; return result; }
  MCHANDLE h;
};

extern "C" void multiCamCaptureFunction(PMCCALLBACKINFO callBackInfo)
{
  if (callBackInfo->Signal == MC_SIG_SURFACE_PROCESSING )
    {
      PiiMultiCamDriver *pDriver = reinterpret_cast<PiiMultiCamDriver*>(callBackInfo->Context);
      pDriver->surfaceFilled();
    }
  else if (callBackInfo->Signal == MC_SIG_ACQUISITION_FAILURE)
    {
      qWarning("Acquisition FAILURE!");
      qWarning("SignalInfo = %i", callBackInfo->SignalInfo);
    }
}

PiiMultiCamDriver::Data::Data() :
  pAllocatedBuffer(0), pFrameBuffer(0),
  iBufferMemory(1024 * 1024), // Default size is 1 MB
  iBufferWidth(0), iBufferHeight(0), iFrameHeight(0), iSignalInterval(16),
  iLastCapturedBuffer(0),
  bOpen(false),
  iBoard(0), iBoardType(0),
  cameraLinkMode(Base1T8), triggerSource(NoTrigger), dTriggerRate(1.0),
  hChannel(0),
  surfaceMutex(QMutex::NonRecursive),
  captureThread(this),
  pSerialDevice(0),
  pSerialProtocol(0)
{
}

PiiMultiCamDriver::PiiMultiCamDriver()
{
  PII_D;
  // Initialize the _mapTempCameraSettings
  // In the next two lines it is assumed that the member variable
  // _cameraLinkMode is first initialized above.
  storeChannelCountForCamera();
  storeBitsPerPixelForCamera();
  d->mapTempCameraSettings["triggerMode"] = 1; // FreeRun
}

PiiMultiCamDriver::~PiiMultiCamDriver()
{
  PII_D;
  if (d->bOpen)
    close();
  if (d->pSerialProtocol)
    delete d->pSerialProtocol;
  if (d->pSerialDevice)
    delete d->pSerialDevice;
}

bool PiiMultiCamDriver::setConfigurationValue(const char* name, const QVariant& value)
{
  PII_D;
  // First it is checked, if the name of the configuration value
  // contains the namespace "camera". If it does, the configuration
  // value is assumed to be the configuration value of the serial protocol
  // object, and the corresponding configuration value is set. Else
  // the the configuration value is assumed to be the
  // property of this class, and the corresponding property is set.
  if (strlen(name) > 7 && strncmp(name, "camera.", 7) == 0)
    {
      const char* shortName = name+7;
      // Set the configuration value of the camera.
      if (d->pSerialProtocol)  
        return d->pSerialProtocol->setConfigurationValue(shortName, value); //7 == strlen("driver.")
      else
        d->mapTempCameraSettings[shortName] = value;
      return true;
    }
  else
    return setProperty(name,value);
}

QVariant PiiMultiCamDriver::configurationValue(const char* name) const
{
  const PII_D;
  // First it is checked, if the name of the configuration value
  // contains the namespace "camera". If it does, the configuration
  // value is assumed to be the configuration value of the serial protocol
  // object, and the corresponding configuration value is returned.
  // Else the the configuration value is assumed to be the
  // property of this class, and the corresponding property is returned.
  if (strlen(name)>7 &&  strncmp(name, "camera.", 7)==0)
    {
      const char* shortName = name+7;
      // Set the configuration value of the camera.
      if (d->pSerialProtocol)
        return d->pSerialProtocol->getConfigurationValue(shortName); // 7 == strlen("driver.")
      else
        if (d->mapTempCameraSettings.contains(shortName))
          return d->mapTempCameraSettings.value(shortName);
        else
          return QVariant();
    }
  else
    return property(name);
}

bool PiiMultiCamDriver::selectUnit(const QString& unit)
{
  PII_D;
  QFileInfo fileInfo(unit);
  
  if (false == fileInfo.exists())
    return false;

  // The multicam-specific (= tol-level) settings are set to the
  // properties of this class. The camera specific settings (in camera
  // group) are stored in d->mapTempCameraSettings. Where they are read,
  // when intialize()-function is called.
  
  QSettings settings(unit, QSettings::IniFormat);
  QStringList keys = settings.childKeys();
  foreach (QString key, keys)
    if (key[0] != '#') // ignore comment lines
      setProperty(key.toAscii(), settings.value(key));

  // Read camera-specific keys
  if (settings.childGroups().contains("camera"))
    {
      settings.beginGroup("camera");
      keys = settings.childKeys();
      foreach(QString key, keys)
        {
          //qDebug("Setting %s to %s", qPrintable(key), qPrintable(settings.value(key).toString()));
          if (key[0] != '#')
            d->mapTempCameraSettings[key] = settings.value(key);
        }
      settings.endGroup();
    }
  
  return true;
}

void PiiMultiCamDriver::surfaceFilled()
{
  PII_D;
  d->surfaceMutex.lock();
  // Get the index of the buffer last filled by the driver
  McGetParamInt(d->hChannel, MC_SurfaceIndex, &d->iLastCapturedBuffer);
  // Wake up a possibly waiting capture thread
  d->captureCondition.wakeOne();
  d->surfaceMutex.unlock();
}

void PiiMultiCamDriver::setTopology()
{
  PII_D;
  MCSTATUS status;
  int iTopology;
  // Not all boards support multiple camera topologies. We now only
  // check for Expert 2.
  switch (d->iBoardType)
    {
    case MC_BoardType_EXPERT_2:
    case MC_BoardType_EXPERT_2_CPCI:
      // If channel is "M", there's one medium mode camera.
      if (d->strChannel == "M")
        iTopology = MC_BoardTopology_MONO;
      // In other cases, there must be a base mode camera in either
      // (or both) connectors.
      else
        iTopology = MC_BoardTopology_DUO;
      status = McSetParamInt(MC_BOARD + d->iBoard, MC_BoardTopology, iTopology);
      if (status != MC_OK)
        MULTICAM_THROW(tr("Cannot set board topology."), status);
      break;
    }
}

void PiiMultiCamDriver::createChannel()
{
  PII_D;
  int iChannelType = MC_CHANNEL;
  int iConnector = MC_Connector_A;

  // Channel type depends on active board and selected channel
  switch (d->iBoardType)
    {
    case MC_BoardType_EXPERT_2:
    case MC_BoardType_EXPERT_2_CPCI:
      if (d->strChannel == "M")
        {
          iChannelType = MC_CHANNEL_EXPERT_M;
          iConnector = MC_Connector_M;
        }
      else if (d->strChannel == "A")
        {
          iChannelType = MC_CHANNEL_EXPERT_A;
          iConnector = MC_Connector_A;
        }
      else if (d->strChannel == "B")
        {
          iChannelType = MC_CHANNEL_EXPERT_B;
          iConnector = MC_Connector_B;
        }
      break;
    }

  // Create the acquisition channel
  MCSTATUS status = McCreate(iChannelType, &d->hChannel);
  if (status != MC_OK)
    MULTICAM_THROW(tr("Cannot initialize acquisition channel %1.").arg(d->strChannel), status);
  
  // Associate a board with the channel 
  status = McSetParamInt(d->hChannel, MC_DriverIndex, d->iBoard);
  if (status != MC_OK)
    MULTICAM_THROW(tr("Cannot associate board %1 with channel.").arg(d->iBoard), status);

  // Associate a connector with the channel 
  status = McSetParamInt(d->hChannel, MC_Connector, iConnector);
  if (status != MC_OK)
    MULTICAM_THROW(tr("Cannot associate connector with channel."), status);

  // Set acquisition mode
  status = McSetParamInt(d->hChannel, MC_AcquisitionMode, MC_AcquisitionMode_WEB);
  if (status != MC_OK)
    MULTICAM_THROW(tr("Cannot set acquisition mode."), status);
}

void PiiMultiCamDriver::setTapConfiguration()
{
  PII_D;
  int iTapGeometry = MC_TapGeometry_1X2;
  
  if (d->cameraLinkMode <= Base1T24)
    iTapGeometry = MC_TapGeometry_1X;
  
  //PENDING different camera models need different geometries
  
  MCSTATUS status = McSetParamInt(d->hChannel, MC_TapConfiguration, static_cast<int>(d->cameraLinkMode));
  if (status != MC_OK)
    MULTICAM_THROW(tr("Cannot change tap configuration."), status);

  status = McSetParamInt(d->hChannel, MC_TapGeometry, iTapGeometry);
  if (status != MC_OK)
    MULTICAM_THROW(tr("Cannot set tap geometry."), status);
}


/* This function stores the configuration value 'setChannelCount' of
 * the camera temporarily in d->mapTempCameraSettings. The value is
 * calculated from member variable d->cameraLinkMode.
 */
  void PiiMultiCamDriver::storeChannelCountForCamera()
{
  PII_D;
  // The rules for setting the configuration value "channelCount" in
  // PiiAviivaM2CLSerialProtocol based  on property "CameraLinkMode"
  // of PiiMultiCamDriver.
  // ------------------------------------------------------
  // | Multicam (CameraLinkMode) | Camera (channel count) |
  // |=====================================================
  // |       Base1TXX            |        1               |
  // |-----------------------------------------------------
  // |       Base2TXX            |        2               |
  // |-----------------------------------------------------
  // |       Base3TXX,           |                        |
  // |       Base4TXX,           |     - (undefined)      |
  // |       MediumXXXX          |                        |
  // |       Full8T8             |                        |
  // |-----------------------------------------------------

  // REMOVE >>
//   if (d->pSerialProtocol == 0)
//     return;
// REMOVE <<
  
  if ( (d->cameraLinkMode >= Base1T8 && d->cameraLinkMode <= Base1T24) ||
       (d->cameraLinkMode >= Base1T30B2 && d->cameraLinkMode <= Base1T48B2) ||
       (d->cameraLinkMode >= Base1T24B3 && d->cameraLinkMode <= Medium1T48) )
    d->mapTempCameraSettings["channelCount"] = 1;
  else if ( (d->cameraLinkMode >= Base2T8 && d->cameraLinkMode <= Base2T12) ||
            (d->cameraLinkMode >= Base2T14B2 && d->cameraLinkMode <= Base2T24B2) ||
            (d->cameraLinkMode >= Medium2T14 && d->cameraLinkMode <= Medium2T24) )
    d->mapTempCameraSettings["channelCount"] = 2;
  else if ( d->cameraLinkMode == Base3T8 ||
            (d->cameraLinkMode >= Base3T10B2 && d->cameraLinkMode <= Base3T16B2) ||
            (d->cameraLinkMode >= Medium3T10 && d->cameraLinkMode <= Medium3T16) )
    d->mapTempCameraSettings["channelCount"] = 3;
  else if ( (d->cameraLinkMode >= Base4T8B2 && d->cameraLinkMode <= Base4T12B2) ||
            (d->cameraLinkMode >= Medium4T8 && d->cameraLinkMode <= Medium4T12) )
    d->mapTempCameraSettings["channelCount"] = 4;
  else if (d->cameraLinkMode == Medium8T8B2 || d->cameraLinkMode == Medium8T8B2)
    d->mapTempCameraSettings["channelCount"] = 8;
//   else
//     {
//       PII_THROW(PiiCameraDriverException, tr("Undefined camera link mode value."));
//     }
}

/* This function stores the configuration value 'bitPerPixel' of the
 * camera temporarily in d->mapTempCameraSettings. The value is
 * calculated from member variable d->cameraLinkMode.
 */
void PiiMultiCamDriver::storeBitsPerPixelForCamera()
{
  PII_D;
  // The rules for setting the configuration value "bitsPerPixel" through
  // PiiAviivaM2CLSerialProtocol based on the property "CameraLinkMode" 
  // of PiiMultiCamDriver.
  // ------------------------------------------------------
  // | Multicam (CameraLinkMode) | Camera (bitsPerPixel)  |
  // |=====================================================
  // |       Base1T8,            |        8               |
  // |       Base2T8             |                        |
  // |-----------------------------------------------------
  // |       Base1T10,           |        10              |
  // |       Base2T10            |                        |
  // |-----------------------------------------------------
  // |       Base1T12,           |        12              |
  // |       Base2T12            |                        |
  // |-----------------------------------------------------
  // |       Other values        |    - (undefined)       | 
  // |-----------------------------------------------------

  // REMOVE >>
//   if (d->pSerialProtocol == 0)
//     return;
// REMOVE <<
  
  switch(d->cameraLinkMode)
    {
    case Base1T8:
    case Base2T8:
    case Base3T8:
    case Base4T8B2:
    case Medium4T8:
    case Medium8T8B2:
    case Full8T8:
      d->mapTempCameraSettings["bitsPerPixel"] = 8;
      break;
    case Base1T10:
    case Base2T10:
    case Base3T10B2:
    case Base4T10B2:
    case Medium3T10:
    case Medium4T10:
      d->mapTempCameraSettings["bitsPerPixel"] = 10;
      break;      
    case Base1T12:
    case Base2T12:
    case Base4T12B2:
    case Base3T12B2:
    case Medium3T12:
    case Medium4T12:
      d->mapTempCameraSettings["bitsPerPixel"] = 12;
      break;      
    case Base1T14:
    case Base2T14B2:
    case Base3T14B2:
    case Medium2T14:
    case Medium3T14:
      d->mapTempCameraSettings["bitsPerPixel"] = 14;
      break;
    case Base1T16:
    case Base2T16B2:
    case Base3T16B2:
    case Medium2T16:
    case Medium3T16:
      d->mapTempCameraSettings["bitsPerPixel"] = 16;
      break;
    case Base1T24:
    case Base2T24B2:
    case Base1T24B3:
    case Medium2T24:
      d->mapTempCameraSettings["bitsPerPixel"] = 24;
      break;
    case Base1T30B2:
    case Base1T30B3:
    case Medium1T30:
      d->mapTempCameraSettings["bitsPerPixel"] = 30;
      break;
    case Base1T36B2:
    case Base1T36B3:
    case Medium1T36:
      d->mapTempCameraSettings["bitsPerPixel"] = 36;
      break;
    case Base1T42B2:
    case Base1T42B3:
    case Medium1T42:
      d->mapTempCameraSettings["bitsPerPixel"] = 42;
      break;
    case Base1T48B2:
    case Base1T48B3:
    case Medium1T48:
      d->mapTempCameraSettings["bitsPerPixel"] = 48;
      break;
//     default:
//       PII_THROW(PiiCameraDriverException, tr("Undefined camera link mode value."));
  };
}

/* This function stores the configuration value 'triggerMode' of
 * the camera temporarily in d->mapTempCameraSettings. The value is
 * calculated from member variable d->strCameraConfig.
 */
void PiiMultiCamDriver::storeTriggerModeForCamera()
{
  PII_D;
  // Rules for setting the configuration value "triggerMode" of serial
  // protocol (camera) based on [CamMode][ExpMode] parts combination
  // of the property "cameraConfig" of the class PiiMultiCamDriver.

  // --------------------------------------------------------------------
  // | [CamMode] | [ExpMode] |   Camera  (trigger mode)                 |
  // |==================================================================|
  // |     S     |     C     |   FreeRun ( == 1)                        |
  // |-----------|-----------|------------------------------------------|
  // |     S     |     G     |   - (no correspondent option)            |
  // |-----------|-----------|------------------------------------------|
  // |     S     |     P     |   FreeRun ( == 1)                        |
  // |-----------|----- -----|------------------------------------------|
  // |     R     |     C     |   ExternalTrigger ( = 2)                 |
  // |-----------|-----------|------------------------------------------|
  // |           |           | ExternalTriggerAndIntegration (= 3)      |
  // |           |           | (or ExternalTriggerAndIntegration2, = 4) |
  // |     R     |     G     | The first one is always set.             |
  // |           |           | THIS MUST BE REVIEWER LATER!!            |
  // |-----------|-----------|------------------------------------------|
  // |     R     |     P     |  ExternalTrigger ( = 2)                  |
  // --------------------------------------------------------------------
  //
  // For camera mode:
  //
  //  S = line rate synchronously generated inside the camera (free
  //      run)
  //  R = line rate externally controller through a line-reset pulse
  //      (triggered)
  // For exposure:
  //
  //  C = controlled by camera
  //  G = controlled by frame grabber
  //  P = permanent exposure

  QString mode;
  QRegExp rxgain(QString("[LPI]\\d+([RS][CGP])"));
  int pos = rxgain.indexIn(d->strCameraConfig);
  if (pos > -1)
   {
     mode = rxgain.cap(1);
   }

  //qDebug() << "PiiMultiCamDriver::setTriggerModeForCamera() d->strCameraConfig = " << d->strCameraConfig;
  //qDebug() << "PiiMultiCamDriver::setTriggerModeForCamera() mode = " << mode;
  
  if (mode == "SC")
    d->mapTempCameraSettings["triggerMode"] = 1; // FreeRun
  else if (mode == "SP")
    d->mapTempCameraSettings["triggerMode"] = 1; // FreeRun
  else if (mode == "RC")
    d->mapTempCameraSettings["triggerMode"] = 2; // ExternalTrigger
  else if (mode == "RG")
    d->mapTempCameraSettings["triggerMode"] = 3; // ExternalTriggerAndIntegration
  else if (mode == "RP")
    d->mapTempCameraSettings["triggerMode"] = 2; // ExternalTrigger
  
}

/* This function gets the camera settings from the camera and stores
   them in the d->mapTempCameraSettings*/
void PiiMultiCamDriver::storeCameraSettingsInMap()
{
  PII_D;

  //if (d->pSerialProtocol && d->pSerialDevice) // REMOVE
  if (d->pSerialProtocol)
    {
      // REMOVE >>
      /* PENDING
       * Use here the functions getConfigurationValueCount() and
       * getMetaInformation() of class PiiConfigurable for finding out the
       * configuration values that must be stored in the map. Now the
       * implementation is PiiAviivaM2CL-specific.
       */
//       d->mapTempCameraSettings["gain"] = d->pSerialProtocol->getConfigurationValue("gain");
//       d->mapTempCameraSettings["busSpeed"] = d->pSerialProtocol->getConfigurationValue("busSpeed");
//       d->mapTempCameraSettings["channelCount"] = d->pSerialProtocol->getConfigurationValue("channelCount");
//       d->mapTempCameraSettings["bitsPerPixel"] = d->pSerialProtocol->getConfigurationValue("bitsPerPixel");
//       d->mapTempCameraSettings["integrationTime"] = d->pSerialProtocol->getConfigurationValue("integrationTime");
//       d->mapTempCameraSettings["triggerMode"] = d->pSerialProtocol->getConfigurationValue("triggerMode");
      // TODO: Uncomment the following lines and comment out the above
      // files, once the meta configuration value system has been implemented.
// REMOVE
      
      int valueCount = d->pSerialProtocol->getConfigurationValueCount();
      for (int i = 0; i < valueCount; i++)
        {
          PiiMetaConfigurationValue* metaInfo = d->pSerialProtocol->getMetaInformation(i);
          if (metaInfo)
            d->mapTempCameraSettings[metaInfo->getName()] = d->pSerialProtocol->getConfigurationValue(metaInfo->getName());
        }
    }
}

/* This function creates the serial protocol class based on the
 * protocol name given as a parameter. The class name is formed from
 * the protocol name by adding the prefic "Pii" and the suffix
 * "SerialProtocol" to the protocol name. E.g. if the protocol name is
 * AviivaM2CL, the corresponding class will be PiiAviivaM2CL.
 * Note that the protocol class will be created immediately, afeter the
 * corresponding property is set. This is different from the other
 * properties in this class, where the action corresponding to the
 * property is performed when the function intialize() is called first
 * time after setting a property. Because protocol is not dependent
 * directly of any hardware, it is safe to creat the protocol class as
 * soon, as the protocol name is known.
 */
void PiiMultiCamDriver::setSerialProtocol(const QString& serialProtocol)
{
  PII_D;
  //AviivaM2CL >> PiiAviivaM2CLSerialProtocol
  d->strSerialProtocol = serialProtocol;
}

void PiiMultiCamDriver::initSerialProtocol()
{
  PII_D;
  QString className;

  className = QString("Pii%1SerialProtocol").arg(d->strSerialProtocol);
  
  // TODO: Create instance of some default serial protocol class (e.g.
  // PiiDefaultSerialProtocol), if creating the class fails here.
  PiiCameraConfigurationProtocol* protocolClass = qobject_cast<PiiCameraConfigurationProtocol*>(PiiEngine::getRegistry()->create(className));
  if (protocolClass != 0)
    {
      delete d->pSerialProtocol;
      d->pSerialProtocol = protocolClass;
      createSerialDevice();
    }
  else
    PII_THROW(PiiCameraDriverException, tr("Camera configration protocol not available."));
}

void PiiMultiCamDriver::setCameraConfig()
{
  PII_D;
  // Associate camera to the channel
  MCSTATUS status = McSetParamStr(d->hChannel, MC_Camera, qPrintable(d->strCameraModel));
  if (status != MC_OK)
    MULTICAM_THROW(tr("Cannot associate camera with channel."), status);

  // Set camera config parameters
  status = McSetParamStr(d->hChannel, MC_CamConfig, qPrintable(d->strCameraConfig));
  if (status != MC_OK)
    MULTICAM_THROW(tr("Cannot set camera configuration parameters."), status);
}

/* This function sets the trigger source for MultiCamDriver. It doesn't
 * change the trigger mode of the camera. There is a separate function
 * for that.
 */
void PiiMultiCamDriver::setTriggerMode()
{
  PII_D;
  MCSTATUS status = McSetParamInt(d->hChannel, MC_LineCaptureMode, MC_LineCaptureMode_ALL);
  if (status != MC_OK)
    MULTICAM_THROW(tr("Cannot set line capture mode."), status);
  
  int mode = MC_LineRateMode_CAMERA;
  switch (d->triggerSource)
    {
    case NoTrigger:
      mode = MC_LineRateMode_CAMERA;
      break;
    case InternalClockTrigger:
      mode = MC_LineRateMode_PERIOD;
      break;
    case ExternalTrigger:
      mode = MC_LineRateMode_PULSE;
      break;
    case EncoderTrigger:
      mode = MC_LineRateMode_CONVERT;
      break;
    }
  
  status = McSetParamInt(d->hChannel, MC_LineRateMode, mode);
  if (status != MC_OK)
    MULTICAM_THROW(tr("Cannot set line rate mode."), status);

 
  // Set up clock frequency
  if (d->triggerSource == InternalClockTrigger)
    {
      int period = static_cast<int>((1.0 / d->dTriggerRate) * 1000000); // microseconds
      if (period < 1) period = 1;
      else if (period > 1000000) period = 1000000;
      status = McSetParamInt(d->hChannel, MC_Period_us, period);
      if (status != MC_OK)
        MULTICAM_THROW(tr("Cannot set internal clock period."), status);
    }
  // Set up the encoder conversion ratio
  else if (d->triggerSource == EncoderTrigger)
    {
      initTriggerRate();
    }

  if ( d->triggerSource == EncoderTrigger || d->triggerSource == ExternalTrigger )
    {
      status = McSetParamInt(d->hChannel, MC_ConverterTrim, 0);
      if (status != MC_OK)
        MULTICAM_THROW(tr("Cannot set converter trim."), status);

      status = McSetParamInt(d->hChannel, MC_MaxSpeed, 100000);
      if (status != MC_OK)
        MULTICAM_THROW(tr("Cannot set max speed."), status);

      status = McSetParamInt(d->hChannel, MC_AcqTimeout_ms, MC_INFINITE);
      if (status != MC_OK)
        MULTICAM_THROW(tr("Cannot set AcqTimeout."), status);

      status = McSetParamInt(d->hChannel, MC_OnMinSpeed, MC_OnMinSpeed_MUTING);
      if (status != MC_OK)
        MULTICAM_THROW(tr("Cannot set onminspeed."), status);

      status = McSetParamInt(d->hChannel, MC_LineTrigCtl, MC_LineTrigCtl_TTL);
      if (status != MC_OK)
        MULTICAM_THROW(tr("Cannot set linetrigcontrol."), status);

      status = McSetParamInt(d->hChannel, MC_LineTrigEdge, MC_LineTrigEdge_GOHIGH);
      if (status != MC_OK)
        MULTICAM_THROW(tr("Cannot set linetrigedge."), status);

      status = McSetParamInt(d->hChannel, MC_LineTrigFilter, MC_LineTrigFilter_STRONG);      
      if (status != MC_OK)
        MULTICAM_THROW(tr("Cannot set linetrigFilter."), status);

      status = McSetParamInt(d->hChannel, MC_LineTrigLine, MC_LineTrigLine_NOM);
      if (status != MC_OK)
        MULTICAM_THROW(tr("Cannot set linetrigline."), status);
    }
}

void PiiMultiCamDriver::initTriggerRate()
{
  PII_D;
  if ( d->bOpen )
    {
      PiiFraction<int> fraction = PiiFraction<int>::create(d->dTriggerRate);
      if (fraction.numerator <= 0 || fraction.numerator > 10000 ||
          fraction.denominator <= 0 || fraction.denominator > 10000)
        {
          qWarning() << tr("Invalid encoder conversion ration supplied.");
          PII_THROW(PiiCameraDriverException, tr("Invalid encoder conversion ration supplied."));
        }

      MCSTATUS status = McSetParamInt(d->hChannel, MC_EncoderPitch, fraction.numerator);
      if (status != MC_OK)
        MULTICAM_THROW(tr("Cannot set encoder pitch."), status); 

      status = McSetParamInt(d->hChannel, MC_LinePitch, fraction.denominator);
      if (status != MC_OK)
        MULTICAM_THROW(tr("Cannot set line pitch."), status);
    }
}

void PiiMultiCamDriver::initialize()
{
  PII_D;
  //qDebug("PiiMultiCamDriver::initialize() BEGIN");
  if (d->bOpen)
    PII_THROW(PiiCameraDriverException, tr("MultiCam driver is already open. Close the driver first."));
    
  MCSTATUS status = McOpenDriver(0);
  if (status != MC_OK)
    MULTICAM_THROW(tr("Cannot open MultiCam driver"), status);

  d->bOpen = true;
  status = McGetParamInt(MC_BOARD + d->iBoard, MC_BoardType, &d->iBoardType);
  if (status != MC_OK)
    MULTICAM_THROW(tr("Cannot read the type of boards %1.").arg(d->iBoard), status);

  // REMOVE ><
  // Create the serial device, which is used for writing the serial
  // commands to the camera. If the serial protocol doesn't exist, the
  // serial device is not created.
  // REMOVE <<
  // Creates the serial protocol and serial device classes and sets
  // the serial device for the serial protocol.
  initSerialProtocol();
  // REMOVE >>
//   if (d->pSerialProtocol)
//     createSerialDevice();
  // REMOVE <<
  // Configure board topology based on board type and selected
  // channel.
  setTopology();
  // Create a channel and bind it to the board and connector.
  createChannel();
  // Set camera configuration parameters for multicam
  setCameraConfig();
  // Initialize tap configuration parameters
  setTapConfiguration();
  // Set trigger parameters
  setTriggerMode();

  //qDebug() << "PiiMultiCamDriver::initialize(), d->mapTempCameraSettings = " << d->mapTempCameraSettings;
  // The camera settings.
  if (d->pSerialProtocol)
    {
      // Go through all the configuration values in the camera.
      // It is checked from the metaconfiguration value, if the
      // configuration value is writable. If it is, the corresponding
      // value is read from d->mapTempCameraSettings and is written to
      // the camera.
      int valueCount = d->pSerialProtocol->getConfigurationValueCount();
      for (int i = 0; i < valueCount; i++)
        {
          PiiMetaConfigurationValue* metaInfo = d->pSerialProtocol->getMetaInformation(i);
          if (metaInfo)
            {
              const char* key = metaInfo->getName();
              if (metaInfo->isWritable() && d->mapTempCameraSettings.contains(key))
                if (false == d->pSerialProtocol->setConfigurationValue(key, d->mapTempCameraSettings.value(key)))
                  PII_THROW(PiiCameraDriverException, tr("Cannot set the property '%1' with the value %2 to the camera").arg(key).arg(d->mapTempCameraSettings[key].toString()));
            }
        }

      // REMOVE >>
//       QStringList keys = d->mapTempCameraSettings.keys();
//       foreach(QString key, keys)
//         {
//           qDebug() << key;
//           if (false == d->pSerialProtocol->setConfigurationValue(key.toAscii(), d->mapTempCameraSettings[key]))
//             PII_THROW(PiiCameraDriverException, tr("Cannot set the property '%1' with the value %2 to the camera").arg(key).arg(d->mapTempCameraSettings[key].toString()));
//         }
      // REMOVE <<
    }

  // Because configuration values can now be read from the camera
  // directly, the map for storing the camera settings temporarily,
  // can be cleared.
  d->mapTempCameraSettings.clear();
  
  // Register callback function
  status = McRegisterCallback(d->hChannel, multiCamCaptureFunction, this);
  if (status != MC_OK)
    MULTICAM_THROW(tr("Cannot register callback function."), status);

  // Enable callback for the SURFACE_PROCESSING event
  status = McSetParamInt(d->hChannel, MC_SignalEnable + MC_SIG_SURFACE_PROCESSING, MC_SignalEnable_ON);
  if (status != MC_OK)
    MULTICAM_THROW(tr("Cannot enable callback event."), status);

  // Enable callback for the ACQUISITION_FAILURE event
  status = McSetParamInt(d->hChannel, MC_SignalEnable + MC_SIG_ACQUISITION_FAILURE, MC_SignalEnable_ON);
  if (status != MC_OK)
    MULTICAM_THROW(tr("Cannot enable callback event."), status);

  /* The way MultiCam handles the frame buffer is somewhat peculiar. 
   * The buffer is composed of "surfaces" that are grouped into a
   * "cluster". Initially, all surfaces are "free". When capture
   * begins, the grabber will start "filling" the first free surface. 
   * When it is done, the surface is said to be "filled". If the
   * SURFACE_PROCESSING callback is enabled, the surface will turn
   * into "processing" mode while the callback executes. Once the
   * callback is done, the surface turns back to "filled".
   *
   * If there are no "free" surfaces available, the grabber will start
   * filling the oldest "filled" one.
   *
   * The maximum number of surfaces per cluster is 4096. It seems that
   * surface addresses are assigned directly to hardware, which has
   * only limited memory for them.
   */
  
  // Retrieve image parameters
  
  // MC_PageLength_Ln is the d->only_ way to affect signal
  // interval with line-scan cameras.
  if (d->strCameraConfig.size() > 0 && d->strCameraConfig[0] == 'L')
    {
      status = McSetParamInt(d->hChannel, MC_PageLength_Ln, d->iSignalInterval);
      if (status != MC_OK)
        MULTICAM_THROW(tr("Cannot set signal interval."), status);
      d->iFrameHeight = 1;
    }
  else
    {
      status = McGetParamInt(d->hChannel, MC_ImageSizeY, &d->iFrameHeight);
      if (status != MC_OK)
        MULTICAM_THROW(tr("Cannot read image height."), status);
    }


  status = McGetParamInt(d->hChannel, MC_ImageSizeX, &d->iBufferWidth);
  if (status != MC_OK)
    MULTICAM_THROW(tr("Cannot read image width."), status);

  //qDebug("PiiMultiCamDriver::initialize(), d->iFrameHeight = %d", d->iFrameHeight);
  //qDebug("PiiMultiCamDriver::initialize(), d->iBufferWidth = %d", d->iBufferWidth);
  
  // Check the color format
  int iColorFormat;
  status = McGetParamInt(d->hChannel, MC_ColorFormat, &iColorFormat);
  if (status != MC_OK)
    MULTICAM_THROW(tr("Cannot read color format."), status);

  // PENDING other color formats
  if (iColorFormat != MC_ColorFormat_Y8)
    MULTICAM_THROW(tr("Unsupported color format."), status);

  // Get image pitch (distance in bytes between successive pixels)
  int iImagePitch;
  status = McGetParamInt(d->hChannel, MC_BufferPitch, &iImagePitch);
  if (status != MC_OK) 
    MULTICAM_THROW(tr("Cannot read image pitch."), status);
  
  // Create an image buffer
  // Get image buffer size
  int iSurfaceSize;
  
  status = McGetParamInt(d->hChannel, MC_BufferSize, &iSurfaceSize);
  if (status != MC_OK) 
    MULTICAM_THROW(tr("Cannot read image buffer size."), status);
  
  long totalSize = qMax(iSurfaceSize, d->iBufferMemory);

  // Pointer manipulation needed to align the MC_SurfaceAddr pointer
  // to pages of memory (DMA requirement)
  d->pAllocatedBuffer = malloc(totalSize + 0xf);
  if (d->pAllocatedBuffer == 0)
    MULTICAM_THROW(tr("Cannot allocate %i bytes of memory for frame buffer.").arg(totalSize), status);
  
  long lAligned = reinterpret_cast<long>(d->pAllocatedBuffer);
  lAligned += ((-lAligned) & 0xf);
  
  d->pFrameBuffer = reinterpret_cast<unsigned char*>(lAligned);

  // Now we have an aligned pointer to the beginning of the frame
  // buffer. Loop over the whole buffer and create surfaces so that
  // each stores d->iSignalInterval frames.

  int iMemoryUsed = 0;
  d->iBufferHeight = 0;

  while (iMemoryUsed + iSurfaceSize <= totalSize &&
         d->lstSurfaces.size() < 4096) // Magic number: the board can only store this many surface addresses
    {
      createSurface(d->pFrameBuffer + iMemoryUsed, iSurfaceSize, iImagePitch);
      iMemoryUsed += iSurfaceSize;
      d->iBufferHeight += d->iFrameHeight * d->iSignalInterval;
    }

  // Set Channel to Idle state
  status = McSetParamInt(d->hChannel, MC_ChannelState, MC_ChannelState_IDLE);
  if (status != MC_OK)
    {
      free(d->pAllocatedBuffer);
      d->pAllocatedBuffer = 0;
      d->pFrameBuffer = 0;
      MULTICAM_THROW(tr("Cannot change channel state to IDLE."), status);
    }
  //qDebug("PiiMultiCamDriver::initialize() END");
}

void PiiMultiCamDriver::createSurface(void* address, int size, int pitch)
{
  PII_D;
  // Create a surface
  MCHANDLE hSurface;
  MCSTATUS status = McCreate(MC_DEFAULT_SURFACE_HANDLE, &hSurface);
  if (status != MC_OK) 
    MULTICAM_THROW(tr("Cannot create surface object."), status);

  // Exception-safety
  AutoHandle handle(hSurface);
  
  // Set surface parameters
  status = McSetParamInt(hSurface, MC_SurfaceSize, size);
  if (status != MC_OK) 
    MULTICAM_THROW(tr("Cannot set surface size."), status);

  status = McSetParamInt(hSurface, MC_SurfacePitch, pitch);
  if (status != MC_OK)
    MULTICAM_THROW(tr("Cannot assign surface pitch."), status);

  status = McSetParamInt(hSurface, MC_SurfaceAddr, static_cast<int>(reinterpret_cast<long>(address)));
  if (status != MC_OK) 
    MULTICAM_THROW(tr("Cannot set surface address."), status);

  // Surface Context Information
  status = McSetParamInt(hSurface, MC_SurfaceContext, static_cast<int>(reinterpret_cast<long>(address)));
  if (status != MC_OK) 
    MULTICAM_THROW(tr("Cannot set surface context information."), status);

  // Associate surface with channel
  status = McSetParamInst(d->hChannel, MC_Cluster + d->lstSurfaces.size(), hSurface);
  if (status != MC_OK) 
    MULTICAM_THROW(tr("Cannot associate surface with channel."), status);

  d->lstSurfaces << handle.release();
}

/* A private helper function, which creates the serial device. Throws
 * exception if either creating the device or connecting to the device
 * fails. This function also sets the serial device for the serial
 * protocol member (d->pSerialProtocol). The serial protocol must
 * exists, before this function is callled.
 */
void PiiMultiCamDriver::createSerialDevice()
{
  PII_D;
  bool success = false;
  // Create the device, which will be used for reading and writing the
  // camera commands using a serial protocol. We create a waiting
  // device with timeout of 10 milliseconds. We need the timeout,
  // because sometimes there is a small delay until the response is
  // received from the camera.
  d->pSerialDevice = new PiiWaitingIODevice<PiiCameraLinkSerialDevice>(10);
  static_cast<PiiWaitingIODevice<PiiCameraLinkSerialDevice>* >(d->pSerialDevice)->setTryToReadEverything(true);
  if (d->pSerialDevice == 0)
    PII_THROW(PiiCameraDriverException, tr("Creating the device failed."));

  // channel A & M => port 0
  // channel B => port 1
  d->pSerialDevice->setPortIndex(d->strChannel == "B" ? 1 : 0);
  d->pSerialDevice->setBaudRate(9600);

  success = d->pSerialDevice->open(QIODevice::ReadWrite);
  if (success == false)
    {
      delete d->pSerialDevice;
      d->pSerialDevice = 0;
      PII_THROW(PiiCameraDriverException, tr("Connecting to the serial device failed."));
    }
  d->pSerialProtocol->setDevice(d->pSerialDevice);
}

bool PiiMultiCamDriver::startCapture(int /*frames*/)
{
  PII_D;
  // Start at the beginning of the surface sequence
  McSetParamInt(d->hChannel, MC_SurfaceIndex, 0);
  d->iLastCapturedBuffer = 0;
  
  // Run the capture thread
  d->captureThread.startThread(QThread::TimeCriticalPriority);

  //Activate acquisition sequence
  MCSTATUS status = McSetParamInt(d->hChannel, MC_ChannelState, MC_ChannelState_ACTIVE);
  return status == MC_OK;
}

bool PiiMultiCamDriver::stopCapture()
{
  PII_D;
  // Shut down the capture thread
  d->captureThread.stop();
  d->captureThread.wait();

  MCSTATUS status = McSetParamInt(d->hChannel, MC_ChannelState, MC_ChannelState_IDLE);
  return status == MC_OK;
}

void* PiiMultiCamDriver::getFrameBuffer(int frameIndex) const
{
  const PII_D;
  if (frameIndex >= d->iBufferHeight)
    frameIndex %= d->iBufferHeight;
  else
    while (frameIndex < 0) frameIndex += d->iBufferHeight;

  return d->pFrameBuffer + frameIndex * d->iBufferWidth;
}

void PiiMultiCamDriver::getFrameBuffers(int firstFrameIndex, int cnt, void** scanLines) const
{
  const PII_D;
  // Bypass virtual function resolution for slightly better speed
  scanLines[0] = PiiMultiCamDriver::getFrameBuffer(firstFrameIndex);
  for (int r=1; r<cnt; r++)
    // Cast to char* to perform pointer arithmetic
    scanLines[r] = static_cast<char*>(scanLines[r-1]) + d->iBufferWidth;
}
  
QSize PiiMultiCamDriver::getBufferSize() const
{
  const PII_D;
  return QSize(d->iBufferWidth, d->iBufferHeight);
}

QSize PiiMultiCamDriver::getFrameSize() const
{
  const PII_D;
  return QSize(d->iBufferWidth, d->iFrameHeight);
}

bool PiiMultiCamDriver::close()
{
  PII_D;
  if ( d->bOpen )
    {
      // Just to be sure... Someone may call close() while we are
      // still capturing.
      stopCapture();
      
      McDelete(d->hChannel);
      d->bOpen = false;
      for (int i=d->lstSurfaces.size(); i--; )
        McDelete(d->lstSurfaces[i]);
      d->lstSurfaces.clear();
      
      free(d->pAllocatedBuffer);
      d->pAllocatedBuffer = 0;
      d->pFrameBuffer = 0;
      
      // The map for storing the camera settings temporarily must be
      // filled, before connection to the serial device is closed.
      storeCameraSettingsInMap();
      // TODO: Add warning, if closing the serial device fails. Currently
      // it is not possible, because close() function of
      // PiiCameraLinkSerialDevice doesn't return anything. 
      if ( d->pSerialDevice )
        d->pSerialDevice->close();
      delete d->pSerialProtocol;
      d->pSerialProtocol = 0;
      delete d->pSerialDevice;
      d->pSerialDevice = 0;
      
      MCSTATUS status = McCloseDriver();
      if (status != MC_OK)
        {
          qWarning("PiiMultiCamDriver::close() failed with error code %i.", status);
          return false;
        }
      return true;
    }
  return false;
}

bool PiiMultiCamDriver::isOpen() const
{
  const PII_D;
  return d->bOpen;
}

bool PiiMultiCamDriver::isCapturing() const
{
  const PII_D;
  int state;
  MCSTATUS status = McGetParamInt(d->hChannel, MC_ChannelState, &state);
  return status == MC_OK && state == MC_ChannelState_ACTIVE;
}

void PiiMultiCamDriver::setCameraLinkMode(CameraLinkMode cameraLinkMode)
{
  PII_D;
  // Store the value of cameraLinkMode temporay. It will be set for
  // MultiCamDriver, when initialize() function is called later.
  d->cameraLinkMode = cameraLinkMode;
  // The following function stores configuration values 'channelCount'
  // and 'bitsPerPixel' of the camera temporarily in the member
  // variable d->mapTempCameraSettings. They will be set for
  // camera, when initialize() function is called later.
  storeChannelCountForCamera();
  storeBitsPerPixelForCamera();
}

void PiiMultiCamDriver::setCameraConfig(const QString& cameraConfig)
{
  PII_D;
  // Store camera config value temporay. It will be set for
  // MultiCamDriver, when initialize() function is called later.
  d->strCameraConfig = cameraConfig;
  // The following function stores triggerMode of the camera temporay
  // in the member variable d->mapTempCameraSettings. It will be set for
  // camera, when initialize() function is called later.
  storeTriggerModeForCamera();
}

//************** Signal handler thread ***************

PiiMultiCamCaptureThread::PiiMultiCamCaptureThread(PiiMultiCamDriver* parent) :
  d->pParent(parent), d->bRunning(false), d->iSignalCounter(0)
{
}

void PiiMultiCamCaptureThread::stop()
{
  PII_D;
  // Make sure that the thread is not left waiting.
  d->pParent->d->surfaceMutex.lock();
  d->bRunning = false;
  d->pParent->d->captureCondition.wakeOne();
  d->pParent->d->surfaceMutex.unlock();
}

void PiiMultiCamCaptureThread::run()
{
  PII_D;
  d->iSignalCounter = 0;
  // Maximum number of pages the capture can lag behind before
  // skipping frames.
  int iLagLimit = d->pParent->d->lstSurfaces.size() / 4;

  while (d->bRunning)
    {
      d->pParent->d->surfaceMutex.lock();
      // If we have handled all captured frames, wait for new data.
      if (d->iSignalCounter == d->pParent->d->iLastCapturedBuffer)
        {
          if (!d->bRunning)
            {
              d->pParent->d->surfaceMutex.unlock();
              break;
            }
          d->pParent->d->captureCondition.wait(&d->pParent->d->surfaceMutex);
          // This prevents duplicate signals to listener if the image
          // data stops coming before stop() is called.
          if (!d->bRunning)
            {
              d->pParent->d->surfaceMutex.unlock();
              break;
            }
        }
      // If buffer index is smaller than signal counter, the driver has
      // started filling buffers from the beginning again. We need to roll
      // around.
      int surfaceIndex = d->pParent->d->iLastCapturedBuffer;
      if (surfaceIndex < d->iSignalCounter)
        surfaceIndex += d->pParent->d->lstSurfaces.size();

      d->pParent->d->surfaceMutex.unlock();

      // Maximum lag exceeded
      if (surfaceIndex - d->iSignalCounter > iLagLimit)
        {
          qWarning("We are losing frames! Diff: %d, counter: %d captured: %d, buffers: %d",
                   surfaceIndex-d->iSignalCounter, d->iSignalCounter, surfaceIndex, d->pParent->d->lstSurfaces.size());
          // Skip frames up to the current capture position
          if (d->pParent->listener())
            d->pParent->listener()->framesMissed(d->iSignalCounter * d->pParent->d->iSignalInterval,
                                               surfaceIndex * d->pParent->d->iSignalInterval - 1);
          d->iSignalCounter = surfaceIndex;
        }
      else
        {
          // Increase handled surface counter
          ++d->iSignalCounter;
          // Image processing will be performed here
          if (d->pParent->listener())
            d->pParent->listener()->frameCaptured(d->iSignalCounter * d->pParent->d->iSignalInterval - 1);
        }
      d->iSignalCounter %= d->pParent->d->lstSurfaces.size();
    }
}

void PiiMultiCamDriver::setSignalInterval(int signalInterval) { _d()->iSignalInterval = signalInterval; }
int PiiMultiCamDriver::signalInterval() const { return _d()->iSignalInterval; }
void PiiMultiCamDriver::setBoard(int board) { _d()->iBoard = board; }
int PiiMultiCamDriver::board() const { return _d()->iBoard; }
PiiMultiCamDriver::CameraLinkMode PiiMultiCamDriver::cameraLinkMode() const { return _d()->cameraLinkMode; }
void PiiMultiCamDriver::setChannel(const QString& channel) { _d()->strChannel = channel; }
QString PiiMultiCamDriver::channel() const { return _d()->strChannel; }
void PiiMultiCamDriver::setCameraModel(const QString& cameraModel) { _d()->strCameraModel = cameraModel; }
QString PiiMultiCamDriver::cameraModel() const { return _d()->strCameraModel; }
QString PiiMultiCamDriver::serialProtocol() const { return _d()->strSerialProtocol;}
QString PiiMultiCamDriver::cameraConfig() const { return _d()->strCameraConfig; }
void PiiMultiCamDriver::setBufferMemory(int bufferMemory) { _d()->iBufferMemory = bufferMemory; }
int PiiMultiCamDriver::bufferMemory() const { return _d()->iBufferMemory; }
void PiiMultiCamDriver::setTriggerSource(TriggerSource triggerSource) { _d()->triggerSource = triggerSource; }
PiiMultiCamDriver::TriggerSource PiiMultiCamDriver::triggerSource() const { return _d()->triggerSource; }
void PiiMultiCamDriver::setTriggerRate(double triggerRat e) { _d()->dTriggerRate = triggerRate; initTriggerRate(); }
double PiiMultiCamDriver::triggerRate() const { return _d()->dTriggerRate; }
