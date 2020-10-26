#pragma once

#include "definitions.h"

//NTSTATUS GetProcNameByPID(ULONG pid, PUNICODE_STRING procName)
//{
//	NTSTATUS status;
//	HANDLE hProcess;
//	PEPROCESS eProcess = NULL;
//	ULONG returnedLength;
//	UNICODE_STRING func;
//	PVOID buffer = NULL;
//	PUNICODE_STRING imageName = NULL;
//
//	if (pid == 0 || procName == NULL)
//		return STATUS_INVALID_PARAMETER;
//
//	if (pid == 4)
//	{
//		RtlInitUnicodeString(&func, L"System");
//		RtlCopyUnicodeString(procName, &func);
//		return STATUS_SUCCESS;
//	}
//
//	status = PsLookupProcessByProcessId((HANDLE)pid, &eProcess);
//	if (!NT_SUCCESS(status))
//		return status;
//
//	status = ObOpenObjectByPointer(eProcess, 0, NULL, 0, 0, KernelMode, &hProcess);
//	if (!NT_SUCCESS(status))
//		return status;
//
//	ObDereferenceObject(eProcess);
//	ZwQueryInformationProcess(hProcess, ProcessImageFileName, NULL, 0, &returnedLength);
//
//	//buffer = ExAllocatePoolWithTag(PagedPool, returnedLength, BUF_POOL_TAG);
//	if (!buffer)
//		return STATUS_NO_MEMORY;
//
//	status = ZwQueryInformationProcess(hProcess, ProcessImageFileName, buffer, returnedLength, &returnedLength);
//	if (NT_SUCCESS(status))
//	{
//		imageName = (PUNICODE_STRING)buffer;
//		if (procName->MaximumLength > imageName->Length)
//			RtlCopyUnicodeString(procName, imageName);
//		else
//			status = STATUS_BUFFER_TOO_SMALL;
//	}
//	ExFreePool(buffer);
//	return status;
//}