#ifndef SCOPE_MEASURE_H_
#define SCOPE_MEASURE_H_

#include <string>
#include <chrono>

#include <logger.h>

namespace newsbeuter {

#ifndef NDEBUG
class scope_measure {
	public:
		scope_measure(const std::string& func, level ll = level::DEBUG);
		~scope_measure();
		void stopover(const std::string& son = "");
	private:
		std::chrono::time_point<std::chrono::steady_clock> start;
		std::string funcname;
		level lvl;
};
#else
class scope_measure {
	public:
		scope_measure(const std::string&, level = level::DEBUG) {}
		~scope_measure() {}
		void stopover(const std::string& = "") {}
};
#endif

}

#endif /* SCOPE_MEASURE_H_ */
