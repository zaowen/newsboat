#ifndef NEWSBOAT_EMPTYFORMACTION_H_
#define NEWSBOAT_EMPTYFORMACTION_H_

#include "formaction.h"

namespace newsboat {

class EmptyFormAction : public FormAction {
public:
	EmptyFormAction(View* v, const std::string& formstr, ConfigContainer* cfg);
	virtual ~EmptyFormAction() = default;

	std::string id() const override;
	std::string title() override;

	void init() override;
	void prepare() override;

	KeyMapHintEntry* get_keymap_hint() override;

protected:
	bool process_operation(Operation op,
		bool automatic = false,
		std::vector<std::string>* args = nullptr) override;
};

} // namespace newsboat

#endif /* NEWSBOAT_EMPTYFORMACTION_H_ */
