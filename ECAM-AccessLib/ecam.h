#pragma once

#include <cstdio>
#include <cstdlib>
#include <thread>
#include <windows.h>
#include <iostream>

#include <cfgmgr32.h>
#include <SetupAPI.h>

#include "../ECAM-access/ecam-access-ioctl.h"

/// <summary>
/// Initialize the library.
/// </summary>
/// <returns>0 if library initialized successfully</returns>
__declspec(dllexport) INT32 __stdcall InitializeLib();

/// <summary>
/// Read a dword from the PCI configuration space.
/// </summary>
/// <param name="bus_info">Bus info of the PCI device</param>
/// <param name="value">Returned data</param>
/// <returns>Status of the operation</returns>
__declspec(dllexport) INT32 __stdcall ReadPciConfigDword(UINT8 bus, UINT8 device, UINT8 function, UINT16 offset, DWORD* value);

/// <summary>
/// Get the PCI device ID of the PCI device at the specified bus, device, and function.
/// </summary>
/// <param name="bus"></param>
/// <param name="device"></param>
/// <param name="function"></param>
/// <param name="vendorID"></param>
/// <param name="deviceID"></param>
/// <returns>Status of the operation</returns>
__declspec(dllexport) INT32 __stdcall GetPciDeviceId(UINT8 bus, UINT8 device, UINT8 function, UINT16 *vendorID, UINT16 *deviceID);

/// <summary>
/// Get the PCI bus numbers, primary, secondary, and subordinate, of the PCI device at the specified bus, device, and function.
/// </summary>
/// <param name="bus"></param>
/// <param name="device"></param>
/// <param name="function"></param>
/// <param name="primaryBusNumber"></param>
/// <param name="secondaryBusNumber"></param>
/// <param name="subordinateBusNumber"></param>
/// <returns>Status of the operation</returns>
__declspec(dllexport) INT32 __stdcall GetPciBusNumbers(UINT8 bus, UINT8 device, UINT8 function, UINT8* primaryBusNumber, UINT8* secondaryBusNumber, UINT8* subordinateBusNumber);


/// <summary>
/// Find all PCI devices with the specified vendor and device IDs.
/// </summary>
/// <param name="vendorID"></param>
/// <param name="deviceID"></param>
/// <param name="deviceList">Array of found devices</param>
/// <returns>Status of the operation</returns>
__declspec(dllexport) INT32 __stdcall FindPciDevices(UINT16 vendorID, UINT16 deviceID, PCI_DEVICE_INFO deviceList[], UINT32 maxSize);


__declspec(dllexport) INT32 __stdcall FindPciDevicesEx(UINT16 vendorID,
	UINT16 deviceID,
	PCI_DEVICE_INFO deviceList[],
	UINT32 maxSize,
	UINT8 maxBusNumber,
	UINT8 maxDeviceNumber,
	UINT8 maxFunctionNumber);

__declspec(dllexport) INT32 __stdcall FindLinkWidth(UINT8 bus, UINT8 device, UINT8 function, UINT32* linkWidth);
