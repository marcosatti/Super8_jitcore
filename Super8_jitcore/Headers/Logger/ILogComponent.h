#pragma once

#include <string>

#include "Logger.h"

// Forward Declaration
class Logger;

class ILogComponent {
public:
	ILogComponent();
	~ILogComponent();

	virtual std::string getComponentName() = 0; // CASE SENSITIVE!
	void setLoggerComponentID(size_t id);
	size_t getLoggerComponentID();
	void setLoggerCallback(Logger * logger);
	void unsetLoggerCallback();
protected:
	void logMessage(std::string msg);
private:
	Logger * __logger;
	size_t __loggerComponentID;
};