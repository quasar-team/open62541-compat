/*
 * logit_logger.h
 *
 *  Created on: May 20, 2021
 *      Author: pnikiel
 */

#ifndef INCLUDE_LOGIT_LOGGER_H_
#define INCLUDE_LOGIT_LOGGER_H_

#include <LogIt.h>
#include <open62541.h>

//! Can be safely run many times, does anything only if the component does not exist yet.
void initializeOpen62541LogIt (Log::LOG_LEVEL logLevel = Log::INF);

extern UA_Logger theLogItLogger;

#endif /* INCLUDE_LOGIT_LOGGER_H_ */
