#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>

#include "ILogComponent.h"

#define FORMAT_NUM_CONST_CHARS 4 // There are always 3 more characters added to each component title, listed in the FORMAT_OPTIONS. Extra space at the end.

// Forward Declaration
class ILogComponent;

struct COMPONENT_ENTRY {
	std::string name;
	std::vector<ILogComponent*> * object_list;
};

struct FORMAT_OPTIONS {
	bool formatAutoUpdate = true;
	char formatMessageTitleSuffix = ':';
	char formatComponentNumberPrefix = '<';
	char formatComponentNumberSuffix = '>';
	size_t formatMessageTitleMinLength = 0;
};

class Logger {
public:
	Logger(bool _formatAutoUpdate = true);
	~Logger();

	size_t registerComponent(ILogComponent * component);
	void deregisterComponent(ILogComponent * component);
	void logMessage(ILogComponent * component, std::string msg);
	void updateFormat();
	void updateFormatAutoUpdate(bool _update);
	void updateFormatMessageTitleSuffix(char _spacer);
private:
	std::vector<COMPONENT_ENTRY> * component_list;
	COMPONENT_ENTRY	allocNewComponent(std::string name);
	FORMAT_OPTIONS format_options;
};