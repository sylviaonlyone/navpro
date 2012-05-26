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

#include "PiiBoostClassifier.h"

const char* pBoostFactoryNotSetError = QT_TRANSLATE_NOOP("PiiBoostClassifier", "Factory object is not set.");
const char* pBoostTooFewClassesError =
  QT_TRANSLATE_NOOP("PiiBoostClassifier",
                    "Training set must containt at least two classes. It only has %1.");
const char* pBoostTooManyClassesError =
  QT_TRANSLATE_NOOP("PiiBoostClassifier",
                    "Only SammeBoost accepts more than two classes. The training set contains %1.");
const char* pBoostTooWeakClassifierError =
  QT_TRANSLATE_NOOP("PiiBoostClassifier",
                    "Boosting was interrupted due to a too weak classifier. Error = %1, must be less than %2.");
