#pragma once

inline ULONG_PTR DYN_PTE_BASE = 0;
inline ULONG_PTR DYN_PDE_BASE = 0;


inline PHANDLE_TABLE* pPspCidTable = nullptr;
inline f_ExpLookupHandleTableEntry ExpLookupHandleTableEntry = nullptr;

namespace hide
{
	inline void UnlinkFromListEntry(LIST_ENTRY* entry)
	{
		const auto prev = entry->Blink;
		const auto next = entry->Flink;
		prev->Flink = next;
		next->Blink = prev;

		entry->Flink = 0;
		entry->Blink = 0;
	}

	inline PMMPTE GetPTEForVA(IN PVOID pAddress)
	{
		PMMPTE pPDE = (PMMPTE)(((((ULONG_PTR)pAddress >> PDI_SHIFT) << PTE_SHIFT) & 0x3FFFFFF8ull) + DYN_PDE_BASE);
		if (pPDE->u.Hard.LargePage)
			return pPDE;

		return (PMMPTE)(((((ULONG_PTR)pAddress >> PTI_SHIFT) << PTE_SHIFT) & 0x7FFFFFFFF8ull) + DYN_PTE_BASE);
	}

	inline bool ClearPiDDBCache(const ULONG dwTimeDateStamp)
	{
		if (!PiDDBCacheTbl)
			return false;

		auto entry = PIDBB_CACHE_ENTRY{};
		entry.TimeDateStamp = dwTimeDateStamp;

		const auto list_entry = reinterpret_cast<PIDBB_CACHE_ENTRY*>(RtlLookupElementGenericTableAvl(PiDDBCacheTbl, &entry));
		if (!list_entry)
			return false;

		UnlinkFromListEntry(&list_entry->List);
		return (RtlDeleteElementGenericTableAvl(PiDDBCacheTbl, list_entry) == TRUE);
	}

	inline bool NullPFN(uintptr_t address, ULONG size)
	{
		PMDL pMDL = IoAllocateMdl((PVOID)address, size, FALSE, FALSE, NULL);

		if (!pMDL)
		{
			return false;
		}

		PPFN_NUMBER mdlPages = MmGetMdlPfnArray(pMDL);
		if (!mdlPages)
		{
			return false;
		}

		ULONG mdlPageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES(MmGetMdlVirtualAddress(pMDL), MmGetMdlByteCount(pMDL));

		ULONG nullPfn = 0x0;
		MM_COPY_ADDRESS sourceAddress = { 0 };
		sourceAddress.VirtualAddress = &nullPfn;

		for (ULONG i = 0; i < mdlPageCount; i++)
		{
			size_t bytes = 0;
			MmCopyMemory(&mdlPages[i], sourceAddress, sizeof(ULONG), MM_COPY_MEMORY_VIRTUAL, &bytes);
		}
		return true;
	}

	inline void NTAPI UInitialize()
	{
		KDDEBUGGER_DATA64 debuggerDataBlock = { 0 };
		KDDEBUGGER_DATA_ADDITION64 debuggerDataAddBlock = { 0 };

		CONTEXT Context = { 0 };
		PDUMP_HEADER DumpHeader = NULL;
		PKDDEBUGGER_DATA64 KdDebuggerDataBlock = NULL;
		PKDDEBUGGER_DATA_ADDITION64 KdDebuggerDataAdditionBlock = NULL;

		Context.ContextFlags = CONTEXT_FULL;
		RtlCaptureContext(&Context);
		DumpHeader = (PDUMP_HEADER)ExAllocatePool(NonPagedPool, DUMP_BLOCK_SIZE);
		if (NULL != DumpHeader)
		{
			KeCapturePersistentThreadState(&Context, NULL, 0, 0, 0, 0, 0, DumpHeader);
			KdDebuggerDataBlock = (PKDDEBUGGER_DATA64)((PUCHAR)DumpHeader + KDDEBUGGER_DATA_OFFSET);
			RtlCopyMemory(&debuggerDataBlock, KdDebuggerDataBlock, sizeof(KDDEBUGGER_DATA64));
			KdDebuggerDataAdditionBlock = (PKDDEBUGGER_DATA_ADDITION64)(KdDebuggerDataBlock + 1);
			RtlCopyMemory(&debuggerDataAddBlock, KdDebuggerDataAdditionBlock, sizeof(KDDEBUGGER_DATA_ADDITION64));

			ExFreePool(DumpHeader);
		}

		ULONGLONG mask = (1ll << (PHYSICAL_ADDRESS_BITS - 1)) - 1;
		DYN_PTE_BASE = (ULONG_PTR)debuggerDataAddBlock.PteBase;
		DYN_PDE_BASE = (ULONG_PTR)((debuggerDataAddBlock.PteBase & ~mask) | ((debuggerDataAddBlock.PteBase >> 9) & mask));
	}

	inline SIZE_T StartAddress = NULL;

