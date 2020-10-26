//
// Created by ant on 9/7/2020.
//

#ifndef OREO_LUNAR_UM_LOG_H
#define OREO_LUNAR_UM_LOG_H

#include "includes.h"

std::mutex Mutex;

template<typename T>
void logType(T data)
{
    std::cout << data << std::endl;
}

void log(const char* data)
{
    std::cout << data << std::endl;
}

#endif //OREO_LUNAR_UM_LOG_H
