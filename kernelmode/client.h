#pragma once

#include "common.h"
#include "mouse.h"
#include "sockets.h"

class socket_client
{
public:
	unsigned int process_id { 0 };
	mouse::mouse_object mouse_object { 0 };

	unsigned int handle_set_process_id(oreo_net::data_structures::c_set_process_id& packet)
	{
		this->process_id = packet.process_id;
		return 0;
	}

	// 0: success
	// 1: error
	// :D
	unsigned int handle_click_mouse(oreo_net::data_structures::c_click_mouse& packet)
	{
		UNREFERENCED_PARAMETER(packet);

		if (!mouse_object.callback || !mouse_object.device_object)
			if (!NT_SUCCESS(mouse::initialize_object(&mouse_object)))
				return 1;

		if (!mouse_object.callback || !mouse_object.device_object)
			return 1;

		mouse::invoke_callback(mouse_object, packet.screen_x, packet.screen_y, 0, packet.button_flags);
		return 0;
	}

	unsigned int handle_packet(oreo_net::packet& packet)
	{
		DbgPrint("oreo.software: packet received (type: %d) \n", packet.header.type);

		switch (packet.header.type)
		{

		case oreo_net::packet_type::c_packet_close:
			return 999;
		case oreo_net::packet_type::c_set_process_id:
			return handle_set_process_id(packet.data.c_set_process_id);
		case oreo_net::packet_type::c_click_mouse:
			return handle_click_mouse(packet.data.c_click_mouse);
		default:
			return 999;
		}
	}
};

static void NTAPI client_thread(void* connection_socket)
{
	DbgPrint("oreo.software: new connection made");

	const auto client_connection = SOCKET(ULONG_PTR(connection_socket));
	auto client = socket_client();
	oreo_net::packet packet{ };

	while (true)
	{
		const auto receive_result = recv(client_connection, (void*)&packet, sizeof(packet), 0);

		if (receive_result <= 0)
			break;

		if (receive_result < sizeof(oreo_net::packet_header))
			return;

		if (packet.header.secret != oreo_net::secret)
			continue;

		const auto handle_result = client.handle_packet(packet);

		if (handle_result == 999)
			break;

		// request has been handled, let the client know
		oreo_net::packet complete_packet{ };
		complete_packet.header.secret = oreo_net::secret;
		complete_packet.header.type = oreo_net::packet_type::s_request_completed;
		complete_packet.data.s_request_completed.return_code = handle_result;
		if (send(client_connection, &complete_packet, sizeof(complete_packet), 0) == SOCKET_ERROR)
			break;
	}

	DbgPrint("oreo.software: connection closed");
	closesocket(client_connection);
}