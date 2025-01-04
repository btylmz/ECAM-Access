#pragma once

#include <windows.h>

#pragma pack(push, 1)
struct MCFGHeader {
    DWORD Signature;          // 'MCFG'
    DWORD Length;             // Total length of the table
    BYTE Revision;
    BYTE Checksum;
    BYTE OEMID[6];
    BYTE OEMTableID[8];
    DWORD OEMRevision;
    DWORD CreatorID;
    DWORD CreatorRevision;
    BYTE Reserved[8];
};

struct MCFGEntry {
    UINT64 BaseAddress;       // ECAM Base Address
    UINT16 SegmentGroup;      // Segment group number
    UINT8 StartBusNumber;     // Starting bus number
    UINT8 EndBusNumber;       // Ending bus number
    UINT32 Reserved;          // Reserved, should be zero
};
#pragma pack(pop)

extern "C"
{
    /// <summary>
	/// Retrieve the ECAM base address from the MCFG table. Currently works with systems that have a single PCI bus segment group.
    /// </summary>
    /// <returns></returns>
    UINT64 GetECAMBaseAddress();
}