#include "definitions.h"
#include "common.h"
#include "sockets.h"

using namespace oreo_net;

static unsigned int handle_copy_memory(const data_structures::c_copy_memory& packet)
{
	DbgPrint("oreo.software: handling copy_memory (source address: %lu, destination address: %lu) \n", packet.src_address, packet.dest_address);

	PEPROCESS dest_process = nullptr;
	PEPROCESS src_process = nullptr;

	if (!NT_SUCCESS(PsLookupProcessByProcessId(HANDLE(packet.dest_process_id), &dest_process)))
		return 1;

	if (!NT_SUCCESS(PsLookupProcessByProcessId(HANDLE(packet.src_process_id), &src_process)))
	{
		ObDereferenceObject(dest_process);
		return 1;
	}

	SIZE_T return_size = 0;
	auto status = MmCopyVirtualMemory(
		src_process,
		(void*)packet.src_address,
		dest_process,
		(void*)packet.dest_address,
		packet.size,
		UserMode,
		&return_size
	);

	ObDereferenceObject(dest_process);
	ObDereferenceObject(src_process);

	return UINT64(status);
}

static unsigned int handle_get_module_base_address(const data_structures::c_get_module_base& packet)
{
	DbgPrint("oreo.software: handling get_module_base_address \n");
	DbgPrint("oreo.software: process id %d \n", packet.process_id);
	DbgPrint("oreo.software: module name %ws \n", packet.module_name);
	DbgPrint("oreo.software: process id %d \n", packet.dest_process_id);
	DbgPrint("oreo.software: process id %llu \n", packet.dest_address);

	UNICODE_STRING module_name;
	RtlInitUnicodeString(&module_name, packet.module_name);

	PEPROCESS src_process = nullptr;
	PEPROCESS dest_process = nullptr;

	if (!NT_SUCCESS(PsLookupProcessByProcessId(HANDLE(packet.process_id), &src_process)))
		return UINT64(STATUS_SUCCESS);

	if (!NT_SUCCESS(PsLookupProcessByProcessId(HANDLE(packet.dest_process_id), &dest_process)))
		return UINT64(STATUS_SUCCESS);

	PPEB peb = PsGetProcessPeb(src_process);
	if (!peb)
	{
		DbgPrint("oreo.software: couldn't get process peb");
		ObDereferenceObject(src_process);
		ObDereferenceObject(dest_process);
		return UINT64(STATUS_SUCCESS);
	}

	KAPC_STATE state;
	KeStackAttachProcess(src_process, &state);

	PPEB_LDR_DATA ldr = peb->Ldr;
	if (!ldr)
	{
		DbgPrint("oreo.software: ldr is invalid");
		ObDereferenceObject(src_process);
		ObDereferenceObject(dest_process);
		1;
	}

	if (!ldr->Initialized)
	{
		DbgPrint("oreo.software: ldr not initialized");
		ObDereferenceObject(src_process);
		ObDereferenceObject(dest_process);
		return 1;
	}

	for (PLIST_ENTRY list_entry = (PLIST_ENTRY)ldr->InLoadOrderModuleList.Flink;
		list_entry != &ldr->InLoadOrderModuleList;
		list_entry = (PLIST_ENTRY)list_entry->Flink) 
	{
		PLDR_DATA_TABLE_ENTRY ldr_entry = CONTAINING_RECORD(list_entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

		if (!RtlCompareUnicodeString(&ldr_entry->BaseDllName, &module_name, true))
		{
			DbgPrint("oreo.software: module base is %llu", ldr_entry->DllBase);

			SIZE_T return_size = 0;
			auto status = MmCopyVirtualMemory(
				src_process,
				(void*) &ldr_entry->DllBase,
				dest_process,
				(void*)packet.dest_address,
				sizeof(ldr_entry->DllBase),
				UserMode,
				&return_size
			);

			KeUnstackDetachProcess(&state);
			ObDereferenceObject(src_process);
			ObDereferenceObject(dest_process);
			return status;
		}
	}

	KeUnstackDetachProcess(&state);
	ObDereferenceObject(src_process);
	ObDereferenceObject(dest_process);
	return STATUS_SUCCESS;
}

unsigned int handle_incoming_packet(const packet& packet)
{
	DbgPrint("oreo.software: packet received (type: %d) \n", packet.header.type);

	switch (packet.header.type)
	{
	case packet_type::c_copy_memory:
		return handle_copy_memory(packet.data.c_copy_memory);
	case packet_type::c_get_module_base:
		return handle_get_module_base_address(packet.data.c_get_module_base);
	default:
		break;
	}

	return 999;
}

bool complete_request(const SOCKET client_connection, const unsigned int result)
{
	packet packet{ };
	packet.header.secret = secret;
	packet.header.type = packet_type::c_packet_close;
	packet.data.s_request_completed.return_code = result;

	return send(client_connection, &packet, sizeof(packet), 0) != SOCKET_ERROR;
}