#include <random>

#include "constants.h"
#

bool running = true;
bool hold_to_click = false;
bool clicking = false;
int cps = 10;
int minimum_input_delay_ms = 5;
int maximum_input_delay_ms = 15;

inline void print_header()
{
    system("cls");

    std::cout << R"(
                                      ______
  ____  ________  ____    _________  / __/ /__      ______ _________
 / __ \/ ___/ _ \/ __ \  / ___/ __ \/ /_/ __/ | /| / / __ `/ ___/ _ \
/ /_/ / /  /  __/ /_/ / (__  ) /_/ / __/ /_ | |/ |/ / /_/ / /  /  __/
\____/_/   \___/\____(_)____/\____/_/  \__/ |__/|__/\__,_/_/   \___/
                                          Lunar Client Auto-Clicker
--------------------------------------------------------------------------
)" << std::endl;
}

void update_console()
{
    print_header();
    log_newline("BINDS");
    log_newline("--------------------------");
    log_newline("Bind mode: L-Ctrl + Right");
    log_newline("Increase CPS: L-Ctrl + Up");
    log_newline("Decrease CPS: L-Ctrl + Down");
    log_newline("Click: Mouse 5");
    newline();
    newline();
    log_newline("STATUS");
    log_newline("--------------------------");
    log("Click mode: ");
    log_newline(hold_to_click ? "Hold" : "Toggle");
    log("Clicking: ");
    log_newline(clicking ? "Yes" : "No");
    log("CPS: ");
    log_newline(std::to_string(cps));
}

int main()
{
    print_header();

    if (!g_driver->Initialize())
    {
        log_newline("Failed to connect to driver");
        system("pause");
        return 0;
    }
    else
    {
        log_newline("Connected to driver");
    }

    // Constant pool scanner broken atm, working on fixing
    //
    //    if (!g_process->Initialize())
    //    {
    //        log_newline("Couldn't find AAL process... exiting");
    //        system("pause");
    //        return 0;
    //    }
    //
    //    if (!gConstants->UpdateConstantPoolAddress())
    //    {
    //        log_newline("Couldn't find constant pool... exiting");
    //        system("pause");
    //        return 0;
    //    }

    update_console();

    // Stack over flow best :D
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(minimum_input_delay_ms, maximum_input_delay_ms);

    int last_click_ms = 0;
    while (running)
    {
        if (GetAsyncKeyState(VK_LCONTROL))
        {
            // SWITCH CLICK MODE
            if (GetAsyncKeyState(VK_RIGHT) & 1)
            {
                hold_to_click = !hold_to_click;
                update_console();
                goto update_settings;
            }

            // INCREASE CPS
            if (GetAsyncKeyState(VK_UP) & 1)
            {
                cps++;
                update_console();
                goto update_settings;
            }

            // DECREASE CPS
            if (GetAsyncKeyState(VK_DOWN) & 1)
            {
                cps--;
                goto update_settings;
            }

            goto skip_settings;
         
        update_settings:
            update_console();
            goto finish;
        }

    skip_settings:
        // Toggle clicking
        if (!hold_to_click && GetAsyncKeyState(VK_XBUTTON2) & 1)
        {
            clicking = !clicking;
            update_console();
            goto finish;
        }

        if ((hold_to_click && GetAsyncKeyState(VK_XBUTTON2)) || clicking)
        {
            int current_ms = GetTickCount();

            if (current_ms - last_click_ms < 50)
                goto finish;

            if (current_ms - last_click_ms >= 1000 / cps)
            {
                g_driver->click_mouse(0, 0, 0x1);
                Sleep(distribution(generator));
                g_driver->click_mouse(0, 0, 0x2);

                last_click_ms = current_ms;
            }
        }

    finish:
        // Save processing power grrr
        Sleep(5);
    }

    system("pause");
    return 0;
}