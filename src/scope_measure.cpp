#include <scope_measure.h>

namespace newsbeuter {

#ifndef NDEBUG

scope_measure::scope_measure(const std::string& func, level ll)
	: funcname(func), lvl(ll)
{
	start = std::chrono::steady_clock::now();
}

void scope_measure::stopover(const std::string& son) {
	using namespace std::chrono;
	microseconds diff = duration_cast<microseconds>(steady_clock::now() - start);
	LOG(lvl, "scope_measure: function `%s' (stop over `%s') took %lu.%06lu s so far",
			funcname, son,
			diff.count() / 1000000, diff.count() % 1000000);
}

scope_measure::~scope_measure() {
	using namespace std::chrono;
	microseconds diff = duration_cast<microseconds>(steady_clock::now() - start);
	LOG(level::INFO, "scope_measure: function `%s' took %lu.%06lu s",
			funcname, diff.count() / 1000000, diff.count() % 1000000);
}

#endif

}
