#include "stdafx.h"
#include "../../Headers/Logger/ILogComponent.h"

using namespace std;

ILogComponent::ILogComponent()
{
	__logger = nullptr;
	__loggerComponentID = (size_t) -1;
}

ILogComponent::~ILogComponent()
{
}

void ILogComponent::setLoggerComponentID(size_t id)
{
	__loggerComponentID = id;
}

size_t ILogComponent::getLoggerComponentID()
{
	return __loggerComponentID;
}

void ILogComponent::setLoggerCallback(Logger * logger)
{
	__logger = logger;
}

void ILogComponent::unsetLoggerCallback()
{
	__logger = nullptr;
}

void ILogComponent::logMessage(string msg)
{
	if (getLoggerComponentID() != -1) {
		__logger->logMessage(this, msg);
	}
}
