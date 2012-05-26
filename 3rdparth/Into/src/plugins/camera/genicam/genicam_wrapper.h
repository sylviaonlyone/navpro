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

#ifndef _GENICAM_WRAPPER_H
#define _GENICAM_WRAPPER_H

#ifdef _WIN32
#  define GENICAM_WAPI(TYPE) __declspec(dllexport) TYPE __stdcall
#  define GENICAM_TYPEDEF(TYPE, FUNC) typedef __declspec(dllexport) TYPE (__stdcall *FUNC)
#else
#  define GENICAM_WAPI(TYPE) TYPE
#  define GENICAM_TYPEDEF(TYPE, FUNC) typedef TYPE (*FUNC)
#endif

#ifdef __cplusplus
extern "C" {
#endif

  struct genicam_device;

  GENICAM_WAPI(int) genicam_initialize(void);
  GENICAM_WAPI(int) genicam_terminate(void);

  GENICAM_WAPI(int) genicam_list_cameras(char** names, int* count);
  GENICAM_WAPI(const char*) genicam_next_camera(const char* serial);

  GENICAM_WAPI(int) genicam_open_device(const char* serial, genicam_device **device);
  GENICAM_WAPI(int) genicam_close_device(genicam_device* device);

  GENICAM_WAPI(int) genicam_free(void* data);
  GENICAM_WAPI(const char*) genicam_last_error(void);
  
  GENICAM_WAPI(int) genicam_set_property(genicam_device* device, const char* name, int value);
  GENICAM_WAPI(int) genicam_get_property(genicam_device* device, const char* name, int *value);

  GENICAM_WAPI(int) genicam_register_framebuffers(genicam_device* device, unsigned char* buffer, int count);
  GENICAM_WAPI(int) genicam_deregister_framebuffers(genicam_device* device);

  GENICAM_WAPI(int) genicam_grab_frame(genicam_device* device, unsigned char** buffer, int timeout);
  GENICAM_WAPI(int) genicam_requeue_buffers(genicam_device* device);

  GENICAM_WAPI(int) genicam_start_capture(genicam_device* device);
  GENICAM_WAPI(int) genicam_stop_capture(genicam_device* device);

#ifdef __cplusplus
}
#endif

#endif // _GENICAM_WRAPPER_H
