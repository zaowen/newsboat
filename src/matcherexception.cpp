#include "matcherexception.h"

#include "config.h"
#include "ruststring.h"
#include "strprintf.h"

namespace newsboat {

const char* MatcherException::what() const throw()
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

MatcherException MatcherException::from_rust_error(MatcherErrorFfi error)
{
	const std::string info = RustString(error.info);
	const std::string info2 = RustString(error.info2);
	return MatcherException(error.type, info, info2);
}

} // namespace newsboat
