#include <cstring>
#include <errno.h>
#include <iostream>

#include "config.h"
#include "exception.h"
#include "pbcontroller.h"
#include "pbview.h"
#include "utils.h"

extern "C" {
	void rs_setup_human_panic(void);
}

using namespace podboat;

int main(int argc, char* argv[])
{
	rs_setup_human_panic();
	utils::initialize_ssl_implementation();

	setlocale(LC_CTYPE, "");
	setlocale(LC_MESSAGES, "");

	textdomain(PACKAGE);
	bindtextdomain(PACKAGE, LOCALEDIR);
	// Internally, Newsboat stores all strings in UTF-8, so we require gettext
	// to return messages in that encoding.
	bind_textdomain_codeset(PACKAGE, "UTF-8");

	PbController c;

	try {
		c.initialize(argc, argv);

		podboat::PbView v(&c);

		return c.run(v);
	} catch (const newsboat::Exception& e) {
		std::cerr << strprintf::fmt(_("Caught newsboat::Exception with "
					"message: %s"),
				e.what())
			<< std::endl;
		::exit(EXIT_FAILURE);
	}
}
