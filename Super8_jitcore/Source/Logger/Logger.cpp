#include "stdafx.h"
#include "../../Headers/Logger/Logger.h"

using namespace std;

Logger::Logger(bool _autoUpdateFormat)
{
	component_list = new vector<COMPONENT_ENTRY>();
	component_list->reserve(20);
	format_options.formatAutoUpdate = _autoUpdateFormat;
}

Logger::~Logger()
{
	for (size_t i = 0; i < component_list->size(); i++) {
		delete component_list->at(i).object_list;
	}
	delete component_list;
}

size_t Logger::registerComponent(ILogComponent * component)
{
	string name = component->getComponentName();

	// Check if the component is already registered
	if (component->getLoggerComponentID() != -1) {
		// Do not do anything.
		return component->getLoggerComponentID();
	}

	size_t objectListIndex = (size_t)-1;
	size_t componentListIndex = (size_t) -1;

	// Check for exisitng component name in component list.
	for (size_t i = 0; i < component_list->size(); i++) {
		if (name == component_list->at(i).name) {
			componentListIndex = i;
			break;
		}
	}

	// Existing component name found.
	if (componentListIndex != (size_t)-1) {
		component_list->at(componentListIndex).object_list->push_back(component);
		objectListIndex = component_list->at(componentListIndex).object_list->size() - 1;
	} 
	// No component name found, allocate a new one.
	else {
		COMPONENT_ENTRY & entry = allocNewComponent(name);
		entry.object_list->push_back(component);
		objectListIndex = entry.object_list->size() - 1;
		component_list->push_back(entry);
	}

	// Set component object properties.
	component->setLoggerCallback(this);
	component->setLoggerComponentID(objectListIndex);
	return objectListIndex;
}

void Logger::deregisterComponent(ILogComponent * component)
{
	string name = component->getComponentName();
	size_t objectIndex = component->getLoggerComponentID();

	size_t componentListIndex = (size_t)-1;
	// Check for exisitng component name in component list.
	for (size_t i = 0; i < component_list->size(); i++) {
		if (name == component_list->at(i).name) {
			componentListIndex = i;
			break;
		}
	}

	// Existing component name found.
	if (componentListIndex != (size_t)-1) {
		COMPONENT_ENTRY & entry = component_list->at(componentListIndex);

		// Remove object.
		entry.object_list->erase(entry.object_list->begin() + objectIndex);

		// Need to also update objects on their new id's.
		for (size_t i = 0; i < entry.object_list->size(); i++) {
			entry.object_list->at(i)->setLoggerComponentID(i);
		}
	}
	// else {} ---- do nothing.

	// Set component object properties.
	component->setLoggerComponentID((size_t)-1);
	component->unsetLoggerCallback();
}

void Logger::logMessage(ILogComponent * component, string msg)
{
	if (format_options.formatAutoUpdate) updateFormat();

	// Setup initial string
	stringstream buffer;
	buffer << component->getComponentName()
		<< format_options.formatComponentNumberPrefix
		<< component->getLoggerComponentID()
		<< format_options.formatComponentNumberSuffix
		<< format_options.formatMessageTitleSuffix;

	// Pad string until min length is reached
	for (size_t i = (size_t) buffer.tellp(); i <= format_options.formatMessageTitleMinLength; i++) {
		buffer.put(' ');
	}

	// Put message into buffer
	buffer << msg;

	// Display final message
	cout << buffer.str() << endl;
}

void Logger::updateFormat()
{
	// This function does any pre-processing needed before a message can be printed.

	// Work out the min title length thats needed for aligned message body's.
	// Reset to 0
	format_options.formatMessageTitleMinLength = 0;

	// Calculate largest component name length
	for (size_t i = 0; i < component_list->size(); i++) {
		format_options.formatMessageTitleMinLength = max(component_list->at(i).name.length(), format_options.formatMessageTitleMinLength);
	}

	// Add the number of constant characters as defined.
	format_options.formatMessageTitleMinLength += FORMAT_NUM_CONST_CHARS;
}

void Logger::updateFormatAutoUpdate(bool _update)
{
	format_options.formatAutoUpdate = _update;
}

void Logger::updateFormatMessageTitleSuffix(char _suffixchar)
{
	format_options.formatMessageTitleSuffix = _suffixchar;
}

COMPONENT_ENTRY Logger::allocNewComponent(string name)
{
	vector<ILogComponent*> * object_list = new vector<ILogComponent*>();
	object_list->reserve(10);
	return { name, object_list };
}
