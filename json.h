#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <regex>
#include <algorithm>
#include <cctype>
#include <iterator>
#include <map>

namespace Json {
	typedef std::pair<std::string, std::string> JSONPair;
	typedef std::map<std::string, std::string> JSONMap;

	inline bool IsNumber(const std::string & s)
	{
		if (s.empty()) { return false; }
		std::stringstream ss;
		double d;
		ss << s;
		ss >> d;
		return (ss.good() || ss.eof());
	}

	inline bool IsEmpty(const std::string & s)
	{
		return std::find_if(s.begin(), s.end(), [](int ch) {
			return !std::isspace(ch);
		}) == s.end();
	}

	inline void Trim(std::string & s)
	{
		if (IsEmpty(s)) {
			s = "";
		}
		else {
			s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
				return !std::isspace(ch);
			})); // Trim left

			s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
				return !std::isspace(ch);
			}).base(), s.end()); // Trim right
		}
	}

	inline std::string __RemoveSurroundingSymbols(std::string s, const char& l, const char& r = '"')
	{
		Trim(s);
		if (s.size() > 1 && *s.begin() == l && *s.rbegin() == r) {
			s = s.substr(1, s.length() - 2);
		}
		Trim(s);
		return s;
	}

	inline std::string RemoveQuotes(std::string s)
	{
		return __RemoveSurroundingSymbols(s, '"');
	}

	inline std::string RemoveBrackets(std::string s)
	{
		return __RemoveSurroundingSymbols(s, '{', '}');
	}

	std::istream & operator>>(std::istream & stream, JSONPair& p)
	{
		std::string s;
		std::getline(stream, s, ':');
		std::string formatted = RemoveQuotes(s);
		if (formatted == s || formatted.empty()) {
			throw std::exception("Data parsing error.\n", 447453);
		}
		p.first = formatted;

		std::getline(stream, s, ':');
		Trim(s);
		if (IsNumber(s)) {
			p.second = s;
		}
		else {
			formatted = RemoveQuotes(s);
			if (formatted == s) {
				throw std::exception("Data parsing error.\n", 447453);
			}
			p.second = formatted;
		}

		return stream;
	}

	std::ostream & operator <<(std::ostream &stream, const Json::JSONMap &m) {
		Json::JSONMap::const_iterator it;
		for (it = m.cbegin(); it != m.cend(); it++)
		{
			if (it != m.cbegin()) { stream << ','; }
			stream << '"' << it->first << "\":\"" << it->second << '"';
		}
		return stream;
	}

	std::istream& __Getline(std::istream & stream, JSONPair& p, char delim)
	{
		std::string s;
		if (getline(stream, s, delim)) {
			std::stringstream ss(s);
			ss >> p;
		}
		return stream;
	}

	std::string SeparateValueFromData(std::string &str) {
		std::string match = "";
		std::regex rxp("\"value\":\"((?:(?=\\\\?)\\\\?.)*?)\"");
		std::smatch mr;
		if (std::regex_search(str, mr, rxp)) {
			std::string match = mr[1].str();
			std::string prefix = "\"value\":\"";
			size_t pos = str.find(prefix + match) + prefix.length();
			str.erase(pos, match.length());
		}
		return match;
	}

	JSONMap CreateMapFromJSON(std::string str)
	{
		JSONPair p;
		JSONMap m;
		std::string value = SeparateValueFromData(str);
		std::stringstream ss(RemoveBrackets(str));
		while (__Getline(ss, p, ',')) {
			m.insert(p);
		}
		if (!value.empty()) {
			m.at("value") = value;
		}
		return m;
	}

	std::string GenerateUserJSON(std::string cmd, std::string name) {
		JSONMap m;
		m.insert(std::make_pair("object", "user"));
		m.insert(std::make_pair("command", cmd));
		m.insert(std::make_pair("user", name));
		std::stringstream ss;
		ss << '{' << m << '}';
		return ss.str();
	}
}
