#pragma once

#ifdef DRIVER_DEBUG_MODE
#define DbgOut(x, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[UltraDrv] " x "\n", __VA_ARGS__)
#else
#define DbgOut(x, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[RUST] " x "\n", __VA_ARGS__) //
#endif

#include <ntifs.h>
#include <ntimage.h>
#include <windef.h>

//inline unsigned char jmp_cave[] = { 0x50, 0x48, 0xB8, 0xEF, 0xBE, 0xAD, 0xDE, 0xEF, 0xBE, 0xAD, 0xDE, 0x48, 0x87, 0x04, 0x24, 0xC3 };

#include "native.hpp"
#include "xorstr.hpp"

#include "utils.hpp"
#include "funcs.hpp"

//#include "kaspersky.hpp"
#include "process.hpp"
#include "comms.hpp"
#include "hooks.hpp"

#include "hiding.hpp"
typedef struct _PORT_INFORMATION
{
	ULONG64		MagicValue;		// 0x0000 
	ULONG64		Lock;			// 0x0008
	PVOID		BufferStruct;	// 0x0010
} PORT_INFORMATION, * PPORT_INFORMATION;