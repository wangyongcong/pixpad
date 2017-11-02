#pragma once
#include <string>

namespace wyc
{
	class IShellCommand
	{
	public:
		// command name
		virtual const std::string& name() const = 0;
		// short description
		virtual const std::string& description() const = 0;
		// execute command with std args
		virtual bool execute(int argc, char *argv[]) = 0;
		// execute command with string args
		virtual bool execute(const std::string &cmdline) = 0;
	};

} // namespace wyc