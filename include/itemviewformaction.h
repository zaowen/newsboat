#ifndef NEWSBOAT_ITEMVIEWFORMACTION_H_
#define NEWSBOAT_ITEMVIEWFORMACTION_H_

#include "formaction.h"
#include "htmlrenderer.h"
#include "regexmanager.h"
#include "rss.h"
#include "textformatter.h"

namespace newsboat {

class ItemListFormAction;

class ItemViewFormAction : public Formaction {
public:
	ItemViewFormAction(View*,
		std::shared_ptr<ItemListFormAction> il,
		std::string formstr);
	~ItemViewFormAction() override;
	void prepare() override;
	void init() override;
	void set_guid(const std::string& guid_)
	{
		guid = guid_;
	}
	void set_feed(std::shared_ptr<RssFeed> fd)
	{
		feed = fd;
	}
	void set_highlightphrase(const std::string& text);
	keymap_hint_entry* get_keymap_hint() override;
	void handle_cmdline(const std::string& cmd) override;

	std::string id() const override
	{
		return "article";
	}
	std::string title() override;

	void finished_qna(Operation op ) override;

	std::vector<std::pair<LineType, std::string>> render_html(
		const std::string& source,
		std::vector<linkpair>& thelinks,
		const std::string& url);

	void set_regexmanager(RegexManager* r);

	void update_percent();

private:
	void process_operation(Operation op ,
		bool automatic = false,
		std::vector<std::string>* args = nullptr) override;
	void set_head(const std::string& s,
		const std::string& feedtitle,
		unsigned int unread,
		unsigned int total);
	void highlight_text(const std::string& searchphrase);

	void render_source(std::vector<std::pair<LineType, std::string>>& lines,
		std::string source);

	void do_search();

	std::string guid;
	std::shared_ptr<RssFeed> feed;
	bool show_source;
	std::vector<linkpair> links;
	bool quit;
	RegexManager* rxman;
	unsigned int num_lines;
	std::shared_ptr<ItemListFormAction> itemlist;
	bool in_search;
};

} // namespace newsboat

#endif /* NEWSBOAT_ITEMVIEWFORMACTION_H_ */
