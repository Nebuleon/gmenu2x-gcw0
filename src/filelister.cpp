/***************************************************************************
 *   Copyright (C) 2006 by Massimiliano Torromeo                           *
 *   massimiliano.torromeo@gmail.com                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "filelister.h"

#include "debug.h"
#include "utilities.h"

//for browsing the filesystem
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <set>

using namespace std;

FileLister::FileLister()
	: showDirectories(true)
	, showFiles(true)
{
}

void FileLister::setFilter(const string &filter)
{
	if (filter.empty() || filter == "*") {
		this->filter.clear();
	} else {
		split(this->filter, filter, ",");
	}
}

void FileLister::setShowDirectories(bool showDirectories)
{
	this->showDirectories = showDirectories;
}

void FileLister::setShowFiles(bool showFiles)
{
	this->showFiles = showFiles;
}

static void moveNames(set<string, case_less>&& from, vector<string>& to)
{
	to.reserve(from.size());
	for (string const& name : from) {
		// Casting away the const is necessary to make the move work.
		// It will leave the set in an invalid state where it contains multiple
		// empty strings, but since the set is an rvalue we don't care.
		to.emplace_back(move(const_cast<string&>(name)));
	}
	to.shrink_to_fit();
}

void FileLister::browse(const string& path, bool clean)
{
	if (clean) {
		directories.clear();
		files.clear();
	}

	string slashedPath = path;
	if (path[path.length() - 1] != '/')
		slashedPath.push_back('/');

	DIR *dirp;
	if ((dirp = opendir(slashedPath.c_str())) == NULL) {
		if (errno != ENOENT) {
			ERROR("Unable to open directory: %s\n", slashedPath.c_str());
		}
		return;
	}

	set<string, case_less> directorySet;
	set<string, case_less> fileSet;

	while (struct dirent *dptr = readdir(dirp)) {
		string file = dptr->d_name;

		if (file[0] == '.' && file != "..")
			continue;

		string filepath = slashedPath + file;
		struct stat st;
		int statRet = stat(filepath.c_str(), &st);
		if (statRet == -1) {
			ERROR("Stat failed on '%s' with error '%s'\n", filepath.c_str(), strerror(errno));
			continue;
		}
		if (find(excludes.begin(), excludes.end(), file) != excludes.end())
			continue;

		if (S_ISDIR(st.st_mode)) {
			if (!showDirectories)
				continue;

			directorySet.insert(file);
		} else {
			if (!showFiles)
				continue;

			if (filter.empty()) {
				fileSet.insert(file);
				continue;
			}

			for (vector<string>::iterator it = filter.begin(); it != filter.end(); ++it) {
				if (file.find('.') == string::npos) {
					if (!it->empty())
						continue;

					fileSet.insert(file);
					break;
				}

				if (it->length() < file.length()) {
					if (file[file.length() - it->length() - 1] != '.')
						continue;

					string file_lowercase =
								file.substr(file.length() - it->length());

					/* XXX: This won't accept UTF-8 codes.
						* Thanksfully file extensions shouldn't contain any. */
					transform(file_lowercase.begin(), file_lowercase.end(),
								file_lowercase.begin(), ::tolower);

					if (file_lowercase.compare(0, it->length(), *it) == 0) {
						fileSet.insert(file);
						break;
					}
				}
			}
		}
	}

	closedir(dirp);

	if (!directorySet.empty()) {
		for (string& dir : directories) {
			directorySet.emplace(move(dir));
		}
		directories.clear();
		moveNames(move(directorySet), directories);
	}

	if (!fileSet.empty()) {
		for (string& file : files) {
			fileSet.emplace(move(file));
		}
		files.clear();
		moveNames(move(fileSet), files);
	}
}

unsigned int FileLister::size()
{
	return files.size() + directories.size();
}

unsigned int FileLister::dirCount()
{
	return directories.size();
}

unsigned int FileLister::fileCount()
{
	return files.size();
}

string FileLister::operator[](uint x)
{
	return at(x);
}

string FileLister::at(uint x)
{
	if (x < directories.size())
		return directories[x];
	else
		return files[x-directories.size()];
}

bool FileLister::isFile(unsigned int x)
{
	return x >= directories.size() && x < size();
}

bool FileLister::isDirectory(unsigned int x)
{
	return x < directories.size();
}

void FileLister::insertFile(const string &file) {
	files.insert(files.begin(), file);
}

void FileLister::addExclude(const string &exclude) {
	excludes.push_back(exclude);
}
