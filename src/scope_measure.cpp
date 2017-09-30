#include <scope_measure.h>

#include <sys/time.h>

namespace newsbeuter {

scope_measure::scope_measure(const std::string& func, level ll)
	: funcname(func), lvl(ll)
{
	gettimeofday(&tv1, nullptr);
}

void scope_measure::stopover(const std::string& son) {
	gettimeofday(&tv2, nullptr);
	unsigned long diff = (((tv2.tv_sec - tv1.tv_sec) * 1000000) + tv2.tv_usec) - tv1.tv_usec;
	LOG(lvl, "scope_measure: function `%s' (stop over `%s') took %lu.%06lu s so far", funcname, son, diff / 1000000, diff % 1000000);
}

scope_measure::~scope_measure() {
	gettimeofday(&tv2, nullptr);
	unsigned long diff = (((tv2.tv_sec - tv1.tv_sec) * 1000000) + tv2.tv_usec) - tv1.tv_usec;
	LOG(level::INFO, "scope_measure: function `%s' took %lu.%06lu s", funcname, diff / 1000000, diff % 1000000);
}

}
