#include "client.h" //includes sockets & server_shared

static SOCKET create_listen_socket()
{
	SOCKADDR_IN address{ };
	address.sin_family = AF_INET;
	address.sin_port = htons(oreo_net::port);

	const auto listen_socket = socket_listen(AF_INET, SOCK_STREAM, 0);

	if (listen_socket == INVALID_SOCKET)
	{
		DbgPrint("Oreo > Failed to create listen socket \n");
		return INVALID_SOCKET;
	}

	if (bind(listen_socket, (SOCKADDR*)&address, sizeof(address)) == SOCKET_ERROR)
	{
		DbgPrint("Oreo > Failed to bind socket \n");
		closesocket(listen_socket);
		return INVALID_SOCKET;
	}

	if (listen(listen_socket, 10) == SOCKET_ERROR)
	{
		DbgPrint("Oreo > Failed to set socket listening \n");
		closesocket(listen_socket);
		return INVALID_SOCKET;
	}

	return listen_socket;
}

void NTAPI server_thread(void*)
{
	auto status = KsInitialize();
	if (!NT_SUCCESS(status))
	{
		DbgPrint("oreo.software: failed to initialize ksocket \n");
		return;
	}

	const auto listen_socket = create_listen_socket();
	if (listen_socket == INVALID_SOCKET)
	{
		DbgPrint("oreo.software: failed to create listening socket \n");
		KsDestroy();
		return;
	}

	DbgPrint("oreo.software: listening on port %d \n", oreo_net::port);

	while (true)
	{
		sockaddr socket_addr{ };
		socklen_t socket_length{ };

		const auto client_connection = accept(listen_socket, &socket_addr, &socket_length);
		if (client_connection == INVALID_SOCKET)
		{
			DbgPrint("oreo.software: failed to accept connection \n");
			break;
		}

		HANDLE thread_handle = nullptr;

		status = PsCreateSystemThread(
			&thread_handle, 
			GENERIC_ALL, 
			nullptr, 
			nullptr,
			nullptr, 
			client_thread, 
			(void*)client_connection);

		if (!NT_SUCCESS(status))
		{
			DbgPrint("oreo.software: failed to create connection thread \n");
			closesocket(client_connection);
			break;
		}

		ZwClose(thread_handle);
	}

	closesocket(listen_socket);
	KsDestroy();
}