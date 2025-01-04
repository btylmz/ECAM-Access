#include "ecam-access.h"

NTSTATUS
DriverEntry(PDRIVER_OBJECT  DriverObject,
    PUNICODE_STRING RegistryPath)
{
    WDF_DRIVER_CONFIG driverConfig;
    NTSTATUS          status;

#if DBG
    DbgPrint("\nECAM Access Driver -- Compiled %s %s\n",
        __DATE__,
        __TIME__);
#endif

    //
    // Provide pointer to our EvtDeviceAdd event processing callback
    // function
    //
    WDF_DRIVER_CONFIG_INIT(&driverConfig, 
                            ECAMAccessEvtDeviceAdd);

    //
    // Create our WDFDriver instance
    //
    status = WdfDriverCreate(DriverObject,
        RegistryPath,
        WDF_NO_OBJECT_ATTRIBUTES,
        &driverConfig,
        WDF_NO_HANDLE);

    if (!NT_SUCCESS(status)) {
#if DBG
        DbgPrint("WdfDriverCreate failed 0x%0x\n",
            status);
#endif
    }

    return (status);
}


NTSTATUS
ECAMAccessEvtDeviceAdd(WDFDRIVER Driver,
    PWDFDEVICE_INIT DeviceInit)
{
    NTSTATUS                    status;
    WDF_OBJECT_ATTRIBUTES       objAttributes;
    WDFDEVICE                   device;
    WDF_IO_QUEUE_CONFIG         queueConfig;
    PDEVICE_CONTEXT             deviceContext;


    UNREFERENCED_PARAMETER(Driver);

    WDF_OBJECT_ATTRIBUTES_INIT(&objAttributes);

    //
    // Specify our device context
    //
    WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&objAttributes, DEVICE_CONTEXT);

    //
    // Create our device object
    //
    status = WdfDeviceCreate(&DeviceInit,
        &objAttributes,
        &device);

    if (!NT_SUCCESS(status)) {
#if DBG
        DbgPrint("WdfDeviceCreate failed 0x%0x\n",
            status);
#endif
        goto Done;
    }

    deviceContext = GetContextFromDevice(device);
    deviceContext->DriverObject = Driver;

    //
    // Tell the Framework that this device will need an interface so that
    // application can interact with it.
    //
    status = WdfDeviceCreateDeviceInterface(device,
        &GUID_DEVINTERFACE_ECAM_ACCESS,
        NULL);

    if (!NT_SUCCESS(status))
    {
#if DBG
        DbgPrint("WdfDeviceCreateDeviceInterface failed 0x%0x\n",
            status);
#endif
        return status;
    }

    //
    // Configure our Queue of incoming requests
    //
    // We use only the default Queue, and we set it for sequential processing.
    // This means that the driver will only receive one request at a time
    // from the Queue, and will not get another request until it completes
    // the previous one.
    //
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig,
        WdfIoQueueDispatchSequential);

    //
    // Define our I/O Event Processing callbacks
    //
    // We handle, read, write, and device control (IOCTL) requests.
    //
    // WDF will automagically handle Create and Close requests for us and will
    // complete any other request types with STATUS_INVALID_DEVICE_REQUEST.
    //
    queueConfig.EvtIoDeviceControl = ECAMAccessEvtDeviceControl;

    //
    // Because this is a queue for a software-only device, indicate
    // that the queue doesn't need to be power managed.
    //
    queueConfig.PowerManaged = WdfFalse;

    status = WdfIoQueueCreate(device,
        &queueConfig,
        WDF_NO_OBJECT_ATTRIBUTES,
        WDF_NO_HANDLE);

    if (!NT_SUCCESS(status)) {
#if DBG
        DbgPrint("WdfIoQueueCreate for default queue failed 0x%0x\n",
            status);
#endif
        goto Done;
    }

    return STATUS_SUCCESS;
Done:

    return (status);
}
