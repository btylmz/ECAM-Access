#include "ecam-access.h"

VOID
ECAMAccessEvtDeviceControl(WDFQUEUE   Queue,
    WDFREQUEST Request,
    size_t     OutputBufferLength,
    size_t     InputBufferLength,
    ULONG      IoControlCode)
{
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    NTSTATUS                    status = STATUS_SUCCESS;
    PDEVICE_CONTEXT             deviceContext;
    WDF_REQUEST_PARAMETERS      params;
    WDFDEVICE                   device;
    PBUS_INFO                   busInfo;


#if DBG
    DbgPrint("ECAMAccessEvtDeviceControl\n");
#endif

    device = WdfIoQueueGetDevice(Queue);
    deviceContext = GetContextFromDevice(device);

    WDF_REQUEST_PARAMETERS_INIT(&params);

    WdfRequestGetParameters(
        Request,
        &params);


    switch (IoControlCode)
    {
        case IOCTL_INITIATE_ECAM_ACCESS:
        {
            ULONG64* baseAddress;
            status = WdfRequestRetrieveInputBuffer(Request,
                sizeof(ULONG64),
                reinterpret_cast<PVOID*>(&baseAddress),
                nullptr);

            if (!NT_SUCCESS(status))
            {
    #if DBG
                DbgPrint("Failed to retrieve input buffer\n");
    #endif
                goto Done;
            }

            deviceContext->EcamBaseAddress = *baseAddress;

            WdfRequestCompleteWithInformation(Request,
                status,
                sizeof(ULONG64));

    #if DBG
            DbgPrint("Initiated ECAM access, base address: %llx\n", deviceContext->EcamBaseAddress);
    #endif

            return;
        }

        case IOCTL_READ_DWORD_ECAM:
        {
            ULONG* value;
            status = WdfRequestRetrieveInputBuffer(Request,
                sizeof(BUS_INFO),
                reinterpret_cast<PVOID*>(&busInfo),
                nullptr);

            if (!NT_SUCCESS(status))
            {
                goto Done;
            }

            status = WdfRequestRetrieveOutputBuffer(Request,
                sizeof(ULONG),
                reinterpret_cast<PVOID*>(&value),
                nullptr);

            if (!NT_SUCCESS(status))
            {
                goto Done;
            }

            PHYSICAL_ADDRESS ECAMAddress;
            ECAMAddress.QuadPart = deviceContext->EcamBaseAddress + (busInfo->b << 20) + (busInfo->d << 15) + (busInfo->f << 12);

            // Map ECAM base address
            PVOID configSpace = MmMapIoSpace(ECAMAddress, 4096, MmNonCached);
            if (configSpace == NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto Done;
            }

            *value = READ_REGISTER_ULONG(reinterpret_cast<PULONG>(static_cast<PUCHAR>(configSpace) + busInfo->RegisterOffset));

            MmUnmapIoSpace(configSpace, 4096);

            WdfRequestCompleteWithInformation(Request,
                status,
                sizeof(ULONG));

            return;
        }

        case IOCTL_FIND_PCI_DEVICES:
        {
            PCI_DEVICE_INFO* deviceList;
            ULONG foundDevices = 0;
            UCHAR maxBusNumber = 0, maxDeviceNumber = 0, maxFunctionNumber = 0;
            PSEARCH_INFO searchInfo;
            ULONG value;
            USHORT vendorID, deviceID;
            ULONG maxSize = 0;

            status = WdfRequestRetrieveInputBuffer(Request,
                sizeof(SEARCH_INFO),
                reinterpret_cast<PVOID*>(&searchInfo),
                nullptr);

            if (!NT_SUCCESS(status))
            {
    #if DBG
                DbgPrint("Failed to retrieve input buffer\n");
    #endif
                goto Done;
            }

            status = WdfRequestRetrieveOutputBuffer(Request,
                sizeof(PCI_DEVICE_INFO) * searchInfo->MaxSize,
                reinterpret_cast<PVOID*>(&deviceList),
                nullptr);

            if (!NT_SUCCESS(status))
            {
    #if DBG
                DbgPrint("Failed to retrieve output buffer\n");
    #endif
                goto Done;
            }

            maxSize = searchInfo->MaxSize;
            maxBusNumber = searchInfo->MaxBusNumber;
            maxDeviceNumber = searchInfo->MaxDeviceNumber;
            maxFunctionNumber = searchInfo->MaxFunctionNumber;

            for (UCHAR b = 0; b < maxBusNumber; b++)
            {
                for (UCHAR d = 0; d < maxDeviceNumber; d++)
                {
                    for (UCHAR f = 0; f < maxFunctionNumber; f++)
                    {
                        PHYSICAL_ADDRESS ECAMAddress;
                        ECAMAddress.QuadPart = deviceContext->EcamBaseAddress + ((UINT64)b << 20) + ((UINT64)d << 15) + ((UINT64)f << 12);

                        // Map ECAM base address
                        PVOID configSpace = MmMapIoSpace(ECAMAddress, 256, MmNonCached);
                        if (configSpace == NULL) {
                            status = STATUS_INSUFFICIENT_RESOURCES;
                            goto Done;
                        }

                        // Read vendor ID and device ID
                        value = READ_REGISTER_ULONG(reinterpret_cast<PULONG>(static_cast<PUCHAR>(configSpace)));

                        vendorID = value & 0xFFFF;
                        deviceID = (value >> 16) & 0xFFFF;


                        if (vendorID == searchInfo->VendorID && deviceID == searchInfo->DeviceID)
                        {
                            deviceList[foundDevices].VendorID = vendorID;
                            deviceList[foundDevices].DeviceID = deviceID;
                            deviceList[foundDevices].BusNumber = b;
                            deviceList[foundDevices].DeviceNumber = d;
                            deviceList[foundDevices].FunctionNumber = f;

                            foundDevices++;
                            maxSize--;
                        }

                        if (maxSize == 0)
                        {
                            MmUnmapIoSpace(configSpace, 256);
                            WdfRequestCompleteWithInformation(Request,
                                status,
                                sizeof(PCI_DEVICE_INFO) * (foundDevices));
                            return;
                        }

                        MmUnmapIoSpace(configSpace, 256);

                    }
                }
            }

            WdfRequestCompleteWithInformation(Request,
                status,
                sizeof(PCI_DEVICE_INFO) * (foundDevices));

            return;
        }

        case IOCTL_READ_LINK_WIDTH:
        {
            USHORT value;
            UCHAR capabilityRegisterPointer;
            UCHAR capabilityRegisterId;
            ULONG* linkWidth = 0;

            status = WdfRequestRetrieveInputBuffer(Request,
                sizeof(BUS_INFO),
                reinterpret_cast<PVOID*>(&busInfo),
                nullptr);

            if (!NT_SUCCESS(status))
            {
                goto Done;
            }

            status = WdfRequestRetrieveOutputBuffer(Request,
                sizeof(ULONG),
                reinterpret_cast<PVOID*>(&linkWidth),
                nullptr);

            if (!NT_SUCCESS(status))
            {
                goto Done;
            }

            PHYSICAL_ADDRESS ECAMAddress;
            ECAMAddress.QuadPart = deviceContext->EcamBaseAddress + (busInfo->b << 20) + (busInfo->d << 15) + (busInfo->f << 12);

            // Map ECAM base address
            PVOID configSpace = MmMapIoSpace(ECAMAddress, 256, MmNonCached);
            if (configSpace == NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto Done;
            }

            // Read capabilities pointer
            value = READ_REGISTER_USHORT(reinterpret_cast<PUSHORT>(static_cast<PUCHAR>(configSpace) + 0x34));
            capabilityRegisterPointer = value & 0xff;

            // If capabilities pointer is zero, then the device does not support PCI Express
            if (capabilityRegisterPointer == 0x00 || capabilityRegisterPointer == 0xff)
            {
                status = STATUS_NOT_SUPPORTED;
                MmUnmapIoSpace(configSpace, 256);
                goto Done;
            }

            // Read until we find the link capabilities register
            while (1)
            {
                value = READ_REGISTER_USHORT(reinterpret_cast<PUSHORT>(static_cast<PUCHAR>(configSpace) + capabilityRegisterPointer));
                capabilityRegisterId = value & 0xff;


                if (capabilityRegisterId == 0x10)
                {
                    // Read PCI Express capabilities register
                    value = READ_REGISTER_USHORT(reinterpret_cast<PUSHORT>(static_cast<PUCHAR>(configSpace) + capabilityRegisterPointer + 0x12));

                    // First 4 bits is LinkSpeed, next 6 bits is LinkWidth
                    *linkWidth = (value >> 4) & 0x3f;

                    break;
                }
                else if (capabilityRegisterId == 0x00 || capabilityRegisterPointer == 0x00 || capabilityRegisterPointer == 0xff)
                {
                    status = STATUS_NOT_SUPPORTED;
                    break;
                }

                capabilityRegisterPointer = (value >> 8) & 0xff;

            }

            MmUnmapIoSpace(configSpace, 256);

            WdfRequestCompleteWithInformation(Request,
                status,
                sizeof(ULONG));

            return;

        }

    }

Done:

    WdfRequestCompleteWithInformation(Request,
        status,
        0);

    return;
}

