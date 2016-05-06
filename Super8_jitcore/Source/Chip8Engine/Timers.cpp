#include "stdafx.h"

#include "Headers\Globals.h"

#include "Headers\Chip8Engine\Timers.h"

using namespace Chip8Engine;

std::string Timers::getComponentName()
{
	return std::string("Timers");
}

Timers::Timers()
{
	// Register this component in logger
	logger->registerComponent(this);
}

Timers::~Timers()
{
	// Deregister this component in logger
	logger->deregisterComponent(this);
}