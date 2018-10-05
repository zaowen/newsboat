#ifndef NEWSBOAT_GOOGLEAPI_H_
#define NEWSBOAT_GOOGLEAPI_H_

#include "cache.h"
#include "remoteapi.h"
#include "urlreader.h"

namespace newsboat {

class OldReaderApi : public RemoteApi {
public:
	explicit OldReaderApi(configcontainer* c);
	~OldReaderApi() override;
	bool authenticate() override;
	std::vector<tagged_feedurl> get_subscribed_urls() override;
	void add_custom_headers(curl_slist** custom_headers) override;
	bool mark_all_read(const std::string& feedurl) override;
	bool mark_article_read(const std::string& guid, bool read) override;
	bool update_article_flags(const std::string& oldflags,
		const std::string& newflags,
		const std::string& guid) override;

private:
	std::vector<std::string> get_tags(xmlNode* node);
	std::string get_new_token();
	std::string retrieve_auth();
	std::string post_content(const std::string& url,
		const std::string& postdata);
	bool star_article(const std::string& guid, bool star);
	bool share_article(const std::string& guid, bool share);
	bool mark_article_read_with_token(const std::string& guid,
		bool read,
		const std::string& token);
	std::string auth;
	std::string auth_header;
};

class OldReaderUrlReader : public UrlReader {
public:
	OldReaderUrlReader(configcontainer* c,
		const std::string& url_file,
		RemoteApi* a);
	~OldReaderUrlReader() override;
	void write_config() override;
	void reload() override;
	std::string get_source() override;

private:
	configcontainer* cfg;
	std::string file;
	RemoteApi* api;
};

} // namespace newsboat

#endif /* NEWSBOAT_GOOGLEAPI_H_ */
