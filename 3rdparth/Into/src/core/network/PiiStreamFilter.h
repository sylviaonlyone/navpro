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

#ifndef _PIISTREAMFILTER_H
#define _PIISTREAMFILTER_H

#include "PiiNetworkGlobal.h"
#include <QIODevice>

/**
 * An interface for stream filters. A stream filter is a class that
 * converts data written to it and writes it either to another filter
 * or somewhere else. Input data to the filter is sent with the
 * #writeToStream() function. Filter chains can be though of as linked
 * lists. The last filter on the list has no output filter.
 *
 * @ingroup PiiNetwork
 */
class PII_NETWORK_EXPORT PiiStreamFilter
{
public:
  virtual ~PiiStreamFilter();

  /**
   * Writes at most @p maxSize bytes of @p data to this output filter. 
   * Returns the number of bytes actually written.
   */
  virtual qint64 filterData(const char* data, qint64 maxSize) = 0;

  /**
   * @overload
   *
   * Writes @p data to the filter and returns the number of bytes
   * written.
   */
  qint64 filterData(const QByteArray& data);
  
  /**
   * Sets the next output filter in the filter chain. If the output
   * filter is non-null, this filter writes its data to it after
   * conversion. The default implementation does nothing.
   */
  virtual void setOutputFilter(PiiStreamFilter* filter);
  /**
   * Returns the next output filter in the filter chain. The default
   * implementation returns 0.
   */
  virtual PiiStreamFilter* outputFilter() const;
  
  /**
   * Flushes all buffered data to the output filter. Returns the
   * number of bytes written. The default implementation returns 0.
   */
  virtual qint64 flushFilter();
  /**
   * Returns the number of bytes buffered into the filter and pending
   * output. If the filter does not buffer data or does not know how
   * much data is yet to come, it must return -1. The default
   * implementation returns -1.
   */
  virtual qint64 bufferedSize() const;

protected:
  /// @internal
  class Data
  {
  public:
    virtual ~Data();
  } *d;
  
  /// @internal
  PiiStreamFilter(Data* d);

  PiiStreamFilter();
};

/**
 * Default implementation of the PiiStreamFilter interface. Stores the
 * output filter into an internal data member.
 *
 * @ingroup PiiNetwork
 */
class PII_NETWORK_EXPORT PiiDefaultStreamFilter : public PiiStreamFilter
{
public:
  /**
   * Constructs a default stream filter whose output filter is zero.
   */
  PiiDefaultStreamFilter();
  
  /**
   * Sets output filter.
   */
  void setOutputFilter(PiiStreamFilter* outputFilter);
  /**
   * Returns the current output filter.
   */
  PiiStreamFilter* outputFilter() const;

protected:
  /// @internal
  class Data : public PiiStreamFilter::Data
  {
  public:
    Data();
    PiiStreamFilter* pOutputFilter;
  };
  PII_D_FUNC;
  /// @internal
  PiiDefaultStreamFilter(Data* data);
};

#endif //_PIISTREAMFILTER_H
