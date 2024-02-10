#pragma once
#include "mem.h"

BYTE original[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
BYTE payload[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
PVOID* function;

NTSTATUS(*origFunc)(HANDLE _1, uintptr_t _2);

bool CallKernelFunction(void* addr) {
	if (!addr) return false;
	function = reinterpret_cast<PVOID*>(KeGetSystemModuleExport(
		"\\SystemRoot\\System32\\drivers\\dxgkrnl.sys",
		"NtQueryCompositionSurfaceStatistics"));
	DbgPrintEx(0, 0, "%p", function);
	origFunc = (NTSTATUS(*)(HANDLE _1, uintptr_t _2))function;

	if (!function) return false;
	memcpy((PVOID)((ULONG_PTR)original), function, sizeof(original));

	BYTE sc_s[] = { 0x48, 0xB8 };
	BYTE sc_e[] = { 0xFF, 0xE0 };

	RtlSecureZeroMemory(&payload, sizeof(payload));
	memcpy((PVOID)((ULONG_PTR)payload), &sc_s, sizeof(sc_s));
	uintptr_t hook_addr = reinterpret_cast<uintptr_t>(addr);
	memcpy((PVOID)((ULONG_PTR)payload + sizeof(sc_s)), &hook_addr, sizeof(void*));
	memcpy((PVOID)((ULONG_PTR)payload + sizeof(sc_s) + sizeof(void*)), &sc_e, sizeof(sc_e));
	WriteMemoryRO(function, &payload, sizeof(payload));

	return true;
}

NTSTATUS HookFunction(
	HANDLE a1,
	uintptr_t a2
) {

	DbgPrintEx(0, 0, "Hook Called!");
	auto cvrt = (PUSERMODE_COMMAND)a1;
	if (!MmIsAddressValid(a1) || cvrt->magic != MAGIC) {
		WriteMemoryRO(function, &original, sizeof(original));
		auto ret = origFunc(a1, a2);
		WriteMemoryRO(function, &payload, sizeof(payload));
		DbgPrintEx(0, 0, "Return");
		return ret;
	}
	 
	DbgPrintEx(0, 0, "It is MY FUCTION!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! %p", a1);
	DbgPrintEx(0, 0, "HookFunction: %p", HookFunction);

	if (cvrt->mode == Read) { // Return: Read Data
		auto args = (PREAD_CMD_ARGS)(cvrt->arg);
		return KeReadProcMemory(args->pid, args->wh, (uintptr_t)(cvrt->retrn), args->sz);
	}
	else if (cvrt->mode == Write) { // No Return
		auto args = (PWRITE_CMD_ARGS)(cvrt->arg);
		return KeWriteProcMemory(args->pid, args->wh, (uintptr_t)(args->what), args->sz);
	}
	
	return STATUS_SUCCESS;
}