#pragma once


#include <wdm.h>
#include <wdf.h>

#include "ecam-access-ioctl.h"

//
// Device context structure
//
typedef struct _DEVICE_CONTEXT {

    WDFDRIVER DriverObject;
	ULONG64   EcamBaseAddress;

}  DEVICE_CONTEXT, * PDEVICE_CONTEXT;

//
// Accessor structure
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT,
    GetContextFromDevice)

    //
    // Forward declarations
    //
    extern "C"
{
    DRIVER_INITIALIZE                   DriverEntry;
    EVT_WDF_DRIVER_DEVICE_ADD           ECAMAccessEvtDeviceAdd;
    EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL  ECAMAccessEvtDeviceControl;
}

