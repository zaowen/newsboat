#ifndef NEWSBOAT_FORMACTION_H_
#define NEWSBOAT_FORMACTION_H_

#include <string>
#include <vector>

#include "history.h"
#include "keymap.h"
#include "rss.h"
#include "stflpp.h"

namespace newsboat {

class view;

struct keymap_hint_entry {
	Operation op ;
	char* text;
};

typedef std::pair<std::string, std::string> qna_pair;

class formaction {
public:
	formaction(view*, std::string formstr);
	virtual ~formaction();
	virtual void prepare() = 0;
	virtual void init() = 0;
	std::shared_ptr<stfl::form> get_form();
	virtual void set_redraw(bool b)
	{
		do_redraw = b;
	}

	virtual keymap_hint_entry* get_keymap_hint() = 0;

	virtual std::string id() const = 0;

	virtual std::string get_value(const std::string& value);

	virtual void handle_cmdline(const std::string& cmd);

	void process_op(Operation op ,
		bool automatic = false,
		std::vector<std::string>* args = nullptr);

	virtual void finished_qna(Operation op );

	void start_cmdline(std::string default_value = "");

	virtual void recalculate_form();

	std::string get_qna_response(unsigned int i)
	{
		return (qna_responses.size() >= (i + 1)) ? qna_responses[i]
							 : "";
	}
	void start_qna(const std::vector<qna_pair>& prompts,
		Operation finish_op,
		history* h = nullptr);

	void set_parent_formaction(std::shared_ptr<formaction> fa)
	{
		parent_formaction = fa;
	}
	std::shared_ptr<formaction> get_parent_formaction() const
	{
		return parent_formaction;
	}

	virtual std::string title() = 0;

	virtual std::vector<std::string> get_suggestions(
		const std::string& fragment);

	static void load_histories(const std::string& searchfile,
		const std::string& cmdlinefile);
	static void save_histories(const std::string& searchfile,
		const std::string& cmdlinefile,
		unsigned int limit);

	std::string bookmark(const std::string& url,
		const std::string& title,
		const std::string& description,
		const std::string& feed_title);

protected:
	virtual void process_operation(Operation op ,
		bool automatic = false,
		std::vector<std::string>* args = nullptr) = 0;
	virtual void set_keymap_hints();

	void start_bookmark_qna(const std::string& default_title,
		const std::string& default_url,
		const std::string& default_desc,
		const std::string& default_feed_title);
	void open_unread_items_in_browser(std::shared_ptr<rss_feed> feed,
		bool markread);

	view* v;
	std::shared_ptr<stfl::form> f;
	bool do_redraw;

	std::vector<std::string> qna_responses;

	static history searchhistory;
	static history cmdlinehistory;

	std::vector<std::string> valid_cmds;

private:
	std::string prepare_keymap_hint(keymap_hint_entry* hints);
	void start_next_question();

	std::vector<qna_pair> qna_prompts;
	Operation finish_operation;
	history* qna_history;
	std::shared_ptr<formaction> parent_formaction;
};

} // namespace newsboat

#endif /* NEWSBOAT_FORMACTION_H_ */
