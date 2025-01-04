#include <iostream>
#include "ecam.h"

#include <time.h>

static void TestFindPciDevices()
{
	UINT16 vendorID, deviceID;
	UINT32 maxSize;

	// Ask to user for vendor and device IDs
	std::cout << "Enter vendor ID (hex): 0x";
	std::cin >> std::hex >> vendorID;

	std::cout << "Enter device ID (hex): 0x";
	std::cin >> std::hex >> deviceID;

	// Ask for maximum number of devices to find (in decimal NOT HEX)
	std::cout << "Enter maximum number of devices to find: ";
	std::cin >> std::dec >> maxSize;

	clock_t start, end;

	start = clock();              // Start the stopwatch

	PCI_DEVICE_INFO* deviceList = (PCI_DEVICE_INFO*)malloc(sizeof(PCI_DEVICE_INFO) * maxSize);

	std::cout << "Searching for devices with vendor ID 0x" << std::hex << vendorID << " and device ID 0x" << std::hex << deviceID << std::endl;

	int status = FindPciDevices(vendorID, deviceID, deviceList, maxSize);

	// Print info of all devices
	for (int i = 0; i < maxSize; i++)
	{
		std::cout << "\n"; // Separator between devices
		std::cout << "====================\n"; // Separator between devices
		std::cout << "Device " << i << ":\n";
		std::cout << "Vendor ID: 0x" << std::hex << deviceList[i].VendorID << std::endl;
		std::cout << "Device ID: 0x" << std::hex << deviceList[i].DeviceID << std::endl;
		std::cout << "Bus number: " << std::dec << (int)deviceList[i].BusNumber << std::endl;
		std::cout << "Device number: " << std::dec << (int)deviceList[i].DeviceNumber << std::endl;
		std::cout << "Function number: " << std::dec << (int)deviceList[i].FunctionNumber << std::endl;
	}

	end = clock();                // Stop the stopwatch

	double elapsed_time = ((double)(end - start)) / CLOCKS_PER_SEC; // Convert to seconds
	printf("Time taken: %.6f seconds\n", elapsed_time);

}

static void TestFindLinkWidth()
{
	// Ask the user for bus parameters
	UINT8 bus, device, function;
	UINT32 linkWidth;

	std::cout << "Enter bus number: ";
	std::cin >> bus;

	std::cout << "Enter device number: ";
	std::cin >> device;

	std::cout << "Enter function number: ";
	std::cin >> function;

	int status = FindLinkWidth(bus, device, function, &linkWidth);

	if (status != 0)
	{
		std::cerr << "Failed to find link width.\n";
		return;
	}

	std::cout << "Link width: " << linkWidth << std::endl;
}

int main()
{
    int status = InitializeLib();

	if (status != 0) {
		std::cerr << "Failed to initialize library.\n";
		return -1;
	}

	TestFindLinkWidth();
	
	return 0;
}


