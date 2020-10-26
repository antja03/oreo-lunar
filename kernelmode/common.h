#pragma once

//DRIVER SHARED

namespace oreo_net
{
    namespace return_codes
    {
        constexpr auto success = 0x00;
    }

    namespace data_structures
    {
        namespace server
        {
            struct request_completed
            {
                unsigned int return_code;
            };
        }

        struct c_set_process_id
        {
            unsigned int process_id;
        };

        struct c_get_region_info
        {
            unsigned int process_id;
            unsigned long long int address;

            unsigned int client_proc_id;
        };

        struct c_copy_memory
        {
            unsigned int dest_process_id;
            unsigned long long int dest_address;

            unsigned int src_process_id;
            unsigned long long int src_address;

            unsigned int size;
        };

        struct c_get_module_base
        {
            unsigned int process_id;
            unsigned short module_name[256];
            unsigned int module_len;
            unsigned long long dest_process_id;
            unsigned long long int dest_address;
        };

        struct c_click_mouse
        {
            long screen_x;
            long screen_y;
            unsigned short button_flags;
        };
    }

    constexpr auto secret = 0x12345568;
    constexpr auto host = 0x7F000001; // 127.0.0.1
    constexpr auto port = 29132;

    enum class packet_type
    {
        s_request_completed,

        c_click_mouse,
        c_copy_memory,
        c_get_module_base,

        c_set_process_id,
        c_get_region_info,
        c_packet_close,
        packet_get_module_base_address,

    };

    struct PacketGetBaseAddress
    {
        unsigned int process_id;
    };

    struct PacketCleanPiDDBCacheTable {};

    struct PacketCleanMMUnloadedDrivers {};

    struct PacketSpoofDrives {};

    struct PacketCompleted {
        unsigned long long int result;
    };

    struct packet_header
    {
        unsigned int secret;
        packet_type type;
    };

    struct packet
    {
        packet_header header;

        union
        {
            data_structures::c_click_mouse c_click_mouse;
            data_structures::c_set_process_id c_set_process_id;
            data_structures::c_get_module_base c_get_module_base;
            data_structures::c_get_region_info c_get_region_info;
            data_structures::c_copy_memory c_copy_memory;

            data_structures::server::request_completed s_request_completed;
        } data;
    };
}