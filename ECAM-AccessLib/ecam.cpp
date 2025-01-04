#include "ecam.h"
#include "mcfg.h"

extern "C"
{
    /// <summary>
	/// Opens device handle by GUID
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    static HANDLE OpenHandleByGUID(void)
    {
        CONFIGRET configReturn;
        DWORD     lasterror;
        WCHAR     deviceName[MAX_DEVICE_ID_LEN];
        HANDLE    handleToReturn = INVALID_HANDLE_VALUE;

        //
        // Get the device interface -- we only expose one
        //
        deviceName[0] = UNICODE_NULL;

        configReturn = CM_Get_Device_Interface_List((LPGUID)&GUID_DEVINTERFACE_ECAM_ACCESS,
            nullptr,
            deviceName,
            _countof(deviceName),
            CM_GET_DEVICE_INTERFACE_LIST_PRESENT);

        if (configReturn != CR_SUCCESS) {
            lasterror = GetLastError();

            printf("CM_Get_Device_Interface_List fail: %lx\n",
                lasterror);

            goto Exit;
        }

        //
        // Make sure there's an actual name there
        //
        if (deviceName[0] == UNICODE_NULL) {
            lasterror = ERROR_NOT_FOUND;
            goto Exit;
        }

        //
        // Open the device
        //
        handleToReturn = CreateFile(deviceName,
            GENERIC_WRITE | GENERIC_READ,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr);

        if (handleToReturn == INVALID_HANDLE_VALUE) {
            lasterror = GetLastError();
            printf("CreateFile fail: %lx\n",
                lasterror);
        }

    Exit:

        //
        // Return a handle to the device
        //
        return handleToReturn;
    }


}

HANDLE deviceHandle = INVALID_HANDLE_VALUE;
UINT64 baseAddress = 0;

