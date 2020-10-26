#pragma once

#ifndef __ntapi
#define __ntapi
#include <ntifs.h>
#include <ntddmou.h>
#endif

#ifndef __functions
#define __functions

extern "C"
{

	NTKERNELAPI NTSTATUS MmCopyVirtualMemory(
		IN PEPROCESS		SourceProcess,
		IN PVOID			SourceAddress,
		IN PEPROCESS		TargetProcess,
		IN PVOID			TargetAddress,
		IN SIZE_T			BufferSize,
		IN KPROCESSOR_MODE  PreviousMode,
		OUT PSIZE_T			ReturnSize
	);

	NTKERNELAPI NTSTATUS PsLookupProcessByProcessId(
		IN HANDLE	   ProcessId,
		OUT PEPROCESS* Process
	);

	NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(
		IN PEPROCESS Process
	);

	PPEB PsGetProcessPeb(
		IN PEPROCESS Process
	);

	NTSYSAPI NTSTATUS NTAPI NtWriteVirtualMemory(
		IN HANDLE               ProcessHandle,
		IN PVOID                BaseAddress,
		IN PVOID                Buffer,
		IN ULONG                NumberOfBytesToWrite,
		OUT PULONG              NumberOfBytesWritten OPTIONAL);

	NTSYSAPI NTSTATUS NTAPI ObReferenceObjectByName(
		_In_ PUNICODE_STRING		ObjectName,
		_In_ ULONG					Attributes,
		_In_opt_ PACCESS_STATE		AccessState,
		_In_opt_ ACCESS_MASK		DesiredAccess,
		_In_ POBJECT_TYPE			ObjectType,
		_In_ KPROCESSOR_MODE		AccessMode,
		_Inout_opt_ PVOID			ParseContext,
		_Out_ PVOID* OBJECT);
}

#endif