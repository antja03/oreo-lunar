//
// Created by ant on 9/7/2020.
//

#ifndef OREO_LUNAR_UM_LOG_H
#define OREO_LUNAR_UM_LOG_H

#include "includes.h"

std::mutex Mutex;

void newline()
{
    std::cout << std::endl;
}

template<typename T>
void logType(T data)
{
    std::cout << data << std::endl;
}

void log(std::string data)
{
    std::cout << data;
}

void log_newline(std::string data)
{
    std::cout << data << std::endl;
}

#endif //OREO_LUNAR_UM_LOG_H