INT32 __stdcall InitializeLib()
{
	deviceHandle = OpenHandleByGUID();
	if (deviceHandle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	baseAddress = GetECAMBaseAddress();

	if (baseAddress == -1)
	{
		return -1;
	}

	if (!DeviceIoControl(
		deviceHandle,
		IOCTL_INITIATE_ECAM_ACCESS,
		&baseAddress,
		sizeof(UINT64),
		nullptr,
		0,
		nullptr,
		nullptr))
	{
		return -1;
	}

	return 0;
}

INT32 __stdcall ReadPciConfigDword(UINT8 bus, UINT8 device, UINT8 function, UINT16 offset, DWORD* value)
{
    DWORD  bytesReturned;
	BUS_INFO bus_info;
    DWORD readValue = 0;
	bus_info.b = bus;
	bus_info.d = device;
	bus_info.f = function;
	bus_info.RegisterOffset = offset;

	if (!DeviceIoControl(
        deviceHandle, 
        IOCTL_READ_DWORD_ECAM,
        &bus_info,
        sizeof(BUS_INFO),
        &readValue,
        sizeof(DWORD),
        &bytesReturned,
        nullptr))
	{
		return -1;
	}

    *value = readValue;

	return 0;
}

INT32 __stdcall GetPciDeviceId(UINT8 bus, UINT8 device, UINT8 function, UINT16* vendorID, UINT16* deviceID)
{
    DWORD  bytesReturned;
    BUS_INFO bus_info;
    DWORD readValue = 0;
    bus_info.b = bus;
    bus_info.d = device;
    bus_info.f = function;
    bus_info.RegisterOffset = 0x00;

	if (!DeviceIoControl(
		deviceHandle,
        IOCTL_READ_DWORD_ECAM,
		&bus_info,
		sizeof(BUS_INFO),
		&readValue,
		sizeof(DWORD),
		&bytesReturned,
		nullptr))
	{
		return -1;
	}

	// Vendor ID is the lower 16 bits
	*vendorID = (UINT16)(readValue & 0xFFFF);

	// Device ID is the upper 16 bits
	*deviceID = (UINT16)((readValue >> 16) & 0xFFFF);

    return 0;
}

INT32 __stdcall GetPciBusNumbers(UINT8 bus, UINT8 device, UINT8 function, UINT8* primaryBusNumber, UINT8* secondaryBusNumber, UINT8* subordinateBusNumber)
{
	DWORD  bytesReturned;
	BUS_INFO bus_info;
	DWORD readValue = 0;
	bus_info.b = bus;
	bus_info.d = device;
	bus_info.f = function;
	bus_info.RegisterOffset = 0x18;

	if (!DeviceIoControl(
		deviceHandle,
        IOCTL_READ_DWORD_ECAM,
		&bus_info,
		sizeof(BUS_INFO),
		&readValue,
		sizeof(DWORD),
		&bytesReturned,
		nullptr))
	{
		return -1;
	}

	*primaryBusNumber = (UINT8)(readValue & 0xFF);
	*secondaryBusNumber = (UINT8)((readValue >> 8) & 0xFF);
	*subordinateBusNumber = (UINT8)((readValue >> 16) & 0xFF);

	return 0;
}


INT32 __stdcall FindPciDevices(UINT16 vendorID, UINT16 deviceID, PCI_DEVICE_INFO deviceList[], UINT32 maxSize)
{
	int status = 0;
	DWORD bytesReturned = 0;

	if (deviceList == nullptr)
	{
		printf("deviceList is null\n");
		return -1;
	}

	if (maxSize == 0)
	{
		printf("maxSize is 0\n");
		return -1;
	}

	SEARCH_INFO searchInfo;

	searchInfo.VendorID = vendorID;
	searchInfo.DeviceID = deviceID;
	searchInfo.MaxSize = maxSize;
	searchInfo.MaxBusNumber = 255;
	searchInfo.MaxDeviceNumber = 32;
	searchInfo.MaxFunctionNumber = 8;

	if (!DeviceIoControl(
		deviceHandle,
		IOCTL_FIND_PCI_DEVICES,
		&searchInfo,
		sizeof(SEARCH_INFO),
		deviceList,
		sizeof(PCI_DEVICE_INFO) * maxSize,
		&bytesReturned,
		nullptr))
	{
		return -1;
	}

	return status;

}

INT32 __stdcall FindPciDevicesEx(UINT16 vendorID,
	UINT16 deviceID,
	PCI_DEVICE_INFO deviceList[],
	UINT32 maxSize,
	UINT8 maxBusNumber,
	UINT8 maxDeviceNumber,
	UINT8 maxFunctionNumber)
{
	int status = 0;

	if (deviceList == nullptr)
	{
		return -1;
	}

	if (maxSize == 0)
	{
		return -1;
	}

	SEARCH_INFO searchInfo;

	searchInfo.VendorID = vendorID;
	searchInfo.DeviceID = deviceID;
	searchInfo.MaxSize = maxSize;
	searchInfo.MaxBusNumber = maxBusNumber;
	searchInfo.MaxDeviceNumber = maxDeviceNumber;
	searchInfo.MaxFunctionNumber = maxFunctionNumber;

	if (!DeviceIoControl(
		deviceHandle,
		IOCTL_FIND_PCI_DEVICES,
		&searchInfo,
		sizeof(SEARCH_INFO),
		deviceList,
		sizeof(PCI_DEVICE_INFO) * maxSize,
		(DWORD*)&status,
		nullptr))
	{
		return -1;
	}

	return status;
}

INT32 __stdcall FindLinkWidth(UINT8 bus, UINT8 device, UINT8 function, UINT32* linkWidth)
{
	DWORD  bytesReturned;
	BUS_INFO bus_info;
	DWORD readValue = 0;
	bus_info.b = bus;
	bus_info.d = device;
	bus_info.f = function;

	if (!DeviceIoControl(
		deviceHandle,
		IOCTL_READ_LINK_WIDTH,
		&bus_info,
		sizeof(BUS_INFO),
		&readValue,
		sizeof(DWORD),
		&bytesReturned,
		nullptr))
	{
		return -1;
	}

	*linkWidth = readValue;

	return 0;
}