#pragma once

//////////////////////////////////////////////////
// Written by Marco Satti.                      //
// Licensed under GPLv3. See LICENSE file.      //
//////////////////////////////////////////////////

#define COMPONENT_LOGGER_VERSION "1.0"

#include <string>
#include <vector>

#define FORMAT_NUM_CONST_CHARS 6 // There are always 5 more characters added to each component title, listed in the FORMAT_OPTIONS (as well as log level "I/"). Extra space at the end.

extern char log_levels[];

enum LOGLEVEL {
	L_VERBOSE = 0,
	L_DEBUG,
	L_INFO,
	L_WARNING,
	L_ERROR,
	L_FATAL
};

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
<<<<<<< HEAD
	Logger(bool _formatAutoUpdate = true);
=======
	explicit Logger(bool _formatAutoUpdate = true);
>>>>>>> block_test_perf
	~Logger();

	size_t registerComponent(ILogComponent * component);
	void deregisterComponent(ILogComponent * component);
	void logMessage(ILogComponent * component, LOGLEVEL level, std::string msg);
	void updateFormat();
	void updateFormatAutoUpdate(bool _update);
	void updateFormatMessageTitleSuffix(char _spacer);
private:
	std::vector<COMPONENT_ENTRY> * component_list;
	COMPONENT_ENTRY	allocNewComponent(std::string name);
	FORMAT_OPTIONS format_options;
};