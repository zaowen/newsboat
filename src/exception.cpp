#include "exception.h"

#include <cerrno>
#include <cstring>

#include "config.h"
#include "exceptions.h"
#include "strprintf.h"
#include "utils.h"

namespace newsboat {

exception::exception(unsigned int error_code)
	: ecode(error_code)
{
}

exception::~exception() throw() {}

const char* exception::what() const throw()
{
	return ::strerror(ecode);
}

const char* matcherexception::what() const throw()
{
	static std::string errmsg;
	switch (type_) {
	case Type::ATTRIB_UNAVAIL:
		errmsg = strprintf::fmt(
			_("attribute `%s' is not available."), addinfo);
		break;
	case Type::INVALID_REGEX:
		errmsg = strprintf::fmt(
			_("regular expression '%s' is invalid: %s"),
			addinfo,
			addinfo2);
		break;
	}
	return errmsg.c_str();
}

confighandlerexception::confighandlerexception(ActionHandlerStatus e)
{
	msg = get_errmsg(e);
}

const char* confighandlerexception::get_errmsg(ActionHandlerStatus status)
{
	switch (status) {
	case ActionHandlerStatus::INVALID_PARAMS:
		return _("invalid parameters.");
	case ActionHandlerStatus::TOO_FEW_PARAMS:
		return _("too few parameters.");
	case ActionHandlerStatus::INVALID_COMMAND:
		return _("unknown command (bug).");
	case ActionHandlerStatus::FILENOTFOUND:
		return _("file couldn't be opened.");
	default:
		return _("unknown error (bug).");
	}
}

} // namespace newsboat
