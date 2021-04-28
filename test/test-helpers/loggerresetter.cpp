#include "loggerresetter.h"

#include "logger.h"

namespace TestHelpers {

void reset_logger()
{
	::newsboat::Logger::set_logfile("/dev/null");
	::newsboat::Logger::set_user_error_logfile("/dev/null");
	::newsboat::Logger::unset_loglevel();
}

LoggerResetter::LoggerResetter()
{
	reset_logger();
}

LoggerResetter::~LoggerResetter()
{
	reset_logger();
}

} /* namespace TestHelpers */
