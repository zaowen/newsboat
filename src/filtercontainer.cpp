#include "filtercontainer.h"

#include "config.h"
#include "exceptions.h"
#include "matcher.h"
#include "strprintf.h"
#include "utils.h"

namespace newsboat {

filtercontainer::~filtercontainer() {}

void filtercontainer::handle_action(const std::string& action,
	const std::vector<std::string>& params)
{
	/*
	 * filtercontainer does nothing but to save (filter name, filter
	 * expression) tuples. These tuples are used for enabling the user to
	 * predefine filter expressions and then select them from a list by
	 * their name.
	 */
	if (action == "define-filter") {
		if (params.size() < 2)
			throw confighandlerexception(
				ActionHandlerStatus::TOO_FEW_PARAMS);
		Matcher m;
		if (!m.parse(params[1]))
			throw confighandlerexception(StrPrintf::fmt(
				_("couldn't parse filter expression `%s': %s"),
				params[1],
				m.get_parse_error()));
		filters.push_back(filter_name_expr_pair(params[0], params[1]));
	} else
		throw confighandlerexception(
			ActionHandlerStatus::INVALID_COMMAND);
}

void filtercontainer::dump_config(std::vector<std::string>& config_output)
{
	for (const auto& filter : filters) {
		config_output.push_back(StrPrintf::fmt("define-filter %s %s",
			Utils::quote(filter.first),
			Utils::quote(filter.second)));
	}
}

} // namespace newsboat
