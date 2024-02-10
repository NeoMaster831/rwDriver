#pragma once
#include "def.h"

PVOID KeGetSystemModuleBase(const char* mName) {
	ULONG bytes = 0;
	NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, NULL, bytes, &bytes);
	DbgPrintEx(0, 0, "%x, %d", status, bytes);

	if (!bytes) return NULL;

	PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, bytes, 0xdeadbeef);
	PRTL_PROCESS_MODULE_INFORMATION module_arr = modules->Modules;

	status = ZwQuerySystemInformation(SystemModuleInformation, modules, bytes, &bytes);
	DbgPrintEx(0, 0, "%x", status);

	if (!NT_SUCCESS(status)) return NULL;

	DbgPrintEx(0, 0, "%x", modules->NumberOfModules);

	PVOID module_base = 0;

	for (ULONG i = 0; i < modules->NumberOfModules; i++) {
		DbgPrintEx(0, 0, "%s", module_arr[i].FullPathName);
		if (!strcmp((char*)module_arr[i].FullPathName, mName)) {
			module_base = module_arr[i].ImageBase;
		}
	}

	if (modules) ExFreePoolWithTag(modules, 0xdeadbeef);
	if (module_base <= 0) return NULL;

	return module_base;
}

PVOID KeGetSystemModuleExport(const char* mName, LPCSTR rName) {
	PVOID lp_module = KeGetSystemModuleBase(mName);
	if (!lp_module) return NULL;
	return RtlFindExportedRoutineByName(lp_module, rName);
}

bool WriteMemory(void* addr, void* buf, size_t sz) {
	return RtlCopyMemory(addr, buf, sz);
}

bool WriteMemoryRO(void* addr, void* buf, size_t sz) {
	PMDL mdl = IoAllocateMdl(addr, sz, FALSE, FALSE, NULL);

	if (!mdl) return false;
	
	MmProbeAndLockPages(mdl, KernelMode, IoReadAccess);
	PVOID mapping = MmMapLockedPagesSpecifyCache(mdl, KernelMode, MmNonCached, NULL, FALSE, NormalPagePriority);
	MmProtectMdlSystemAddress(mdl, PAGE_READWRITE);

	WriteMemory(mapping, buf, sz);

	MmUnmapLockedPages(mapping, mdl);
	MmUnlockPages(mdl);
	IoFreeMdl(mdl);

	return true;
}

NTSTATUS KeReadProcMemory(DWORD pid, uintptr_t proc_addr, uintptr_t this_addr, size_t sz) {
	SIZE_T _;
	PEPROCESS src, targ;
	PsLookupProcessByProcessId((HANDLE)pid, &src);
	targ = PsGetCurrentProcess();
	if (!src || !targ) return STATUS_UNSUCCESSFUL;
	__try {
		return MmCopyVirtualMemory(src, (PVOID)proc_addr, targ, (PVOID)this_addr, sz, KernelMode, &_);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return STATUS_ACCESS_DENIED;
	}
}

NTSTATUS KeWriteProcMemory(DWORD pid, uintptr_t proc_addr, uintptr_t buf, size_t sz) {
	SIZE_T _;
	PEPROCESS src, targ;
	PsLookupProcessByProcessId((HANDLE)pid, &targ);
	src = PsGetCurrentProcess();
	if (!src || !targ) return STATUS_UNSUCCESSFUL;
	__try {
		return MmCopyVirtualMemory(src, (PVOID)buf, targ, (PVOID)proc_addr, sz, KernelMode, &_);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return STATUS_ACCESS_DENIED;
	}
}