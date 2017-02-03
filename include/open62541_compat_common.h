/*
 * common.h
 *
 *  Created on: Jan 26, 2017
 *      Author: pnikiel
 */

#ifndef OPEN62541_COMPAT_INCLUDE_OPEN62541_COMPAT_COMMON_H_
#define OPEN62541_COMPAT_INCLUDE_OPEN62541_COMPAT_COMMON_H_

#include <stdexcept>

class alloc_error: public std::runtime_error
{
public:
    alloc_error(): std::runtime_error("memory allocation exception") {}
};


#endif /* OPEN62541_COMPAT_INCLUDE_OPEN62541_COMPAT_COMMON_H_ */
