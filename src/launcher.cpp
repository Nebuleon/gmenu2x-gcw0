#include "launcher.h"

#include "debug.h"

#include <cerrno>
#include <cstring>
#include <unistd.h>

using namespace std;


Launcher::Launcher(vector<string> const& commandLine)
	: commandLine(commandLine)
{
}

Launcher::Launcher(vector<string> && commandLine)
	: commandLine(commandLine)
{
}

void Launcher::exec()
{
	vector<const char *> args;
	args.reserve(commandLine.size() + 1);
	for (auto arg : commandLine) {
		args.push_back(arg.c_str());
	}
	args.push_back(nullptr);
	execvp(commandLine[0].c_str(), (char* const*)&args[0]);
	WARNING("Failed to exec '%s': %s\n",
			commandLine[0].c_str(), strerror(errno));
}
