#include "utils.h"

extern "C" DRIVER_INITIALIZE DriverEntry;
extern void NTAPI server_thread(void*);

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	DbgPrint("Oreo > Initializing...");

	HANDLE hThreadHandle = nullptr;

	const auto Status = PsCreateSystemThread(
		&hThreadHandle, 
		GENERIC_ALL, 
		nullptr, 
		nullptr, 
		nullptr, 
		server_thread,
		nullptr);

	if (!NT_SUCCESS(Status))
	{
		DbgPrint("Oreo > Failed to create server.");
		return STATUS_SUCCESS;
	}
	else
	{
		DbgPrint("Oreo > Running.");
	}

	ZwClose(hThreadHandle);
	return STATUS_SUCCESS;
}