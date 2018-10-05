#include "listformatter.h"

#include <assert.h>
#include <limits.h>

#include "stflpp.h"
#include "strprintf.h"
#include "utils.h"

namespace newsboat {

ListFormatter::ListFormatter() {}

ListFormatter::~ListFormatter() {}

void ListFormatter::add_line(const std::string& text,
	unsigned int id,
	unsigned int width)
{
	set_line(UINT_MAX, text, id, width);
	LOG(Level::DEBUG, "ListFormatter::add_line: `%s'", text);
}

void ListFormatter::set_line(const unsigned int itempos,
	const std::string& text,
	unsigned int id,
	unsigned int width)
{
	std::vector<line_id_pair> formatted_text;

	if (width > 0 && text.length() > 0) {
		std::wstring mytext = Utils::clean_nonprintable_characters(
			Utils::str2wstr(text));

		while (mytext.length() > 0) {
			size_t size = mytext.length();
			size_t w = Utils::wcswidth_stfl(mytext, size);
			if (w > width) {
				while (size &&
					(w = Utils::wcswidth_stfl(
						 mytext, size)) > width) {
					size--;
				}
			}
			formatted_text.push_back(line_id_pair(
				Utils::wstr2str(mytext.substr(0, size)), id));
			mytext.erase(0, size);
		}
	} else {
		formatted_text.push_back(line_id_pair(
			Utils::wstr2str(Utils::clean_nonprintable_characters(
				Utils::str2wstr(text))),
			id));
	}

	if (itempos == UINT_MAX) {
		lines.insert(lines.cend(),
			formatted_text.cbegin(),
			formatted_text.cend());
	} else {
		lines[itempos] = formatted_text[0];
	}
}

void ListFormatter::add_lines(const std::vector<std::string>& thelines,
	unsigned int width)
{
	for (const auto& line : thelines) {
		add_line(Utils::replace_all(line, "\t", "        "),
			UINT_MAX,
			width);
	}
}

std::string ListFormatter::format_list(regexmanager* rxman,
	const std::string& location)
{
	format_cache = "{list";
	for (const auto& line : lines) {
		std::string str = line.first;
		if (rxman)
			rxman->quote_and_highlight(str, location);
		if (line.second == UINT_MAX) {
			format_cache.append(StrPrintf::fmt(
				"{listitem text:%s}", stfl::quote(str)));
		} else {
			format_cache.append(
				StrPrintf::fmt("{listitem[%u] text:%s}",
					line.second,
					stfl::quote(str)));
		}
	}
	format_cache.append(1, '}');
	return format_cache;
}

} // namespace newsboat
