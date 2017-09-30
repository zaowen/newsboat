#ifndef SCOPE_MEASURE_H_
#define SCOPE_MEASURE_H_

#include <string>

#include <logger.h>

namespace newsbeuter {

#ifndef NDEBUG
class scope_measure {
	public:
		scope_measure(const std::string& func, level ll = level::DEBUG);
		~scope_measure();
		void stopover(const std::string& son = "");
	private:
		struct timeval tv1, tv2;
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
