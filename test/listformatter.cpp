#include "listformatter.h"

#include "3rd-party/catch.hpp"

using namespace newsboat;

TEST_CASE("add_line(), get_lines_count() and clear()",
	"[ListFormatter]")
{
	ListFormatter fmt;

	REQUIRE(fmt.get_lines_count() == 0);

	SECTION("add_line() adds a line") {
		fmt.add_line("one");
		REQUIRE(fmt.get_lines_count() == 1);

		fmt.add_line("two");
		REQUIRE(fmt.get_lines_count() == 2);

		SECTION("clear() removes all lines") {
			fmt.clear();
			REQUIRE(fmt.get_lines_count() == 0);
		}
	}
}

TEST_CASE("set_line() replaces the item in a list", "[ListFormatter]")
{
	ListFormatter fmt;

	fmt.add_line("hello");
	fmt.add_line("goodb");
	fmt.add_line("ye");

	std::string expected =
		"{list"
		"{listitem text:\"hello\"}"
		"{listitem text:\"goodb\"}"
		"{listitem text:\"ye\"}"
		"}";
	REQUIRE(fmt.format_list() == expected);

	fmt.set_line(1, "oh");

	expected =
		"{list"
		"{listitem text:\"hello\"}"
		"{listitem text:\"oh\"}"
		"{listitem text:\"ye\"}"
		"}";
	REQUIRE(fmt.format_list() == expected);
}

TEST_CASE("format_list() uses regex manager if one is passed",
	"[ListFormatter]")
{
	RegexManager rxmgr;
	ListFormatter fmt(&rxmgr, "article");

	fmt.add_line("Highlight me please!");

	// the choice of green text on red background does not reflect my
	// personal taste (or lack thereof) :)
	rxmgr.handle_action(
		"highlight", {"article", "please", "green", "default"});

	std::string expected =
		"{list"
		"{listitem text:\"Highlight me <0>please</>!\"}"
		"}";

	REQUIRE(fmt.format_list() == expected);
}
