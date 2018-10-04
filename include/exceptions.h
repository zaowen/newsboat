#ifndef NEWSBOAT_EXCEPTIONS_H_
#define NEWSBOAT_EXCEPTIONS_H_

#include <sqlite3.h>
#include <stdexcept>
#include <string>

#include "configparser.h"

namespace newsboat {

class xmlexception : public std::exception {
public:
	explicit xmlexception(const std::string& errmsg)
		: msg(errmsg)
	{
	}
	~xmlexception() throw() override {}
	const char* what() const throw() override
	{
		return msg.c_str();
	}

private:
	std::string msg;
};

class configexception : public std::exception {
public:
	explicit configexception(const std::string& errmsg)
		: msg(errmsg)
	{
	}
	~configexception() throw() override {}
	const char* what() const throw() override
	{
		return msg.c_str();
	}

private:
	std::string msg;
};

class confighandlerexception : public std::exception {
public:
	explicit confighandlerexception(const std::string& emsg)
		: msg(emsg)
	{
	}
	explicit confighandlerexception(ActionHandlerStatus e);
	~confighandlerexception() throw() override {}
	const char* what() const throw() override
	{
		return msg.c_str();
	}
	int status()
	{
		return 0;
	}

private:
	const char* get_errmsg(ActionHandlerStatus e);
	std::string msg;
};

class dbexception : public std::exception {
public:
	explicit dbexception(sqlite3* h)
		: msg(sqlite3_errmsg(h))
	{
	}
	~dbexception() throw() override {}
	const char* what() const throw() override
	{
		return msg.c_str();
	}

private:
	std::string msg;
};

class matcherexception : public std::exception {
public:
	enum class Type { ATTRIB_UNAVAIL, INVALID_REGEX };

	matcherexception(Type et,
		const std::string& info,
		const std::string& info2 = "")
		: type_(et)
		, addinfo(info)
		, addinfo2(info2)
	{
	}

	~matcherexception() throw() override {}
	const char* what() const throw() override;

private:
	Type type_;
	std::string addinfo;
	std::string addinfo2;
};

} // namespace newsboat

#endif /* NEWSBOAT_EXCEPTIONS_H_ */
