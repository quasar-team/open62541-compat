/*
 * logit_logger.cpp
 *
 *  Created on: May 20, 2021
 *      Author: pnikiel
 */

#include <logit_logger.h>
#include <LogIt.h>

static Log::LogComponentHandle logItComponentHandle = Log::INVALID_HANDLE;

void initializeOpen62541LogIt (Log::LOG_LEVEL logLevel)
{
	// register LogIt component for open62541
	Log::LogComponentHandle handle = Log::getComponentHandle("open62541");
	if (handle == Log::INVALID_HANDLE)
	{
		logItComponentHandle = Log::registerLoggingComponent("open62541", logLevel);
	}
}

static void logFromOpen62541 (
		void *logContext,
		UA_LogLevel level,
		UA_LogCategory category,
        const char *msg,
		va_list args)
{
	// translate open62541 log level into LogIt log level
	Log::LOG_LEVEL logItLogLevel;
	switch (level)
	{
		case UA_LOGLEVEL_DEBUG: logItLogLevel = Log::DBG; break;
		case UA_LOGLEVEL_ERROR: logItLogLevel = Log::ERR; break;
		case UA_LOGLEVEL_FATAL: logItLogLevel = Log::ERR; break;
		case UA_LOGLEVEL_INFO: logItLogLevel = Log::INF; break;
		case UA_LOGLEVEL_TRACE: logItLogLevel = Log::TRC; break;
		case UA_LOGLEVEL_WARNING: logItLogLevel = Log::WRN; break;
		default: logItLogLevel = Log::ERR; // just in case they added a new level.
	}

	Log::LOG_LEVEL userLogLevel;
	Log::getComponentLogLevel(Log::getComponentHandle("open62541"), userLogLevel);
	if (userLogLevel > logItLogLevel) return;

	// translate open62541 category.
	std::string categoryStr ("category_unknown");
	switch (category)
	{
		case UA_LOGCATEGORY_CLIENT: categoryStr = "client"; break;
		case UA_LOGCATEGORY_NETWORK: categoryStr = "network"; break;
		case UA_LOGCATEGORY_SECURECHANNEL: categoryStr = "securechannel"; break;
		case UA_LOGCATEGORY_SECURITYPOLICY: categoryStr = "securitypolicy"; break;
		case UA_LOGCATEGORY_SERVER: categoryStr = "server"; break;
		case UA_LOGCATEGORY_SESSION: categoryStr = "session"; break;
		case UA_LOGCATEGORY_USERLAND: categoryStr = "userland"; break;
	}
	char line [1024] = {0};
	vsnprintf(line, sizeof line-1, msg, args);
	LOG(logItLogLevel, logItComponentHandle) << categoryStr << ": " << line;
}

UA_Logger theLogItLogger =
{
		&logFromOpen62541,
		nullptr,
		nullptr
};
