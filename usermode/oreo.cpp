#include <random>

#include "constants.h"

bool running = true;
bool clicking = false;
int cps = 10;
int minimum_input_delay_ms = 5;
int maximum_input_delay_ms = 20;

inline void print_ascii_art()
{
    system("cls");

    std::cout << R"(
                                      ______
  ____  ________  ____    _________  / __/ /__      ______ _________
 / __ \/ ___/ _ \/ __ \  / ___/ __ \/ /_/ __/ | /| / / __ `/ ___/ _ \
/ /_/ / /  /  __/ /_/ / (__  ) /_/ / __/ /_ | |/ |/ / /_/ / /  /  __/
\____/_/   \___/\____(_)____/\____/_/  \__/ |__/|__/\__,_/_/   \___/

--------------------------------------------------------------------------
)" << std::endl;
}

void update_console()
{
    print_ascii_art();
    std::cout <<
        "Clicking: " << clicking << std::endl <<
        "Your CPS is: " << cps << std::endl <<
        "Your minimum delay time is: " << minimum_input_delay_ms << std::endl <<
        "Your maximum delay time is: " << maximum_input_delay_ms << std::endl;
}

int main()
{
    print_ascii_art();

    if (!g_driver->Initialize())
    {
        log("Failed to connect to driver");
        system("pause");
        return 0;
    }
    else
    {
        log("Connected to driver");
    }

    //    if (!g_process->Initialize())
    //    {
    //        log("Couldn't find AAL process... exiting");
    //        system("pause");
    //        return 0;
    //    }
    //
    //    if (!gConstants->UpdateConstantPoolAddress())
    //    {
    //        log("Couldn't find constant pool... exiting");
    //        system("pause");
    //        return 0;
    //    }

    update_console();

    int last_click_ms = 0;
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(minimum_input_delay_ms, maximum_input_delay_ms);

    while (running)
    {
        // TOGGLE CLICKING :D
        if (GetAsyncKeyState(VK_XBUTTON2) & 1)
        {
            clicking = !clicking;
            update_console();
            goto complete;
        }

        // INCREASE CPS
        if (GetAsyncKeyState(VK_UP) & 1 && GetAsyncKeyState(VK_LCONTROL))
        {
            cps++;
            update_console();
            goto complete;
        }

        // DECREASE CPS
        if (GetAsyncKeyState(VK_DOWN) & 1 && GetAsyncKeyState(VK_LCONTROL))
        {
            cps--;
            update_console();
            goto complete;
        }

        if (GetAsyncKeyState(VK_XBUTTON2))
        {
            int current_ms = GetTickCount();

            if (current_ms - last_click_ms < 50)
                goto complete;

            if (current_ms - last_click_ms >= 1000 / cps)
            {
                g_driver->click_mouse(0, 0, 0x1);
                Sleep(distribution(generator));
                g_driver->click_mouse(0, 0, 0x2);

                last_click_ms = current_ms;
            }
        }

    complete:

        // Save processing power grrr
        Sleep(5);
    }

    system("pause");
    return 0;
}