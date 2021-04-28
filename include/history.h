#ifndef NEWSBOAT_HISTORY_H_
#define NEWSBOAT_HISTORY_H_

#include "history.rs.h"

#include <string>

namespace newsboat {

class History {
public:
	History();
	~History() = default;
	void add_line(const std::string& line);
	std::string previous_line();
	std::string next_line();
	void load_from_file(const std::string& file);
	void save_to_file(const std::string& file, unsigned int limit);

private:
	rust::Box<history::bridged::History> rs_object;
};

} // namespace newsboat

#endif /* NEWSBOAT_HISTORY_H_ */
