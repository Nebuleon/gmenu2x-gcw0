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

#ifndef UTILITIES_H
#define UTILITIES_H

#include "inputmanager.h"

#include <pthread.h>

#include <string>
#include <vector>
#include <unordered_map>

typedef std::unordered_map<std::string, std::string, std::hash<std::string>> ConfStrHash;
typedef std::unordered_map<std::string, int, std::hash<std::string>> ConfIntHash;

struct ParallelTask {
	pthread_t thread;
	ParallelTask(pthread_t thread) : thread(thread) {}
};

class case_less {
public:
	bool operator()(const std::string &left, const std::string &right) const;
};

inline bool isUTF8Starter(char c) {
	return (c & 0xC0) != 0x80;
}

/** Returns the string with whitespace stripped from both ends. */
std::string trim(const std::string& s);
/** Returns the string with whitespace stripped from the start. */
std::string ltrim(const std::string& s);
/** Returns the string with whitespace stripped from the end. */
std::string rtrim(const std::string& s);

/** Returns the contents of the given file as a string. */
std::string readFileAsString(std::string const& filename);

/**
 * Writes the given string to a file.
 * The update is done atomically but not durably; if you need durability
 * when fsync() the parent directory afterwards.
 * @return True iff the file was written successfully.
 */
bool writeStringToFile(std::string const& filename, std::string const& data);

/**
 * Tells the file system to commit the given directory to disk.
 * @return True iff the sync was successful.
 */
bool syncDir(std::string const& dirname);

std::string strreplace(std::string orig, const std::string &search, const std::string &replace);
std::string cmdclean(std::string cmdline);

/**
 * Returns the parent directory of the given directory path, or "/" if there is
 * no parent.
 * This function does not check the file system: it is only string manipulation.
 * @return The parent directory path, including a trailing '/'.
 */
std::string parentDir(std::string const& dir);

inline std::string trimExtension(std::string const& filename) {
	return filename.substr(0, filename.rfind('.'));
}

bool fileExists(const std::string &file);
bool rmtree(std::string path);

int constrain(int x, int imin, int imax);

int evalIntConf(ConfIntHash& hash, const std::string &key, int def, int imin, int imax);

/**
 * Splits the given string on the given delimiter, returning the split elements
 * in the given vector.
 * A delimiter can be a string of multiple characters, in which case that
 * entire delimiter string acts as a single delimiter.
 * If the delimiter is empty, the entire string is returned as a single element.
 * Any previous contents of the vector are discarded.
 */
void split(std::vector<std::string>& vec, std::string const& str,
		std::string const& delim);

int intTransition(int from, int to, long int tickStart, long duration=500,
		long tickNow=-1);

void inject_user_event(enum EventCode code = REPAINT_MENU,
			void *data1 = NULL, void *data2 = NULL);

/**
 * Attempts to run the specified function in parallel with the code in the
 * thread calling this function. If required by the function, the specified
 * argument is passed to it.
 * This function never fails. If it cannot create a thread, it will run the
 * function in the caller's thread.
 * Returns an opaque task hint that must be passed to parallelAwait to make
 * the caller's thread await the termination of the task at some later time.
 */
ParallelTask* parallelRun(void* (*func) (void*), void* arg = NULL);

/**
 * Awaits the termination of a parallel task started with a call to
 * parallelRun.
 */
void parallelAwait(ParallelTask* task);

#endif // UTILITIES_H
