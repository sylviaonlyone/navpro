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

#include "PiiPlugin.h"

#include "PiiDemuxOperation.h"
#include "PiiPisoOperation.h"
#include "PiiFlowLevelChanger.h"
#include "PiiSwitch.h"
#include "PiiFrequencyCounter.h"
#include "PiiObjectRateChanger.h"
#include "PiiObjectReplicator.h"
#include "PiiCacheOperation.h"

PII_IMPLEMENT_PLUGIN(PiiFlowControlPlugin);

PII_REGISTER_OPERATION(PiiDemuxOperation);
PII_REGISTER_OPERATION(PiiPisoOperation);
PII_REGISTER_OPERATION(PiiFlowLevelChanger);
PII_REGISTER_OPERATION(PiiSwitch);
PII_REGISTER_OPERATION(PiiFrequencyCounter);
PII_REGISTER_OPERATION(PiiObjectRateChanger);
PII_REGISTER_OPERATION(PiiObjectReplicator);
PII_REGISTER_OPERATION(PiiCacheOperation);
