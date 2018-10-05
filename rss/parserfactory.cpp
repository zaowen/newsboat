#include "rssppinternal.h"

#include "config.h"

namespace rsspp {

std::shared_ptr<RssParser> RssParserFactory::get_object(Feed& f,
	xmlDocPtr doc)
{
	switch (f.rss_version) {
	case RSS_0_91:
	case RSS_0_92:
	case RSS_0_94:
		return std::shared_ptr<RssParser>(new rss_09x_parser(doc));
	case RSS_2_0:
		return std::shared_ptr<RssParser>(new rss_20_parser(doc));
	case RSS_1_0:
		return std::shared_ptr<RssParser>(new rss_10_parser(doc));
	case ATOM_0_3:
	case ATOM_0_3_NONS:
	case ATOM_1_0:
		return std::shared_ptr<RssParser>(new atom_parser(doc));
	case UNKNOWN:
	default:
		throw Exception(_("unsupported feed format"));
	}
}

} // namespace rsspp
