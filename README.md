# ECAM-Access

This repository contains a WDF kernel driver and a user DLL project to access ECAM (Enhanced Configuration Access Mechanism) address of PCI/PCIe devices.

## Features

* Access ECAM addresses for PCI/PCIe devices.

* WDF-based kernel driver for low-level hardware access.

* User-mode DLL for simplified integration with applications.

## Building the Kernel Driver and DLL

1. Clone the repository
2. Make sure Windows Driver Kit is installed
3. Open the solution in **ECAM-Access/ECAM-access.sln**
4. Build the driver and DLL projects

## Usage 

1. Install the driver using devcon.
```bash
devcon install "ECAM-access.inf" "Root\ECAM-access" 
```
2. Use the **test-ecam-accesslib** project as a template to create your project
3. Initialize library using `InitializeLib();`
4. Use `ReadPciConfigDword` function to read from PCI configuration space.
5. Other functions are:
    a. `GetPciBusNumbers`: Returns primary bus number, secondary bus number and subordinate bus number of a PCI/PCIe device.
    b. `FindPciDevices`: Lists devices with the given vendor and device IDs.
    c. `FindLinkWidth`: Finds link width of a device using link capabilites register.

## Directory Structure
ECAM-Access/
├── ECAM-access/            # Kernel driver source code
├── ECAM-AccessLib/         # User-mode DLL source code
├── test-ecam-accesslib/    # Documentation and examples
└── README.md               # Project documentation

## Editing Driver
You are free to use and edit the source code according to your needs. 

