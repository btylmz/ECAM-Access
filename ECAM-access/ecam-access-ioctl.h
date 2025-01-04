#pragma once


//
// Generate our device interface GUID
//
#include <initguid.h> // required for GUID definitions
DEFINE_GUID(GUID_DEVINTERFACE_ECAM_ACCESS,
	0x79963ae7, 0x45de, 0x4a31, 0x8f, 0x34, 0xf0, 0xe8, 0x90, 0xb, 0xc2, 0x17);


//
// The following value is arbitrarily chosen from the space defined by Microsoft
// as being "for non-Microsoft use"
//
#define FILE_DEVICE_ECAM_ACCESS 0xCF55


typedef struct _BUS_INFO {
	UINT16  RegisterOffset;	// Offset of the register to read
	USHORT	b;				// Bus number
	USHORT  d;				// Device number
	USHORT  f;				// Function number
} BUS_INFO, * PBUS_INFO;

typedef struct _search_info {
	UINT16	VendorID;
	UINT16	DeviceID;
	UINT16	MaxSize;
	UINT8	MaxBusNumber;
	UINT8	MaxDeviceNumber;
	UINT8	MaxFunctionNumber;
} SEARCH_INFO, * PSEARCH_INFO;;

typedef struct _pci_device_info {
	UINT16 VendorID;
	UINT16 DeviceID;
	UINT8 BusNumber;
	UINT8 DeviceNumber;
	UINT8 FunctionNumber;
} PCI_DEVICE_INFO, * PPCI_DEVICE_INFO;

//
// Device control codes - values between 2048 and 4095 arbitrarily chosen
//
#define IOCTL_INITIATE_ECAM_ACCESS			CTL_CODE(FILE_DEVICE_ECAM_ACCESS, 2048, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_READ_DWORD_ECAM				CTL_CODE(FILE_DEVICE_ECAM_ACCESS, 2049, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_READ_LINK_WIDTH				CTL_CODE(FILE_DEVICE_ECAM_ACCESS, 2050, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FIND_PCI_DEVICES				CTL_CODE(FILE_DEVICE_ECAM_ACCESS, 2051, METHOD_BUFFERED, FILE_ANY_ACCESS)

