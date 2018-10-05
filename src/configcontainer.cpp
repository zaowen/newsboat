#include "configcontainer.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <pwd.h>
#include <sstream>
#include <sys/types.h>

#include "config.h"
#include "configparser.h"
#include "exceptions.h"
#include "logger.h"
#include "strprintf.h"
#include "utils.h"

namespace newsboat {

const std::string configcontainer::PARTIAL_FILE_SUFFIX = ".part";

configcontainer::configcontainer()
	// create the config options and set their resp. default value and type
	: config_data{{"always-display-description",
			      configdata("false", ConfigData::BOOL)},
		  {"article-sort-order",
			  configdata("date-asc", ConfigData::STR)},
		  {"articlelist-format",
			  configdata("%4i %f %D %6L  %?T?|%-17T|  &?%t",
				  ConfigData::STR)},
		  {"auto-reload", configdata("no", ConfigData::BOOL)},
		  {"bookmark-autopilot",
			  configdata("false", ConfigData::BOOL)},
		  {"bookmark-cmd", configdata("", ConfigData::STR)},
		  {"bookmark-interactive",
			  configdata("false", ConfigData::BOOL)},
		  {"browser",
			  configdata(Utils::get_default_browser(),
				  ConfigData::PATH)},
		  {"cache-file", configdata("", ConfigData::PATH)},
		  {"cleanup-on-quit", configdata("yes", ConfigData::BOOL)},
		  {"confirm-exit", configdata("no", ConfigData::BOOL)},
		  {"cookie-cache", configdata("", ConfigData::PATH)},
		  {"datetime-format", configdata("%b %d", ConfigData::STR)},
		  {"delete-read-articles-on-quit",
			  configdata("false", ConfigData::BOOL)},
		  {"display-article-progress",
			  configdata("yes", ConfigData::BOOL)},
		  {"download-filename-format",
			  configdata("%?u?%u&%Y-%b-%d-%H%M%S.unknown?",
				  ConfigData::STR)},
		  {"download-full-page",
			  configdata("false", ConfigData::BOOL)},
		  {"download-path", configdata("~/", ConfigData::PATH)},
		  {"download-retries", configdata("1", ConfigData::INT)},
		  {"download-timeout", configdata("30", ConfigData::INT)},
		  {"error-log", configdata("", ConfigData::PATH)},
		  {"external-url-viewer", configdata("", ConfigData::PATH)},
		  {"feed-sort-order",
			  configdata("none-desc", ConfigData::STR)},
		  {"feedhq-flag-share", configdata("", ConfigData::STR)},
		  {"feedhq-flag-star", configdata("", ConfigData::STR)},
		  {"feedhq-login", configdata("", ConfigData::STR)},
		  {"feedhq-min-items", configdata("20", ConfigData::INT)},
		  {"feedhq-password", configdata("", ConfigData::STR)},
		  {"feedhq-passwordfile", configdata("", ConfigData::PATH)},
		  {"feedhq-passwordeval", configdata("", ConfigData::STR)},
		  {"feedhq-show-special-feeds",
			  configdata("true", ConfigData::BOOL)},
		  {"feedhq-url",
			  configdata("https://feedhq.org/", ConfigData::STR)},
		  {"feedlist-format",
			  configdata("%4i %n %11u %t", ConfigData::STR)},
		  {"goto-first-unread", configdata("true", ConfigData::BOOL)},
		  {"goto-next-feed", configdata("yes", ConfigData::BOOL)},
		  {"history-limit", configdata("100", ConfigData::INT)},
		  {"html-renderer", configdata("internal", ConfigData::PATH)},
		  {"http-auth-method",
			  configdata("any",
				  std::unordered_set<std::string>({"any",
					  "basic",
					  "digest",
					  "digest_ie",
					  "gssnegotiate",
					  "ntlm",
					  "anysafe"}))},
		  {"ignore-mode",
			  configdata("download",
				  std::unordered_set<std::string>(
					  {"download", "display"}))},
		  {"inoreader-login", configdata("", ConfigData::STR)},
		  {"inoreader-password", configdata("", ConfigData::STR)},
		  {"inoreader-passwordfile",
			  configdata("", ConfigData::PATH)},
		  {"inoreader-passwordeval", configdata("", ConfigData::STR)},
		  {"inoreader-show-special-feeds",
			  configdata("true", ConfigData::BOOL)},
		  {"inoreader-flag-share", configdata("", ConfigData::STR)},
		  {"inoreader-flag-star", configdata("", ConfigData::STR)},
		  {"inoreader-min-items", configdata("20", ConfigData::INT)},
		  {"keep-articles-days", configdata("0", ConfigData::INT)},
		  {"mark-as-read-on-hover",
			  configdata("false", ConfigData::BOOL)},
		  {"max-browser-tabs", configdata("10", ConfigData::INT)},
		  {"markfeedread-jumps-to-next-unread",
			  configdata("false", ConfigData::BOOL)},
		  {"max-download-speed", configdata("0", ConfigData::INT)},
		  {"max-downloads", configdata("1", ConfigData::INT)},
		  {"max-items", configdata("0", ConfigData::INT)},
		  {"newsblur-login", configdata("", ConfigData::STR)},
		  {"newsblur-min-items", configdata("20", ConfigData::INT)},
		  {"newsblur-password", configdata("", ConfigData::STR)},
		  {"newsblur-passwordfile", configdata("", ConfigData::PATH)},
		  {"newsblur-passwordeval", configdata("", ConfigData::STR)},
		  {"newsblur-url",
			  configdata("https://newsblur.com",
				  ConfigData::STR)},
		  {"notify-always", configdata("no", ConfigData::BOOL)},
		  {"notify-beep", configdata("no", ConfigData::BOOL)},
		  {"notify-format",
			  configdata(_("newsboat: finished reload, %f unread "
				       "feeds (%n unread articles total)"),
				  ConfigData::STR)},
		  {"notify-program", configdata("", ConfigData::PATH)},
		  {"notify-screen", configdata("no", ConfigData::BOOL)},
		  {"notify-xterm", configdata("no", ConfigData::BOOL)},
		  {"oldreader-flag-share", configdata("", ConfigData::STR)},
		  {"oldreader-flag-star", configdata("", ConfigData::STR)},
		  {"oldreader-login", configdata("", ConfigData::STR)},
		  {"oldreader-min-items", configdata("20", ConfigData::INT)},
		  {"oldreader-password", configdata("", ConfigData::STR)},
		  {"oldreader-passwordfile",
			  configdata("", ConfigData::PATH)},
		  {"oldreader-passwordeval", configdata("", ConfigData::STR)},
		  {"oldreader-show-special-feeds",
			  configdata("true", ConfigData::BOOL)},
		  {"openbrowser-and-mark-jumps-to-next-unread",
			  configdata("false", ConfigData::BOOL)},
		  {"opml-url", configdata("", ConfigData::STR, true)},
		  {"pager", configdata("internal", ConfigData::PATH)},
		  {"player", configdata("", ConfigData::PATH)},
		  {"podcast-auto-enqueue",
			  configdata("no", ConfigData::BOOL)},
		  {"podlist-format",
			  configdata( "%4i [%6dMB/%6tMB] [%5p %%] [%7k kb/s] %-20S %u -> %F", ConfigData::STR)},
		  {"prepopulate-query-feeds",
			  configdata("false", ConfigData::BOOL)},
		  {"ssl-verifyhost", configdata("true", ConfigData::BOOL)},
		  {"ssl-verifypeer", configdata("true", ConfigData::BOOL)},
		  {"proxy", configdata("", ConfigData::STR)},
		  {"proxy-auth", configdata("", ConfigData::STR)},
		  {"proxy-auth-method",
			  configdata("any",
				  std::unordered_set<std::string>({"any",
					  "basic",
					  "digest",
					  "digest_ie",
					  "gssnegotiate",
					  "ntlm",
					  "anysafe"}))},
		  {"proxy-type",
			  configdata("http",
				  std::unordered_set<std::string>({"http",
					  "socks4",
					  "socks4a",
					  "socks5",
					  "socks5h"}))},
		  {"refresh-on-startup", configdata("no", ConfigData::BOOL)},
		  {"reload-only-visible-feeds",
			  configdata("false", ConfigData::BOOL)},
		  {"reload-threads", configdata("1", ConfigData::INT)},
		  {"reload-time", configdata("60", ConfigData::INT)},
		  {"save-path", configdata("~/", ConfigData::PATH)},
		  {"search-highlight-colors",
			  configdata("black yellow bold",
				  ConfigData::STR,
				  true)},
		  {"show-keymap-hint", configdata("yes", ConfigData::BOOL)},
		  {"show-read-articles", configdata("yes", ConfigData::BOOL)},
		  {"show-read-feeds", configdata("yes", ConfigData::BOOL)},
		  {"suppress-first-reload",
			  configdata("no", ConfigData::BOOL)},
		  {"swap-title-and-hints",
			  configdata("no", ConfigData::BOOL)},
		  {"text-width", configdata("0", ConfigData::INT)},
		  {"toggleitemread-jumps-to-next-unread",
			  configdata("false", ConfigData::BOOL)},
		  {"ttrss-flag-publish", configdata("", ConfigData::STR)},
		  {"ttrss-flag-star", configdata("", ConfigData::STR)},
		  {"ttrss-login", configdata("", ConfigData::STR)},
		  {"ttrss-mode",
			  configdata("multi",
				  std::unordered_set<std::string>(
					  {"single", "multi"}))},
		  {"ttrss-password", configdata("", ConfigData::STR)},
		  {"ttrss-passwordfile", configdata("", ConfigData::PATH)},
		  {"ttrss-passwordeval", configdata("", ConfigData::STR)},
		  {"ttrss-url", configdata("", ConfigData::STR)},
		  {"ocnews-login", configdata("", ConfigData::STR)},
		  {"ocnews-password", configdata("", ConfigData::STR)},
		  {"ocnews-passwordfile", configdata("", ConfigData::PATH)},
		  {"ocnews-passwordeval", configdata("", ConfigData::STR)},
		  {"ocnews-flag-star", configdata("", ConfigData::STR)},
		  {"ocnews-url", configdata("", ConfigData::STR)},
		  {"urls-source",
			  configdata("local",
				  std::unordered_set<std::string>({"local",
					  "opml",
					  "oldreader",
					  "ttrss",
					  "newsblur",
					  "feedhq",
					  "ocnews",
					  "inoreader"}))},
		  {"use-proxy", configdata("no", ConfigData::BOOL)},
		  {"user-agent", configdata("", ConfigData::STR)},

		  /* title formats: */
		  {"articlelist-title-format",
			  configdata(_("%N %V - Articles in feed '%T' (%u "
				       "unread, %t "
				       "total) - %U"),
				  ConfigData::STR)},
		  {"dialogs-title-format",
			  configdata(_("%N %V - Dialogs"), ConfigData::STR)},
		  {"feedlist-title-format",
			  configdata(_("%N %V - Your feeds (%u unread, %t "
				       "total)%?T? - "
				       "tag `%T'&?"),
				  ConfigData::STR)},
		  {"filebrowser-title-format",
			  configdata(_("%N %V - %?O?Open File&Save File? - %f"),
				  ConfigData::STR)},
		  {"help-title-format",
			  configdata(_("%N %V - Help"), ConfigData::STR)},
		  {"itemview-title-format",
			  configdata(_("%N %V - Article '%T' (%u unread, %t "
				       "total)"),
				  ConfigData::STR)},
		  {"searchresult-title-format",
			  configdata(_("%N %V - Search result (%u unread, %t "
				       "total)"),
				  ConfigData::STR)},
		  {"selectfilter-title-format",
			  configdata(_("%N %V - Select Filter"),
				  ConfigData::STR)},
		  {"selecttag-title-format",
			  configdata(_("%N %V - Select Tag"),
				  ConfigData::STR)},
		  {"urlview-title-format",
			  configdata(_("%N %V - URLs"), ConfigData::STR)}}
{
}

configcontainer::~configcontainer() {}

void configcontainer::register_commands(configparser& cfgparser)
{
	// this registers the config options defined above in the configuration
	// parser
	// -> if the resp. config option is encountered, it is passed to the
	// configcontainer
	for (const auto& cfg : config_data) {
		cfgparser.register_handler(cfg.first, this);
	}
}

void configcontainer::handle_action(const std::string& action,
	const std::vector<std::string>& params)
{
	configdata& cfgdata = config_data[action];

	// ConfigData::INVALID indicates that the action didn't exist, and
	// that the returned object was created ad-hoc.
	if (cfgdata.type == ConfigData::INVALID) {
		LOG(Level::WARN,
			"configcontainer::handle_action: unknown action %s",
			action);
		throw confighandlerexception(
			ActionHandlerStatus::INVALID_COMMAND);
	}

	LOG(Level::DEBUG,
		"configcontainer::handle_action: action = %s, type = %u",
		action,
		cfgdata.type);

	if (params.size() < 1) {
		throw confighandlerexception(
			ActionHandlerStatus::TOO_FEW_PARAMS);
	}

	switch (cfgdata.type) {
	case ConfigData::BOOL:
		if (!is_bool(params[0]))
			throw confighandlerexception(StrPrintf::fmt(
				_("expected boolean value, found `%s' instead"),
				params[0]));
		cfgdata.value = params[0];
		break;

	case ConfigData::INT:
		if (!is_int(params[0]))
			throw confighandlerexception(StrPrintf::fmt(
				_("expected integer value, found `%s' instead"),
				params[0]));
		cfgdata.value = params[0];
		break;

	case ConfigData::ENUM:
		if (cfgdata.enum_values.find(params[0]) ==
			cfgdata.enum_values.end())
			throw confighandlerexception(StrPrintf::fmt(
				_("invalid configuration value `%s'"),
				params[0]));
	// fall-through
	case ConfigData::STR:
	case ConfigData::PATH:
		if (cfgdata.multi_option)
			cfgdata.value = Utils::join(params, " ");
		else
			cfgdata.value = params[0];
		break;

	case ConfigData::INVALID:
		// we already handled this at the beginning of the function
		break;
	}
}

bool configcontainer::is_bool(const std::string& s)
{
	const auto bool_values = {"yes", "no", "true", "false"};
	return (std::find(bool_values.begin(), bool_values.end(), s) !=
		bool_values.end());
}

bool configcontainer::is_int(const std::string& s)
{
	return std::all_of(s.begin(), s.end(), ::isdigit);
}

std::string configcontainer::get_configvalue(const std::string& key)
{
	std::string retval = config_data[key].value;
	if (config_data[key].type == ConfigData::PATH) {
		retval = Utils::resolve_tilde(retval);
	}
	return retval;
}

int configcontainer::get_configvalue_as_int(const std::string& key)
{
	std::istringstream is(config_data[key].value);
	int i;
	is >> i;
	return i;
}

bool configcontainer::get_configvalue_as_bool(const std::string& key)
{
	std::string value = config_data[key].value;
	return (value == "true" || value == "yes");
}

void configcontainer::set_configvalue(const std::string& key,
	const std::string& value)
{
	LOG(Level::DEBUG,
		"configcontainer::set_configvalue(%s, %s) called",
		key,
		value);
	config_data[key].value = value;
}

void configcontainer::reset_to_default(const std::string& key)
{
	config_data[key].value = config_data[key].default_value;
}

void configcontainer::toggle(const std::string& key)
{
	if (config_data[key].type == ConfigData::BOOL) {
		set_configvalue(key,
			std::string(get_configvalue_as_bool(key) ? "false"
								 : "true"));
	}
}

void configcontainer::dump_config(std::vector<std::string>& config_output)
{
	for (const auto& cfg : config_data) {
		std::string configline = cfg.first + " ";
		assert(cfg.second.type != ConfigData::INVALID);
		switch (cfg.second.type) {
		case ConfigData::BOOL:
		case ConfigData::INT:
			configline.append(cfg.second.value);
			if (cfg.second.value != cfg.second.default_value)
				configline.append(
					StrPrintf::fmt(" # default: %s",
						cfg.second.default_value));
			break;
		case ConfigData::ENUM:
		case ConfigData::STR:
		case ConfigData::PATH:
			if (cfg.second.multi_option) {
				std::vector<std::string> tokens =
					Utils::tokenize(cfg.second.value, " ");
				for (const auto& token : tokens) {
					configline.append(
						Utils::quote(token) + " ");
				}
			} else {
				configline.append(
					Utils::quote(cfg.second.value));
				if (cfg.second.value !=
					cfg.second.default_value) {
					configline.append(StrPrintf::fmt(
						" # default: %s",
						cfg.second.default_value));
				}
			}
			break;
		case ConfigData::INVALID:
			// can't happen because we already checked this case
			// before the `switch`
			break;
		}
		config_output.push_back(configline);
	}
}

std::vector<std::string> configcontainer::get_suggestions(
	const std::string& fragment)
{
	std::vector<std::string> result;
	for (const auto& cfg : config_data) {
		if (cfg.first.substr(0, fragment.length()) == fragment)
			result.push_back(cfg.first);
	}
	std::sort(result.begin(), result.end());
	return result;
}

FeedSortStrategy configcontainer::get_feed_sort_strategy()
{
	FeedSortStrategy ss;
	const auto sortmethod_info =
		Utils::tokenize(get_configvalue("feed-sort-order"), "-");
	const std::string sortmethod = sortmethod_info[0];

	std::string direction = "desc";
	if (sortmethod_info.size() > 1) {
		direction = sortmethod_info[1];
	}

	if (sortmethod == "none") {
		ss.sm = FeedSortMethod::NONE;
	} else if (sortmethod == "firsttag") {
		ss.sm = FeedSortMethod::FIRST_TAG;
	} else if (sortmethod == "title") {
		ss.sm = FeedSortMethod::TITLE;
	} else if (sortmethod == "articlecount") {
		ss.sm = FeedSortMethod::ARTICLE_COUNT;
	} else if (sortmethod == "unreadarticlecount") {
		ss.sm = FeedSortMethod::UNREAD_ARTICLE_COUNT;
	} else if (sortmethod == "lastupdated") {
		ss.sm = FeedSortMethod::LAST_UPDATED;
	}

	if (direction == "asc") {
		ss.sd = SortDirection::ASC;
	} else if (direction == "desc") {
		ss.sd = SortDirection::DESC;
	}

	return ss;
}

ArticleSortStrategy configcontainer::get_article_sort_strategy()
{
	ArticleSortStrategy ss;
	const auto methods =
		Utils::tokenize(get_configvalue("article-sort-order"), "-");

	if (!methods.empty() &&
		methods[0] == "date") { // date is descending by default
		ss.sm = ArtSortMethod::DATE;
		ss.sd = SortDirection::DESC;
		if (methods.size() > 1 && methods[1] == "asc") {
			ss.sd = SortDirection::ASC;
		}
	} else { // all other sort methods are ascending by default
		ss.sd = SortDirection::ASC;
		if (methods.size() > 1 && methods[1] == "desc") {
			ss.sd = SortDirection::DESC;
		}
	}

	if (!methods.empty()) {
		if (methods[0] == "title") {
			ss.sm = ArtSortMethod::TITLE;
		} else if (methods[0] == "flags") {
			ss.sm = ArtSortMethod::FLAGS;
		} else if (methods[0] == "author") {
			ss.sm = ArtSortMethod::AUTHOR;
		} else if (methods[0] == "link") {
			ss.sm = ArtSortMethod::LINK;
		} else if (methods[0] == "guid") {
			ss.sm = ArtSortMethod::GUID;
		}
	}

	return ss;
}

} // namespace newsboat
