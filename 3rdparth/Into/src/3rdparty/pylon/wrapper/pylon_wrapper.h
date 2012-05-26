/* This file is part of Into.
 * Copyright (C) Intopii
 * All rights reserved.
 */

#ifndef _PYLON_WRAPPER_H
#define _PYLON_WRAPPER_H

#ifdef _WIN32
#  ifdef BUILD_PYLON_WAPI
#    define PYLON_WAPI(type) __declspec(dllexport) type __stdcall
#  else
#    define PYLON_WAPI(type) __declspec(dllimport) type __stdcall
#  endif
#else
#  define PYLON_WAPI(type) type
#endif

#ifdef __cplusplus
extern "C" {
#endif

  struct pylon_device;

  PYLON_WAPI(void) pylon_initialize(void);
  PYLON_WAPI(void) pylon_terminate(void);

  PYLON_WAPI(char*) pylon_list_cameras(int* count);
  PYLON_WAPI(const char*) pylon_next_camera(const char* serial);

  PYLON_WAPI(pylon_device*) pylon_open_device(const char* serial);
  PYLON_WAPI(void) pylon_close_device(pylon_device* device);
  PYLON_WAPI(void) pylon_free(void* data);

  PYLON_WAPI(void) pylon_set_property(pylon_device* device, const char* name, int value);
  PYLON_WAPI(int) pylon_get_property(pylon_device* device, const char* name);

  PYLON_WAPI(void) pylon_register_framebuffers(pylon_device* device, unsigned char* buffer, int count);
  PYLON_WAPI(void) pylon_deregister_framebuffers(pylon_device* device);

  PYLON_WAPI(unsigned char*) pylon_grab_frame(pylon_device* device, int timeout);
  PYLON_WAPI(void) pylon_requeue_buffers(pylon_device* device);

  PYLON_WAPI(void) pylon_start_capture(pylon_device* device);
  PYLON_WAPI(void) pylon_stop_capture(pylon_device* device);

#ifdef __cplusplus
}
#endif

#endif // _PYLON_WRAPPER_H
