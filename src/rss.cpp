#include "rss.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <curl/curl.h>
#include <functional>
#include <iostream>
#include <langinfo.h>
#include <sstream>
#include <sys/utsname.h>
#include <string.h>
#include <time.h>

#include "cache.h"
#include "config.h"
#include "configcontainer.h"
#include "exceptions.h"
#include "htmlrenderer.h"
#include "logger.h"
#include "strprintf.h"
#include "tagsouppullparser.h"
#include "utils.h"

namespace newsboat {

rss_item::rss_item(cache* c)
	: ch(c)
	, idx(0)
	, size_(0)
	, pubDate_(0)
	, unread_(true)
	, enqueued_(false)
	, deleted_(0)
	, override_unread_(false)
{
}

rss_item::~rss_item() {}

rss_feed::rss_feed(cache* c)
	: pubDate_(0)
	, ch(c)
	, empty(true)
	, is_rtl_(false)
	, idx(0)
	, order(0)
	, status_(DlStatus::SUCCESS)
{
}

rss_feed::~rss_feed()
{
	clear_items();
}

// rss_item setters

void rss_item::set_title(const std::string& t)
{
	title_ = t;
	utils::trim(title_);
}

void rss_item::set_link(const std::string& l)
{
	link_ = l;
	utils::trim(link_);
}

void rss_item::set_author(const std::string& a)
{
	author_ = a;
}

void rss_item::set_description(const std::string& d)
{
	description_ = d;
}

void rss_item::set_size(unsigned int size)
{
	size_ = size;
}

std::string rss_item::length() const
{
	std::string::size_type l(size_);
	if (!l)
		return "";
	if (l < 1000)
		return strprintf::fmt("%u ", l);
	if (l < 1024 * 1000)
		return strprintf::fmt("%.1fK", l / 1024.0);

	return strprintf::fmt("%.1fM", l / 1024.0 / 1024.0);
}

void rss_item::set_pubDate(time_t t)
{
	pubDate_ = t;
}

void rss_item::set_guid(const std::string& g)
{
	guid_ = g;
}

void rss_item::set_unread_nowrite(bool u)
{
	unread_ = u;
}

void rss_item::set_unread_nowrite_notify(bool u, bool notify)
{
	unread_ = u;
	std::shared_ptr<rss_feed> feedptr = feedptr_.lock();
	if (feedptr && notify) {
		feedptr->get_item_by_guid(guid_)->set_unread_nowrite(
			unread_); // notify parent feed
	}
}

void rss_item::set_unread(bool u)
{
	if (unread_ != u) {
		bool old_u = unread_;
		unread_ = u;
		std::shared_ptr<rss_feed> feedptr = feedptr_.lock();
		if (feedptr)
			feedptr->get_item_by_guid(guid_)->set_unread_nowrite(
				unread_); // notify parent feed
		try {
			if (ch) {
				ch->update_rssitem_unread_and_enqueued(
					this, feedurl_);
			}
		} catch (const dbexception& e) {
			// if the update failed, restore the old unread flag and
			// rethrow the exception
			unread_ = old_u;
			throw;
		}
	}
}

std::string rss_item::pubDate() const
{
	char text[1024];
	strftime(text,
		sizeof(text),
		_("%a, %d %b %Y %T %z"),
		localtime(&pubDate_));
	return std::string(text);
}

unsigned int rss_feed::unread_item_count()
{
	std::lock_guard<std::mutex> lock(item_mutex);
	return std::count_if(items_.begin(),
		items_.end(),
		[](const std::shared_ptr<rss_item>& item) {
			return item->unread();
		});
}

bool rss_feed::matches_tag(const std::string& tag)
{
	return std::find_if(
		       tags_.begin(), tags_.end(), [&](const std::string& t) {
			       return tag == t;
		       }) != tags_.end();
}

std::string rss_feed::get_firsttag()
{
	return tags_.empty() ? "" : tags_.front();
}

std::string rss_feed::get_tags()
{
	std::string tags;
	for (const auto& t : tags_) {
		if (t.substr(0, 1) != "~" && t.substr(0, 1) != "!") {
			tags.append(t);
			tags.append(" ");
		}
	}
	return tags;
}

void rss_feed::set_tags(const std::vector<std::string>& tags)
{
	tags_ = tags;
}

void rss_item::set_enclosure_url(const std::string& url)
{
	enclosure_url_ = url;
}

void rss_item::set_enclosure_type(const std::string& type)
{
	enclosure_type_ = type;
}

std::string rss_item::title() const
{
	std::string retval;
	if (title_.length() > 0)
		retval = utils::convert_text(
			title_, nl_langinfo(CODESET), "utf-8");
	return retval;
}

std::string rss_item::author() const
{
	return utils::convert_text(author_, nl_langinfo(CODESET), "utf-8");
}

std::string rss_item::description() const
{
	return utils::convert_text(description_, nl_langinfo(CODESET), "utf-8");
}

std::string rss_feed::title() const
{
	bool found_title = false;
	std::string alt_title;
	for (const auto& tag : tags_) {
		if (tag.substr(0, 1) == "~" || tag.substr(0, 1) == "!") {
			found_title = true;
			alt_title = tag.substr(1, tag.length() - 1);
			break;
		}
	}
	return found_title
		? alt_title
		: utils::convert_text(title_, nl_langinfo(CODESET), "utf-8");
}

std::string rss_feed::description() const
{
	return utils::convert_text(description_, nl_langinfo(CODESET), "utf-8");
}

bool rss_feed::hidden() const
{
	return std::any_of(tags_.begin(),
		tags_.end(),
		[](const std::string& tag) { return tag.substr(0, 1) == "!"; });
}

std::shared_ptr<rss_item> rss_feed::get_item_by_guid(const std::string& guid)
{
	std::lock_guard<std::mutex> lock(item_mutex);
	return get_item_by_guid_unlocked(guid);
}

std::shared_ptr<rss_item> rss_feed::get_item_by_guid_unlocked(
	const std::string& guid)
{
	auto it = items_guid_map.find(guid);
	if (it != items_guid_map.end()) {
		return it->second;
	}
	LOG(Level::DEBUG,
		"rss_feed::get_item_by_guid_unlocked: hit dummy item!");
	LOG(Level::DEBUG,
		"rss_feed::get_item_by_guid_unlocked: items_guid_map.size = %d",
		items_guid_map.size());
	// abort();
	return std::shared_ptr<rss_item>(
		new rss_item(ch)); // should never happen!
}

bool rss_item::has_attribute(const std::string& attribname)
{
	if (attribname == "title" || attribname == "link" ||
		attribname == "author" || attribname == "content" ||
		attribname == "date" || attribname == "guid" ||
		attribname == "unread" || attribname == "enclosure_url" ||
		attribname == "enclosure_type" || attribname == "flags" ||
		attribname == "age" || attribname == "articleindex")
		return true;

	// if we have a feed, then forward the request
	std::shared_ptr<rss_feed> feedptr = feedptr_.lock();
	if (feedptr)
		return feedptr->rss_feed::has_attribute(attribname);

	return false;
}

std::string rss_item::get_attribute(const std::string& attribname)
{
	if (attribname == "title")
		return title();
	else if (attribname == "link")
		return link();
	else if (attribname == "author")
		return author();
	else if (attribname == "content")
		return description();
	else if (attribname == "date")
		return pubDate();
	else if (attribname == "guid")
		return guid();
	else if (attribname == "unread")
		return unread_ ? "yes" : "no";
	else if (attribname == "enclosure_url")
		return enclosure_url();
	else if (attribname == "enclosure_type")
		return enclosure_type();
	else if (attribname == "flags")
		return flags();
	else if (attribname == "age")
		return std::to_string(
			(time(nullptr) - pubDate_timestamp()) / 86400);
	else if (attribname == "articleindex")
		return std::to_string(idx);

	// if we have a feed, then forward the request
	std::shared_ptr<rss_feed> feedptr = feedptr_.lock();
	if (feedptr)
		return feedptr->rss_feed::get_attribute(attribname);

	return "";
}

void rss_item::update_flags()
{
	if (ch) {
		ch->update_rssitem_flags(this);
	}
}

void rss_item::set_flags(const std::string& ff)
{
	oldflags_ = flags_;
	flags_ = ff;
	sort_flags();
}

void rss_item::sort_flags()
{
	std::sort(flags_.begin(), flags_.end());

	// Erase non-alpha characters
	flags_.erase(std::remove_if(flags_.begin(),
			     flags_.end(),
			     [](const char c) { return !isalpha(c); }),
		flags_.end());

	// Erase doubled characters
	flags_.erase(std::unique(flags_.begin(), flags_.end()), flags_.end());
}

bool rss_feed::has_attribute(const std::string& attribname)
{
	if (attribname == "feedtitle" || attribname == "description" ||
		attribname == "feedlink" || attribname == "feeddate" ||
		attribname == "rssurl" || attribname == "unread_count" ||
		attribname == "total_count" || attribname == "tags" ||
		attribname == "feedindex")
		return true;
	return false;
}

std::string rss_feed::get_attribute(const std::string& attribname)
{
	if (attribname == "feedtitle")
		return title();
	else if (attribname == "description")
		return description();
	else if (attribname == "feedlink")
		return title();
	else if (attribname == "feeddate")
		return pubDate();
	else if (attribname == "rssurl")
		return rssurl();
	else if (attribname == "unread_count") {
		return std::to_string(unread_item_count());
	} else if (attribname == "total_count") {
		return std::to_string(items_.size());
	} else if (attribname == "tags") {
		return get_tags();
	} else if (attribname == "feedindex") {
		return std::to_string(idx);
	}
	return "";
}

void rss_ignores::handle_action(const std::string& action,
	const std::vector<std::string>& params)
{
	if (action == "ignore-article") {
		if (params.size() < 2)
			throw confighandlerexception(
				ActionHandlerStatus::TOO_FEW_PARAMS);
		std::string ignore_rssurl = params[0];
		std::string ignore_expr = params[1];
		matcher m;
		if (!m.parse(ignore_expr))
			throw confighandlerexception(strprintf::fmt(
				_("couldn't parse filter expression `%s': %s"),
				ignore_expr,
				m.get_parse_error()));
		ignores.push_back(feedurl_expr_pair(
			ignore_rssurl, new matcher(ignore_expr)));
	} else if (action == "always-download") {
		for (const auto& param : params) {
			ignores_lastmodified.push_back(param);
		}
	} else if (action == "reset-unread-on-update") {
		for (const auto& param : params) {
			resetflag.push_back(param);
		}
	} else
		throw confighandlerexception(
			ActionHandlerStatus::INVALID_COMMAND);
}

void rss_ignores::dump_config(std::vector<std::string>& config_output)
{
	for (const auto& ign : ignores) {
		std::string configline = "ignore-article ";
		if (ign.first == "*")
			configline.append("*");
		else
			configline.append(utils::quote(ign.first));
		configline.append(" ");
		configline.append(utils::quote(ign.second->get_expression()));
		config_output.push_back(configline);
	}
	for (const auto& ign_lm : ignores_lastmodified) {
		config_output.push_back(strprintf::fmt(
			"always-download %s", utils::quote(ign_lm)));
	}
	for (const auto& rf : resetflag) {
		config_output.push_back(strprintf::fmt(
			"reset-unread-on-update %s", utils::quote(rf)));
	}
}

rss_ignores::~rss_ignores()
{
	for (const auto& ign : ignores) {
		delete ign.second;
	}
}

bool rss_ignores::matches(rss_item* item)
{
	for (const auto& ign : ignores) {
		LOG(Level::DEBUG,
			"rss_ignores::matches: ign.first = `%s' item->feedurl "
			"= "
			"`%s'",
			ign.first,
			item->feedurl());
		if (ign.first == "*" || item->feedurl() == ign.first) {
			if (ign.second->matches(item)) {
				LOG(Level::DEBUG,
					"rss_ignores::matches: found match");
				return true;
			}
		}
	}
	return false;
}

bool rss_ignores::matches_lastmodified(const std::string& url)
{
	return std::find_if(ignores_lastmodified.begin(),
		       ignores_lastmodified.end(),
		       [&](const std::string& u) { return u == url; }) !=
		ignores_lastmodified.end();
}

bool rss_ignores::matches_resetunread(const std::string& url)
{
	return std::find_if(resetflag.begin(),
		       resetflag.end(),
		       [&](const std::string& u) { return u == url; }) !=
		resetflag.end();
}

void rss_feed::update_items(std::vector<std::shared_ptr<rss_feed>> feeds)
{
	std::lock_guard<std::mutex> lock(item_mutex);
	if (query.length() == 0)
		return;

	LOG(Level::DEBUG, "rss_feed::update_items: query = `%s'", query);

	struct timeval tv1, tv2, tvx;
	gettimeofday(&tv1, nullptr);

	matcher m(query);

	items_.clear();
	items_guid_map.clear();

	for (const auto& feed : feeds) {
		if (!feed->is_query_feed()) { // don't fetch items from other query feeds!
			for (const auto& item : feed->items()) {
				if (m.matches(item.get())) {
					LOG(Level::DEBUG,
						"rss_feed::update_items: "
						"matcher "
						"matches!");
					item->set_feedptr(feed);
					items_.push_back(item);
					items_guid_map[item->guid()] = item;
				}
			}
		}
	}

	gettimeofday(&tvx, nullptr);

	std::sort(items_.begin(), items_.end());

	gettimeofday(&tv2, nullptr);
	unsigned long diff =
		(((tv2.tv_sec - tv1.tv_sec) * 1000000) + tv2.tv_usec) -
		tv1.tv_usec;
	unsigned long diffx =
		(((tv2.tv_sec - tvx.tv_sec) * 1000000) + tv2.tv_usec) -
		tvx.tv_usec;
	LOG(Level::DEBUG,
		"rss_feed::update_items matching took %lu.%06lu s",
		diff / 1000000,
		diff % 1000000);
	LOG(Level::DEBUG,
		"rss_feed::update_items sorting took %lu.%06lu s",
		diffx / 1000000,
		diffx % 1000000);
}

void rss_feed::set_rssurl(const std::string& u)
{
	rssurl_ = u;
	if (utils::is_query_url(u)) {
		/* Query string looks like this:
		 *
		 * query:Title:unread = "yes" and age between 0:7
		 *
		 * So we split by colons to get title and the query itself. */
		std::vector<std::string> tokens =
			utils::tokenize_quoted(u, ":");

		if (tokens.size() < 3) {
			throw _s("too few arguments");
		}

		/* "Between" operator requires a range, which contains a colon.
		 * Since we've been tokenizing by colon, we might've
		 * inadertently split the query itself. Let's reconstruct it! */
		auto query = tokens[2];
		for (auto it = tokens.begin() + 3; it != tokens.end(); ++it) {
			query += ":";
			query += *it;
		}
		// Have to check if the result is a valid query, just in case
		matcher m;
		if (!m.parse(query)) {
			throw strprintf::fmt(
				_("`%s' is not a valid filter expression"),
				query);
		}

		LOG(Level::DEBUG,
			"rss_feed::set_rssurl: query name = `%s' expr = `%s'",
			tokens[1],
			query);

		set_title(tokens[1]);
		set_query(query);
	}
}

void rss_feed::sort(const ArticleSortStrategy& sort_strategy)
{
	std::lock_guard<std::mutex> lock(item_mutex);
	sort_unlocked(sort_strategy);
}

void rss_feed::sort_unlocked(const ArticleSortStrategy& sort_strategy)
{
	switch (sort_strategy.sm) {
	case ArtSortMethod::TITLE:
		std::stable_sort(items_.begin(),
			items_.end(),
			[&](std::shared_ptr<rss_item> a,
				std::shared_ptr<rss_item> b) {
				return sort_strategy.sd ==
						SortDirection::DESC
					? (utils::strnaturalcmp(a->title().c_str(),
						   b->title().c_str()) > 0)
					: (utils::strnaturalcmp(a->title().c_str(),
						   b->title().c_str()) < 0);
			});
		break;
	case ArtSortMethod::FLAGS:
		std::stable_sort(items_.begin(),
			items_.end(),
			[&](std::shared_ptr<rss_item> a,
				std::shared_ptr<rss_item> b) {
				return sort_strategy.sd ==
						SortDirection::DESC
					? (strcmp(a->flags().c_str(),
						   b->flags().c_str()) > 0)
					: (strcmp(a->flags().c_str(),
						   b->flags().c_str()) < 0);
			});
		break;
	case ArtSortMethod::AUTHOR:
		std::stable_sort(items_.begin(),
			items_.end(),
			[&](std::shared_ptr<rss_item> a,
				std::shared_ptr<rss_item> b) {
				return sort_strategy.sd ==
						SortDirection::DESC
					? (strcmp(a->author().c_str(),
						   b->author().c_str()) > 0)
					: (strcmp(a->author().c_str(),
						   b->author().c_str()) < 0);
			});
		break;
	case ArtSortMethod::LINK:
		std::stable_sort(items_.begin(),
			items_.end(),
			[&](std::shared_ptr<rss_item> a,
				std::shared_ptr<rss_item> b) {
				return sort_strategy.sd ==
						SortDirection::DESC
					? (strcmp(a->link().c_str(),
						   b->link().c_str()) > 0)
					: (strcmp(a->link().c_str(),
						   b->link().c_str()) < 0);
			});
		break;
	case ArtSortMethod::GUID:
		std::stable_sort(items_.begin(),
			items_.end(),
			[&](std::shared_ptr<rss_item> a,
				std::shared_ptr<rss_item> b) {
				return sort_strategy.sd ==
						SortDirection::DESC
					? (strcmp(a->guid().c_str(),
						   b->guid().c_str()) > 0)
					: (strcmp(a->guid().c_str(),
						   b->guid().c_str()) < 0);
			});
		break;
	case ArtSortMethod::DATE:
		std::stable_sort(items_.begin(),
			items_.end(),
			[&](std::shared_ptr<rss_item> a,
				std::shared_ptr<rss_item> b) {
				// date is descending by default
				return sort_strategy.sd == SortDirection::ASC
					? (a->pubDate_timestamp() >
						  b->pubDate_timestamp())
					: (a->pubDate_timestamp() <
						  b->pubDate_timestamp());
			});
		break;
	}
}

void rss_feed::remove_old_deleted_items()
{
	std::lock_guard<std::mutex> lock(item_mutex);
	std::vector<std::string> guids;
	for (const auto& item : items_) {
		guids.push_back(item->guid());
	}
	ch->remove_old_deleted_items(rssurl_, guids);
}

void rss_feed::purge_deleted_items()
{
	std::lock_guard<std::mutex> lock(item_mutex);
	scope_measure m1("rss_feed::purge_deleted_items");

	// Purge in items_guid_map
	{
		std::lock_guard<std::mutex> lock2(items_guid_map_mutex);
		for (const auto& item : items_) {
			if (item->deleted()) {
				items_guid_map.erase(item->guid());
			}
		}
	}

	items_.erase(std::remove_if(items_.begin(),
			     items_.end(),
			     [](const std::shared_ptr<rss_item> item) {
				     return item->deleted();
			     }),
		items_.end());
}

void rss_feed::set_feedptrs(std::shared_ptr<rss_feed> self)
{
	std::lock_guard<std::mutex> lock(item_mutex);
	for (const auto& item : items_) {
		item->set_feedptr(self);
	}
}

void rss_item::set_feedptr(std::shared_ptr<rss_feed> ptr)
{
	feedptr_ = std::weak_ptr<rss_feed>(ptr);
}

std::string rss_feed::get_status()
{
	switch (status_) {
	case DlStatus::SUCCESS:
		return " ";
	case DlStatus::TO_BE_DOWNLOADED:
		return "_";
	case DlStatus::DURING_DOWNLOAD:
		return ".";
	case DlStatus::DL_ERROR:
		return "x";
	}
	return "?";
}

void rss_feed::unload()
{
	std::lock_guard<std::mutex> lock(item_mutex);
	for (const auto& item : items_) {
		item->unload();
	}
}

void rss_feed::load()
{
	std::lock_guard<std::mutex> lock(item_mutex);
	ch->fetch_descriptions(this);
}

void rss_feed::mark_all_items_read()
{
	std::lock_guard<std::mutex> lock(item_mutex);
	for (const auto& item : items_) {
		item->set_unread_nowrite(false);
	}
}

} // namespace newsboat
