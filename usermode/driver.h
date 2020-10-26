//
// Created by ant on 9/6/2020.
//

#ifndef OREO_LUNAR_UM_DRIVER_H
#define OREO_LUNAR_UM_DRIVER_H

#include "includes.h"
#include "../oreo driver/common.h"

inline bool validate_packet(const oreo_net::packet& packet)
{
    return sizeof(packet) >= sizeof(oreo_net::packet_header) &&
           packet.header.secret == oreo_net::secret;
}

class c_connection
{
    SOCKET socket_connection;

    bool send_packet(
            oreo_net::packet& packet,
            uint32_t& out_result)
    {
        if (send(socket_connection, (const char*)&packet, sizeof(packet), 0) == SOCKET_ERROR)
            return false;

        oreo_net::packet received_packet{ };
        const auto result = recv(socket_connection, (char*)&received_packet, sizeof(oreo_net::packet), 0);

        if (result == SOCKET_ERROR)
            return false;

        if (!validate_packet(received_packet))
            return false;

        if (received_packet.header.type != oreo_net::packet_type::s_request_completed)
            return false;

        out_result = received_packet.data.s_request_completed.return_code;
        return true;
    }

    uint32_t copy_memory(
            unsigned int src_process_id,
            unsigned long long int src_address,
            unsigned int dest_process_id,
            unsigned long long int dest_address,
            unsigned int size)
    {
        oreo_net::packet packet{ };
        packet.header.secret = oreo_net::secret;
        packet.header.type = oreo_net::packet_type::c_copy_memory;

        auto& packet_data = packet.data.c_copy_memory;
        packet_data.src_process_id = src_process_id;
        packet_data.src_address = src_address;
        packet_data.dest_process_id = dest_process_id;
        packet_data.dest_address = dest_address;
        packet_data.size = size;

        uint32_t result = 0;
        if (send_packet(packet, result))
            return result;

        return 999;
    }

public:
    bool bInitialized = false;

    bool Initialize()
    {
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(2, 2), &wsa_data);

        SOCKADDR_IN address{ };
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(oreo_net::host);
        address.sin_port = htons(oreo_net::port);

        const auto connection = socket(AF_INET, SOCK_STREAM, 0);

        if (connection == INVALID_SOCKET)
            return false;

        if (connect(connection, (SOCKADDR*)&address, sizeof(address)) == SOCKET_ERROR)
        {
            closesocket(connection);
            return false;
        }

        socket_connection = connection;
        bInitialized = true;
        return true;
    }

    void Shutdown()
    {
        closesocket(socket_connection);
        WSACleanup();
        bInitialized = false;
    }

    bool click_mouse(
            long screen_x,
            long screen_y,
            unsigned short button_flags)
    {
        oreo_net::packet packet { };
        packet.header.secret = oreo_net::secret;
        packet.header.type = oreo_net::packet_type::c_click_mouse;

        auto& packet_data = packet.data.c_click_mouse;
        packet_data.screen_x = screen_x;
        packet_data.screen_y = screen_y;
        packet_data.button_flags = button_flags;

        uint32_t result = 0;
        if (send_packet(packet, result) &&
            result != SOCKET_ERROR)
                return true;

        return false;
    }

    inline void copy_bytes(
            unsigned int process_id,
            unsigned long long int address,
            unsigned long long int buffer_base,
            unsigned int size)
    {
        copy_memory(process_id, address, GetCurrentProcessId(), buffer_base, size);
    }

    template<typename T>
    inline T read(
            const uint32_t process_id,
            const uintptr_t address)
    {
        T buffer{ };
        copy_memory(process_id, address, GetCurrentProcessId(), uint64_t(&buffer), sizeof(T));
        return buffer;
    }

    template <typename T>
    inline void write(
            const uint32_t process_id,
            const uintptr_t address,
            const T& value)
    {
        copy_memory(GetCurrentProcessId(), uint64_t(&value), process_id, address, sizeof(T));
    }

    uint64_t get_module_base(const uint32_t process_id, std::wstring module_name)
    {
        uint64_t buffer{ };

        oreo_net::packet packet { };
        packet.header.secret = oreo_net::secret;
        packet.header.type = oreo_net::packet_type::c_get_module_base;

        auto& packet_data = packet.data.c_get_module_base;
        packet_data.process_id = process_id;
        packet_data.dest_process_id = GetCurrentProcessId();
        packet_data.dest_address = uint64_t(&buffer);
        packet_data.module_len = module_name.length();

        // Copying the module name into the packet data
        memset(packet_data.module_name, 0x00, 256);
        memcpy(packet_data.module_name, module_name.c_str(), module_name.length() * 2);

        uint32_t result = 0;
        if (send_packet(packet, result) &&
            result != SOCKET_ERROR)
                return buffer;

        return 0;
    }
};

c_connection* g_driver = new c_connection();

#endif //OREO_LUNAR_UM_DRIVER_H
