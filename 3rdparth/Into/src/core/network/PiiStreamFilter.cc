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

#include "PiiStreamFilter.h"

PiiStreamFilter::Data::~Data() {}

PiiStreamFilter::PiiStreamFilter() : d(0) {}
PiiStreamFilter::PiiStreamFilter(Data* data) : d(data) {}
PiiStreamFilter::~PiiStreamFilter() { delete d; }
void PiiStreamFilter::setOutputFilter(PiiStreamFilter*) {}
PiiStreamFilter* PiiStreamFilter::outputFilter() const { return 0; }
qint64 PiiStreamFilter::flushFilter() { return 0; }
qint64 PiiStreamFilter::bufferedSize() const { return -1; }
qint64 PiiStreamFilter::filterData(const QByteArray& array) { return filterData(array.constData(), array.size()); }

PiiDefaultStreamFilter::Data::Data() : pOutputFilter(0) {}
PiiDefaultStreamFilter::PiiDefaultStreamFilter() : PiiStreamFilter(new Data) {}
PiiDefaultStreamFilter::PiiDefaultStreamFilter(Data* data) : PiiStreamFilter(data) {}
void PiiDefaultStreamFilter::setOutputFilter(PiiStreamFilter* outputFilter) { _d()->pOutputFilter = outputFilter; }
PiiStreamFilter* PiiDefaultStreamFilter::outputFilter() const { return _d()->pOutputFilter; }
