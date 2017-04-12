#pragma once
#include <string>
#include <algorithm>
#include "boost/program_options.hpp"

class CTest
{
public:
	static CTest* new_test() {
		return nullptr;
	}

	virtual ~CTest() {
	}

	virtual void init(const boost::program_options::variables_map &args) {
		if (args.count("out")) {
			m_outfile = args["out"].as<std::string>();
		}
		if (args.count("param")) {
			m_params = args["param"].as<std::vector<std::string>>();
		}
	}

	virtual void run() {
	}

	bool has_param(const std::string &name) const {
		auto it = std::find(m_params.begin(), m_params.end(), name);
		return it != m_params.end();
	}

	bool get_param(const std::string &name, std::string &value) const {
		if (name.empty()) {
			return false;
		}
		size_t sz = name.size();
		for (auto &v : m_params) {
			if (v.size() < sz || 0 != strncmp(v.c_str(), name.c_str(), sz))
				continue;
			if (v.size() == sz)
				return true;
			else if (v[sz] == '=')
			{
				value = v.substr(sz + 1);
				return true;
			}
		}
		return false;
	}

protected:
	std::string m_outfile;
	std::vector<std::string> m_params;
};
