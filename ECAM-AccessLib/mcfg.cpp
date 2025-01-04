#include "mcfg.h"
#include <iostream>
#include <windows.h>


UINT64 GetECAMBaseAddress()
{
    // Get the size of the firmware table names
    DWORD tableSize = EnumSystemFirmwareTables('ACPI', NULL, 0);
    if (tableSize == 0) {
        std::cerr << "Failed to enumerate firmware tables.\n";
        return -1;
    }

    // Get the firmware table names
    char* tableNames = new char[tableSize];
    EnumSystemFirmwareTables('ACPI', tableNames, tableSize);

    // Look for the "MCFG" table signature
    bool foundMCFG = false;
    for (DWORD i = 0; i < tableSize; i += 4) {
        if (memcmp(&tableNames[i], "MCFG", 4) == 0) {
            foundMCFG = true;
            break;
        }
    }

    delete[] tableNames;

    if (!foundMCFG) {
        std::cerr << "MCFG table not found.\n";
        return -1;
    }

    // Retrieve the MCFG table size
    DWORD mcfgSize = GetSystemFirmwareTable('ACPI', *(DWORD*)"MCFG", NULL, 0);
    if (mcfgSize == 0) {
        std::cerr << "Failed to get MCFG table size.\n";
        return -1;
    }

    // Allocate buffer for the MCFG table
    BYTE* mcfgTable = new BYTE[mcfgSize];
    if (GetSystemFirmwareTable('ACPI', *(DWORD*)"MCFG", mcfgTable, mcfgSize) == 0) {
        std::cerr << "Failed to retrieve MCFG table.\n";
        delete[] mcfgTable;
        return -1;
    }

    // Parse the MCFG header
    MCFGHeader* header = reinterpret_cast<MCFGHeader*>(mcfgTable);
    DWORD entryCount = (header->Length - sizeof(MCFGHeader)) / sizeof(MCFGEntry);

    // Parse each MCFG entry and print the ECAM base information
    MCFGEntry* entries = reinterpret_cast<MCFGEntry*>(mcfgTable + sizeof(MCFGHeader));

    // Return the ECAM base address
    UINT64 baseAddress = entries[0].BaseAddress;

    // Clean up
    delete[] mcfgTable;

	return baseAddress;
}