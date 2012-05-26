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

#ifndef _PIIVIDEOFILEWRITER_H
#define _PIIVIDEOFILEWRITER_H

#include <PiiDefaultOperation.h>
#include <PiiQImage.h>

#include "PiiVideoException.h"
#include "PiiVideoGlobal.h"
#include "PiiVideoWriter.h"

/**
 * An operation that writes images into video files in standard video
 * formats. If a video with the same name already exists,
 * it will be overwritten.
 *
 * @inputs
 *
 * @in image - video frames, any gray-level or color image
 *
 * @ingroup PiiVideoPlugin
 */
class PII_VIDEO_EXPORT PiiVideoFileWriter : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The directory the writer will store the video in. Relative and
   * absolute paths are OK. An example: "videos/".
   */
  Q_PROPERTY(QString outputDirectory READ outputDirectory WRITE setOutputDirectory);

  /**
   * The output file name. 
   */
  Q_PROPERTY(QString fileName READ fileName WRITE setFileName);

  /**
   * Frame rate.
   */
  Q_PROPERTY(int frameRate READ frameRate WRITE setFrameRate);
  
  
  PII_OPERATION_SERIALIZATION_FUNCTION

public:
  PiiVideoFileWriter();
  virtual ~PiiVideoFileWriter();
  
  QString outputDirectory() const;
  void setOutputDirectory(const QString& dirName);

  QString fileName() const;
  void setFileName(const QString& fileName);

  int frameRate() const;
  void setFrameRate(int frameRate);
  
protected:
  void process();

private slots:
  void deletePiiVideoWriter( int state );
  
private:
  template <class T> void initPiiVideoWriter(const PiiVariant& obj);
  template <class T> void grayImage(const PiiVariant& obj);
  template <class T> void floatImage(const PiiVariant& obj);
  template <class T> void colorImage(const PiiVariant& obj);
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();

    QString strOutputDirectory, strFileName;
    int iIndex, iWidth, iHeight, iFrameRate;
    
    PiiVideoWriter *pVideoWriter;
    PiiInputSocket* pImageInput;
  };
  PII_D_FUNC;
};


#endif //_PIIVIDEOFILEWRITER_H