	inline HANDLE CreateThread(PKSTART_ROUTINE Routine, PVOID Param = nullptr)
	{
		HANDLE hThread = nullptr;
		PsCreateSystemThread(&hThread, GENERIC_ALL, nullptr, nullptr, nullptr, PKSTART_ROUTINE(Routine), Param);

		return hThread;
	}

	inline HANDLE CreateThreadSpoofed(PVOID StartRoutine)
	{
		if (!StartRoutine)
			return NULL;

		if (!StartAddress)
		{
			LARGE_INTEGER li{ };
			KeQueryTickCount(&li);
			const auto val = 1 + (RtlRandomEx(&li.LowPart) % INT_MAX);

			if (val % 2)
				StartAddress = utils::FindPattern(XS("ntoskrnl.exe"), XS("PAGE"), PBYTE("\xFF\xE1"), XS("xx"));
			else
				StartAddress = utils::FindPattern(XS("ntoskrnl.exe"), XS(".text"), PBYTE("\xFF\xE1"), XS("xx"));
		}

		if (!StartAddress)
		{
			DbgOut("Failed to find a address to spoof thread!");
			return NULL;
		}

		HANDLE hThread = nullptr;
		auto status = PsCreateSystemThread(&hThread, GENERIC_ALL, nullptr, nullptr, nullptr, PKSTART_ROUTINE(StartAddress), StartRoutine);
		return status ? hThread : NULL;
	}

	inline UCHAR GetMiscFlagsOffset()
	{
		static UCHAR offset = 0;

		if (!offset)
		{
			auto addr = PUCHAR(&PsIsSystemThread);

			offset = *reinterpret_cast<PUCHAR>(addr + 2);
		}
		return offset;
	}
	inline bool HideThreadPspCidTable(HANDLE Handle, PCID_TABLE_HIDDEN_THREAD Data)
	{
		if (!pPspCidTable || !ExpLookupHandleTableEntry || !Data)
			return false;

		auto Entry = ExpLookupHandleTableEntry(*pPspCidTable, Handle);

		if (!Entry)
			return false;

		PETHREAD dummyThread = nullptr;
		HANDLE DummyThreadId = nullptr;

		for (uintptr_t i = 0x100; i < 0x3000; i += 4)
		{
			if (NT_SUCCESS(PsLookupThreadByThreadId(reinterpret_cast<HANDLE>(i), &dummyThread)))
			{
				ObDereferenceObject(dummyThread);

				if (reinterpret_cast<HANDLE>(i) != reinterpret_cast<HANDLE>(Handle) && PsIsSystemThread(dummyThread))
				{
					DummyThreadId = reinterpret_cast<HANDLE>(i);
					break;
				}
			}

		}

		if (!DummyThreadId)
			return false;

		auto pobject_header = reinterpret_cast<POBJECT_HEADER>(reinterpret_cast<uintptr_t>(dummyThread) - sizeof(OBJECT_HEADER));

		auto pdummy_thread = reinterpret_cast<POBJECT_HEADER>(ExAllocatePoolWithTag(NonPagedPoolNx, 0x1000, 0x65726854));

		if (!pdummy_thread)
			return false;

		memcpy(pdummy_thread, pobject_header, 0x1000);

		pdummy_thread->HandleCount = 6334;
		pdummy_thread->PointerCount = 6334;

		auto Entry2 = ExpLookupHandleTableEntry(*pPspCidTable, DummyThreadId);

		if (!Entry2)
		{
			ExFreePoolWithTag(pdummy_thread, 0x65726854);
			return false;
		}

		HANDLE_TABLE_ENTRY entry = { 0 };

		memcpy(&Data->OldEntry, Entry, sizeof(HANDLE_TABLE_ENTRY));
		memcpy(&entry, Entry2, sizeof(HANDLE_TABLE_ENTRY));

		entry.ObjectPointerBits = (reinterpret_cast<INT64>(pdummy_thread) + sizeof(OBJECT_HEADER)) >> 4;

		// PsIsSystemThread will fail
		{
			auto BitFlag = reinterpret_cast<PDWORD>(PUCHAR(PsGetCurrentThread()) + GetMiscFlagsOffset());
			*BitFlag &= ~(1 << 10);
		}

		memcpy(Entry, &entry, sizeof(HANDLE_TABLE_ENTRY));

		Data->DummyEThread = pdummy_thread;

		return true;
	}

	inline void RestoreThreadHandlePspCidTable(HANDLE Handle, PCID_TABLE_HIDDEN_THREAD Data)
	{
		if (!pPspCidTable || !ExpLookupHandleTableEntry || !Data)
			return;

		auto Entry = ExpLookupHandleTableEntry(*pPspCidTable, Handle);

		if (!Entry)
			return;

		memcpy(Entry, &Data->OldEntry, sizeof(HANDLE_TABLE_ENTRY));

		ExFreePoolWithTag(Data->DummyEThread, 0x65726854);
	}
}