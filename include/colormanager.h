#ifndef NEWSBOAT_COLORMANAGER_H_
#define NEWSBOAT_COLORMANAGER_H_

#include <functional>
#include <map>
#include <vector>

#include "configactionhandler.h"

namespace podboat {
class PbView;
}

class View;

namespace newsboat {

class ConfigParser;

struct TextStyle {
	std::string fg_color;
	std::string bg_color;
	std::vector<std::string> attributes;
};

class ColorManager : public ConfigActionHandler {
public:
	ColorManager();
	~ColorManager() override;
	void register_commands(ConfigParser& cfgparser);
	void handle_action(const std::string& action,
		const std::vector<std::string>& params) override;
	void dump_config(std::vector<std::string>& config_output) const override;
	void apply_colors(std::function<void(const std::string&, const std::string&)>
		stfl_value_setter) const;
	std::map<std::string, TextStyle> get_styles() const
	{
		return element_styles;
	}

private:
	std::map<std::string, TextStyle> element_styles;
};

} // namespace newsboat

#endif /* NEWSBOAT_COLORMANAGER_H_ */
