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

#ifndef _PIIQIMAGE_H
#define _PIIQIMAGE_H

#include "PiiMatrix.h"
#include "PiiColorTable.h"
#include "PiiSharedPtr.h"
#include <QMetaType>
#include <QImage>

/** @file */

namespace Pii
{
  /**
   * Returns @p true if @a image owns its data.
   *
   * @relates PiiQImage
   */
  PII_CORE_EXPORT bool hasOwnData(QImage* image);

  /**
   * Set the internal data pointer of @a image to @p data.
   *
   * @relates PiiQImage
   */
  PII_CORE_EXPORT void setQImageData(QImage* image, uchar* data);

  /**
   * Set the width of @a image to @p width.
   *
   * @relates PiiQImage
   */
  PII_CORE_EXPORT void setQImageWidth(QImage* image, int width);

  /**
   * Set the row length of @a image to @p bytesPerLine.
   *
   * @relates PiiQImage
   */
  PII_CORE_EXPORT void setQImageRowLength(QImage* image, int bytesPerLine);

  /**
   * Set the format of @a image to @p format.
   *
   * @relates PiiQImage
   */
  PII_CORE_EXPORT void setQImageFormat(QImage* image, QImage::Format format);
  
  /**
   * Makes @a image believe it no longer owns its internal buffer. The
   * image works as usual but the buffer must be freed (with @p
   * free()) by the caller.
   *
   * @relates PiiQImage
   */
  PII_CORE_EXPORT uchar* releaseQImageBits(QImage* image);


  /**
   * Converts any color image (rgb/indexed) to gray scale.
   */
  PII_CORE_EXPORT void convertToGray(QImage& img);
  
  /**
   * Converts any image to 32-bit rgb.
   */
  PII_CORE_EXPORT void convertToRgba(QImage& img);
}

/**
 * A matrix that shares data with a QImage, and an image that shares
 * data with a PiiMatrix. The matrix is constructed with either a
 * QImage or a PiiMatrix, and modifying either will also modify the
 * other. The data will always be owned by the PiiMatrix part of the
 * class; if you construct a %PiiQImage with a QImage, the
 * QImage will be hacked to believe it doesn't own its data any more.
 *
 * %PiiQImage breaks many usual programming paradigms, and can be
 * considered an ugly, dangerous hack. But it saves a lot of memory
 * and processing time by making conversions between PiiMatrix and
 * QImage unnecessary.
 * 
 * Since QImage supports 1, 8 and 32 bit images, and PiiMatrix cannot
 * go below 8 bits, the template type must be either 8 or 32 bits in
 * size. Use unsigned char for gray scale and indexed images, and
 * PiiColor4<unsigned char> for color images.
 *
 * @b WARNING! If you make a copy of PiiQImage either as a QImage or
 * as a PiiMatrix, and subsequently call any non-const function on the
 * original object, the internal data will be automatically duplicated
 * by either QImage or PiiMatrix. As a result, the inherited PiiQImage
 * ends up having two data buffers, and either one may change
 * depending on which function you call. (Now, stop here and re-read. 
 * Seriously.) There is no way PiiQImage could prevent this. To ease
 * things a bit make sure that PiiQImages are always passed as
 * pointers.
 *
 * @code
 * QImage img;
 * PiiColorQImage* pMatrix = PiiColorQImage::create(img); // takes img's data
 * // pMatrix->row(0) and pMatrix->scanLine(0) point to the same memory location
 * QImage img2(*pMatrix); // Creates a shallow copy of matrix
 * pMatrix->scanLine(0); // Detaches the QImage part of matrix
 * // Now, the row pointers are different
 * @endcode
 *
 * If you construct a %PiiQImage with a PiiMatrix, make sure
 * that the original goes out of scope or gets modified before you
 * modify the result.
 * 
 * @code
 * PiiMatrix<unsigned char> mat;
 * PiiGrayQImage* pImg = PiiGrayQImage::create(mat);
 * pImg->row(0); // WRONG, non-const function detaches matrix data
 *
 * // Modify the original before the copy ...
 * PiiGrayQImage* pImg = PiiGrayQImage::create(mat);
 * mat = PiiMatrix<unsigned char>();
 * pImg->row(0); // fine, pImg is the sole owner of its data
 *
 * // ... or make sure the original goes out of scope.
 * PiiGrayQImage* pImg;
 * {
 *   PiiMatrix<unsigned char> mat;
 *   pImg = PiiGrayQImage::create(mat);
 * }
 * pImg->row(0); // fine, pImg is the sole owner of its data
 * @endcode
 *
 * @ingroup Core
 */
