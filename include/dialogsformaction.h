#ifndef NEWSBOAT_DIALOGSFORMACTION_H_
#define NEWSBOAT_DIALOGSFORMACTION_H_

#include "formaction.h"

namespace newsboat {

class DialogsFormAction : public Formaction {
public:
	DialogsFormAction(view*, std::string formstr);
	~DialogsFormAction() override;
	void prepare() override;
	void init() override;
	keymap_hint_entry* get_keymap_hint() override;
	std::string id() const override
	{
		return "dialogs";
	}
	std::string title() override;
	void handle_cmdline(const std::string& cmd) override;

private:
	void process_operation(Operation op ,
		bool automatic = false,
		std::vector<std::string>* args = nullptr) override;
	bool update_list;
};

} // namespace newsboat

#endif /* NEWSBOAT_DIALOGSFORMACTION_H_ */
