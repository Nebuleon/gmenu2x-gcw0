#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <string>
#include <vector>


class Launcher
{
public:
	Launcher(std::vector<std::string> const& commandLine);
	Launcher(std::vector<std::string> && commandLine);

	void exec();

private:
	std::vector<std::string> commandLine;
};

#endif // LAUNCHER_H
