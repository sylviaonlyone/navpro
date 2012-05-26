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

#include "PiiSerializationTraits.h"

#include <cstring> //for memcpy, strlen

namespace PiiSerializationTraits
{
	char* createTemplateName(const char* className, const char* typeName)
	{
		const int iClassLen = strlen(className), iTypeLen = strlen(typeName);
		int iLocation = 0;
		char* pResult = new char[iClassLen + iTypeLen + 3];
    memcpy(pResult, className, iClassLen);
		pResult[iClassLen] = '<';
		iLocation = iClassLen + 1;
		memcpy(pResult + iLocation, typeName, iTypeLen);
		iLocation += iTypeLen;
		pResult[iLocation] = '>';
		pResult[++iLocation] = 0;
		return pResult;
	}
}
