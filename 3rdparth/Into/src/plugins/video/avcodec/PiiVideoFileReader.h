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

#ifndef _PIIVIDEOFILEREADER_H
#define _PIIVIDEOFILEREADER_H

#include "PiiVideoReader.h"

#include <PiiColor.h>
#include <PiiImageReaderOperation.h>

/**
 * An operation for reading videos from files.
 *
 * @inputs
 *
 * @in filename - an optional input for reading in the video file
 * name. If this input is connected, the operation will send the
 * output frames on a flow level one higher than that of the input. If
 * both @p trigger and @p filename are connected, each frame within
 * the video file must be separately triggered.
 *
 * @ingroup PiiVideoPlugin
 */
class PII_VIDEO_EXPORT PiiVideoFileReader : public PiiImageReaderOperation
{
  Q_OBJECT

  /**
   * A video file name. An example: "videos/video1.avi". 
   */
  Q_PROPERTY(QString fileName READ fileName WRITE setFileName);

  /**
   * The number of times the source emits the video. This is useful
   * if you want to process the same video again and again. 1 means
   * once, < 1 means eternally. This property has effect only if the
   * trigger input is not connected.
   *
   */
  Q_PROPERTY(int repeatCount READ repeatCount WRITE setRepeatCount);

  /**
   * FrameStep description
   */
  Q_PROPERTY(int frameStep READ frameStep WRITE setFrameStep);
  
  
  PII_OPERATION_SERIALIZATION_FUNCTION

public:

  PiiVideoFileReader();
  ~PiiVideoFileReader();
  
  QString fileName() const;
  void setFileName(const QString& fileName);

  int repeatCount() const;
  void setRepeatCount(int cnt);

  void setFrameStep(int frameStep);
  int frameStep() const;

protected:

  void process();
  void check(bool reset);

private:
  template <class T> void emitFrames();
  template <class T> void emitFrame(int iFrameStep);
  template <class T> bool tryToEmitFrame(int iFrameStep);
  void initializeVideoReader(const QString& fileName);

  /// @internal
  class Data : public PiiImageReaderOperation::Data
  {
  public:
    Data();

    QString strFileName;
    int iRepeatCount;
    PiiVideoReader* pVideoReader;
    PiiInputSocket *pFileNameInput;
    int iFrameStep, iVideoIndex;
    bool bFileNameConnected, bTriggered;
  };
  PII_D_FUNC;
};

#endif //_PIIVIDEOFILEREADER_H
