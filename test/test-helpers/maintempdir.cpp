#include "maintempdir.h"

#include <cerrno>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

TestHelpers::MainTempDir::tempfileexception::tempfileexception(
	const std::string& error)
	: msg("tempfileexception: " + error)
{
};

const char* TestHelpers::MainTempDir::tempfileexception::what() const throw()
{
	return msg.c_str();
}

TestHelpers::MainTempDir::MainTempDir()
{
	char* tmpdir_p = ::getenv("TMPDIR");

	if (tmpdir_p) {
		tempdir = tmpdir_p;
	} else {
		tempdir = "/tmp/";
	}

	tempdir += "/newsboat-tests/";

	int status = mkdir(tempdir.c_str(), S_IRWXU);
	if (status != 0) {
		// The directory already exists. That's fine, though,
		// but only as long as it has all the properties we
		// need.

		int saved_errno = errno;
		bool success = false;

		if (saved_errno == EEXIST) {
			struct stat buffer;
			if (lstat(tempdir.c_str(), &buffer) == 0) {
				if (buffer.st_mode & S_IRUSR &&
					buffer.st_mode & S_IWUSR &&
					buffer.st_mode & S_IXUSR) {
					success = true;
				}
			}
		}

		if (!success) {
			throw tempfileexception(strerror(saved_errno));
		}
	}
}

TestHelpers::MainTempDir::~MainTempDir()
{
	// Try to remove the tempdir, but don't try *too* hard: there might be
	// other objects still using it. The last one will hopefully delete it.
	::rmdir(tempdir.c_str());
}

const std::string TestHelpers::MainTempDir::get_path() const
{
	return tempdir;
}
