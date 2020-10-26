//
// Created by ant on 9/8/2020.
//

#ifndef OREO_LUNAR_UM_COLLECTION_H
#define OREO_LUNAR_UM_COLLECTION_H

#include <algorithm>
#include <iterator>

template <class T, size_t N>
inline bool ArrayContainsAddress(T (&Array)[N], uint16_t Value)
{
    return std::find(std::begin(Array), std::end(Array), Value) != std::end(Array);
}


#endif
