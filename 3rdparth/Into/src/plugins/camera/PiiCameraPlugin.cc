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

#include "PiiNetworkCameraOperation.h"
#include "PiiCameraOperation.h"
#include "protocols/PiiAviivaM2CLSerialProtocol.h"
#include "protocols/PiiAviivaSC2CLSerialProtocol.h"

PII_IMPLEMENT_PLUGIN(PiiCameraPlugin);

PII_REGISTER_OPERATION(PiiNetworkCameraOperation);
PII_REGISTER_OPERATION(PiiCameraOperation);
PII_REGISTER_CLASS(PiiAviivaM2CLSerialProtocol, PiiCameraConfigurationProtocol);
PII_REGISTER_CLASS(PiiAviivaSC2CLSerialProtocol, PiiCameraConfigurationProtocol);


#ifndef PII_NO_FIREWIRE
#include "firewire/PiiFireWireOperation.h"
PII_REGISTER_OPERATION(PiiFireWireOperation);
#endif



