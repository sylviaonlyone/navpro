// CameraLink serial API as per document February 2002: Camera Link Serial Software Enhancements
#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#ifdef Q_OS_WIN
#  define CLSER_EXPORT __declspec(dllimport)
#  define CLSER_CC __cdecl
#else
#  define CLSER_EXPORT
#  define CLSER_CC
#endif



//////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////

// Baud rates
#define CL_BAUDRATE_9600                        0x00000001
#define CL_BAUDRATE_19200                       0x00000010
#define CL_BAUDRATE_38400                       0x00000100
#define CL_BAUDRATE_57600                       0x00001000
#define CL_BAUDRATE_115200                      0x00010000
#define CL_BAUDRATE_230400                      0x00100000
#define CL_BAUDRATE_460800                      0x01000000
#define CL_BAUDRATE_921600                      0x10000000

//////////////////////////////////////////////////////////////////////
//  Error Codes
//////////////////////////////////////////////////////////////////////
#define CL_ERR_NO_ERR                           0
#define CL_ERR_BUFFER_TOO_SMALL                 -10001
#define CL_ERR_MANU_DOES_NOT_EXIST              -10002
#define CL_ERR_UNABLE_TO_OPEN_PORT              -10003
#define CL_ERR_PORT_IN_USE                      -10003
#define CL_ERR_TIMEOUT                          -10004
#define CL_ERR_INVALID_INDEX                    -10005
#define CL_ERR_INVALID_REFERENCE                -10006
#define CL_ERR_ERROR_NOT_FOUND                  -10007
#define CL_ERR_BAUD_RATE_NOT_SUPPORTED          -10008
#define CL_ERR_UNABLE_TO_LOAD_DLL               -10098
#define CL_ERR_FUNCTION_NOT_FOUND               -10099


// ***************************************************************************
#ifdef __cplusplus
extern "C" {
#endif
    CLSER_EXPORT int CLSER_CC clSerialInit(unsigned long SerialIndex, void** SerialRefPtr);
    CLSER_EXPORT int CLSER_CC clSerialWrite(void* SerialRef, char* Buffer, unsigned long* BufferSize, unsigned long SerialTimeout);
    CLSER_EXPORT int CLSER_CC clSerialRead(void* SerialRef, char* Buffer, unsigned long* BufferSize, unsigned long SerialTimeout);
    CLSER_EXPORT int CLSER_CC clSerialClose(void* SerialRef);

    CLSER_EXPORT int CLSER_CC clGetManufacturerInfo(char* ManufacturerName, unsigned int* BufferSize, unsigned int *Version);
    CLSER_EXPORT int CLSER_CC clGetNumSerialPorts(unsigned int* NumSerialPorts);
    CLSER_EXPORT int CLSER_CC clGetSerialPortIdentifier(unsigned long SerialIndex, char* PortId, unsigned long* BufferSize);
    CLSER_EXPORT int CLSER_CC clGetSupportedBaudRates(void *SerialRef, unsigned int* BaudRates);
    CLSER_EXPORT int CLSER_CC clSetBaudRate(void* SerialRef, unsigned int BaudRate);
    CLSER_EXPORT int CLSER_CC clGetErrorText(int ErrorCode, char *ErrorText, unsigned int *ErrorTextSize);
    CLSER_EXPORT int CLSER_CC clGetNumBytesAvail(void *SerialRef, unsigned int *NumBytes);
    CLSER_EXPORT int CLSER_CC clFlushInputBuffer(void *SerialRef);
#ifdef __cplusplus
};
#endif

#endif //_TERMINAL_H_