template <class T> class PiiQImage :
  public QImage,
  private PiiMatrix<T>
{
public:
  /**
   * Returns a new %PiiQImage that steals the data buffer from
   * @a image. 
   *
   * One must ensure that the depth of the given image (image.depth())
   * equals to the size of the template type (sizeof(T)*8). Otherwise,
   * the behavior is undefined (read: your program will crash). The
   * QImage will still be valid after constructing a %PiiQImage,
   * but it has lost the ownership of its internal buffer. 
   * %PiiQImage will @p free() the buffer when deleted. If @a
   * image doesn't own its data buffer (it is constructed with
   * external data), the external data must remain valid throughout
   * the lifetime of this object and will not be released upon
   * destruction.
   */
  static PiiQImage* create(QImage& image);

  /**
   * Returns a new %PiiQImage is a shallow copy of @a matrix and
   * shares the same data with QImage. If @a matrix is already shared,
   * it will be detached (see PiiTypelessMatrix::detach()). The color
   * depth of the resulting image will correspond to the size of the
   * template type @p T: <tt>unsigned char</tt> and @p char will
   * become an 8-bit indexed image, PiiColor4<unsigned char> and
   * PiiColor4<char> will become 32-bit RGB. Make sure to call a
   * non-const member function (or detach()) of @a matrix before
   * modifying the %PiiQImage.
   */
  static PiiQImage* create(PiiMatrix<T>& matrix);

  /**
   * Returns a new %PiiQImage that is a deep copy of @a matrix
   * and shared the same data with QImage. This function automatically
   * converts the data type of @a matrix to a QImage-compatible type.
   *
   * @code
   * // QImage doesn't support 24-bit RGB
   * PiiMatrix<PiiColor<unsigned char> > mat;
   * // The result is 32-bit RGB
   * PiiColorQImage* pImg = PiiColorQImage::create(mat);
   * @endcode
   */
  template <class U> static PiiQImage* create(const PiiMatrix<U>& matrix);

  ~PiiQImage() {}

  /**
   * Transfers the ownership of the internal data pointer to a new
   * matrix, deletes @p this and returns the new matrix. This function
   * is useful if you need to get rid of the QImage part of a
   * %PiiQImage. If you pass a pointer to a %PiiQImage as
   * a PiiMatrix, you risk leaking memory because PiiMatrix doesn't
   * have a virtual destructor.
   *
   * @code
   * template <class T> void destroy(PiiMatrix<T>* mat)
   * {
   *   delete mat;
   * }
   *
   * QImage qimg;
   * PiiColorQImage* pImg = PiiColorQImage::create(qimg);
   * destroy(pImg); // WRONG! QImage's destructor won't be called
   * destroy(pImg->toMatrixPointer()); // correct
   * // pImg is now deleted, don't touch!
   * @endcode
   */
  PiiMatrix<T>* toMatrixPointer()
  {
    PiiMatrix<T>* pResult = new PiiMatrix<T>(PII_MOVE(*this));
    delete this;
    return pResult;
  }
  
  /**
   * @overload
   * Returns a new matrix in the stack. This function can be used to
   * move the data from a QImage to a PiiMatrix in the stack:
   *
   * @code
   * PiiMatrix<PiiColor4<> > func()
   * {
   *   QImage img(10, 10, QImage::Format_RGB32);
   *   return PiiColorQImage::create(img)->toMatrix();
   * }
   * @endcode
   */
  PiiMatrix<T> toMatrix()
  {
    PiiMatrix<T> result(PII_MOVE(*this));
    delete this;
    return result;
  }
  
  /**
   * Convert a PiiMatrix to a QImage. This function creates a deep
   * copy of matrix' data.
   */
  static QImage matrixToQImage(const PiiMatrix<T>& matrix);

private:
  PiiQImage(void* data, int rows, int columns, int stride,
                  Pii::PtrOwnership ownership);
  PiiQImage(const PiiMatrix<T>& matrix);
  
  PII_DISABLE_COPY(PiiQImage);
};

