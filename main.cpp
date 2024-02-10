#include "hook.h"

extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT driver_object, PUNICODE_STRING reg_path) {
	UNREFERENCED_PARAMETER(driver_object);
	UNREFERENCED_PARAMETER(reg_path);

	DbgPrintEx(0, 0, "Can you hear me?");
	if (CallKernelFunction(HookFunction)) {
		DbgPrintEx(0, 0, "Success.");
	}
	else {
		DbgPrintEx(0, 0, "Failed.");
	}
	return STATUS_SUCCESS;
}