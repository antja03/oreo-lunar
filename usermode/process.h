//
// Created by ant on 9/7/2020.
//

#ifndef OREO_LUNAR_UM_PROCESS_H
#define OREO_LUNAR_UM_PROCESS_H

#include "includes.h"

class c_process
{
public:
    DWORD dwProcessId = -1;

    bool IsInitialized()
    {
        return dwProcessId != -1;
    }

    bool Initialize()
    {
        PROCESSENTRY32W proc;
        proc.dwSize = sizeof(PROCESSENTRY32W);
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        dwProcessId = -1;

        if (hSnap == INVALID_HANDLE_VALUE)
            return false;

        if (Process32FirstW(hSnap, &proc)) {
            do {
                if (wcscmp(L"AlphaAntiLeak.exe", proc.szExeFile) == 0)
                {
                    CloseHandle(hSnap);
                    dwProcessId = proc.th32ProcessID;
                    std::cout << "Process ID: " << dwProcessId << std::endl;
                    return true;
                }
            } while (Process32NextW(hSnap, &proc));
        }

        CloseHandle(hSnap);
        return false;
    }
};

c_process* g_process = new c_process();

#endif //OREO_LUNAR_UM_PROCESS_H