typedef PiiSharedPtr<QImage> QImagePtr;

Q_DECLARE_METATYPE(QImagePtr);

#include <PiiColor.h>

typedef PiiQImage<unsigned char> PiiGrayQImage;
typedef PiiQImage<PiiColor4<unsigned char> > PiiColorQImage;

template <class T> struct PiiQImageTraits;
template <> struct PiiQImageTraits<char> { enum { Format = QImage::Format_Indexed8 }; };
template <> struct PiiQImageTraits<unsigned char> { enum { Format = QImage::Format_Indexed8 }; };
template <> struct PiiQImageTraits<PiiColor4<char> > { enum { Format = QImage::Format_RGB32 }; };
template <> struct PiiQImageTraits<PiiColor4<unsigned char> > { enum { Format = QImage::Format_RGB32 }; };

template <class T> PiiQImage<T>* PiiQImage<T>::create(QImage& image)
{
  Q_ASSERT(image.depth() == sizeof(T)*8);
  void *pData;
  Pii::PtrOwnership ownership = Pii::ReleaseOwnership;
  if (Pii::hasOwnData(&image))
    {
      image.detach();
      pData = Pii::releaseQImageBits(&image);
    }
  // If image doesn't own its data, we can't either.
  else
    {
      pData = (void*)(const_cast<const QImage&>(image).bits());
      ownership = Pii::RetainOwnership;
    }
  PiiQImage<T>* pImage = new PiiQImage<T>(pData, image.height(), image.width(),
                                          image.bytesPerLine(),
                                          ownership);
  pImage->setColorTable(image.colorTable());
  return pImage;
}

template <class T> PiiQImage<T>* PiiQImage<T>::create(PiiMatrix<T>& matrix)
{
  matrix.detach();
  PiiQImage<T>* pImage = new PiiQImage<T>(matrix);
  if (sizeof(T) == 1)
    pImage->setColorTable(Pii::grayColorTable());
  return pImage;
}

template <class T> template <class U>
PiiQImage<T>* PiiQImage<T>::create(const PiiMatrix<U>& matrix)
{
  PiiMatrix<T> tmp(matrix);
  return create(tmp);
}

template <class T> PiiQImage<T>::PiiQImage(void* data, int rows, int columns, int stride,
                                           Pii::PtrOwnership ownership) :
  QImage(static_cast<uchar*>(data), columns, rows, stride, (QImage::Format)PiiQImageTraits<T>::Format),
  PiiMatrix<T>(rows, columns, data, ownership, stride)
{
}

template <class T> PiiQImage<T>::PiiQImage(const PiiMatrix<T>& matrix) :
  QImage((uchar*)matrix.row(0),
         matrix.columns(), matrix.rows(),
         matrix.stride(),
         (QImage::Format)PiiQImageTraits<T>::Format),
  PiiMatrix<T>(matrix)
{
}

template <class T> QImage PiiQImage<T>::matrixToQImage(const PiiMatrix<T>& matrix)
{
  QImage result(matrix.columns(), matrix.rows(), (QImage::Format)PiiQImageTraits<T>::Format);
  int iBytesPerRow = matrix.columns() * sizeof(T);
  for (int r=0; r<matrix.rows(); ++r)
    memcpy(result.scanLine(r), matrix.row(r), iBytesPerRow);
  if (sizeof(T) == 1)
    result.setColorTable(Pii::grayColorTable());
  return result;
}

namespace Pii
{
  template <class T> QImage* createQImage(PiiMatrix<T>& matrix)
  {
    return Pii::IfClass<Pii::IsColor<T>,
                        PiiColorQImage,
                        PiiGrayQImage>::Type::create(matrix);
  }
  template <class T> QImage* createQImage(const PiiMatrix<T>& matrix)
  {
    return Pii::IfClass<Pii::IsColor<T>,
                        PiiColorQImage,
                        PiiGrayQImage>::Type::create(matrix);
  }

  template <class T> QImage* createQImage(const PiiMatrix<PiiColor<T> >& matrix)
  {
    PiiMatrix<PiiColor4<T> > matTmp(matrix);
    return createQImage(matTmp);
  }
}

#endif //_PIIQIMAGE_H
