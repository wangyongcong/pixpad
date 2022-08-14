#pragma once
#include <string>

namespace wyc
{
	class IShellCommand
	{
	public:
		// command name
		virtual const std::string& name() const = 0;
		// short description
		virtual const std::string& description() const = 0;
		// execute command with std args
		virtual bool execute(int argc, char *argv[]) = 0;
		// execute command with string args
		virtual bool execute(const std::string &cmdline) = 0;
	};

} // namespace wyc

#ifdef WYC_SHELLCMD_IMPLEMENTATION
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include "stb/stb_log.h"

namespace po = boost::program_options;

namespace wyc
{
	class CShellCommand : public IShellCommand
	{
	public:
		CShellCommand(const char* name, const char *desc)
			: m_name(name)
			, m_desc(desc)
			, m_opt("")
			, m_pos_opt()
		{
		}

		virtual const std::string& name() const override {
			return m_name;
		}

		virtual const std::string& description() const override {
			return m_desc;
		}

		virtual bool execute(int argc, char *argv[]) override
		{
			po::variables_map args;
			if (!parse(argc, argv, args))
				return false;
			if (args.count("help")) {
				show_help();
				return true;
			}
			return process(args);
		}

		virtual bool execute(const std::string &cmdline) override {
			po::variables_map args;
			if (!parse(cmdline, args))
				return false;
			if (args.count("help")) {
				show_help();
				return true;
			}
			return process(args);
		}

		virtual bool process(const po::variables_map &args)
		{
			return true;
		}

		virtual void show_help() const
		{
			std::stringstream ss;
			ss << m_desc << std::endl << m_opt;
			log_info(ss.str().c_str());
		}

	protected:
		std::string m_name;
		std::string m_desc;
		po::options_description m_opt;
		po::positional_options_description m_pos_opt;

		bool parse(int argc, char *argv[], po::variables_map &args_table)
		{
			try {
				po::store(po::command_line_parser(argc, argv).options(m_opt).positional(m_pos_opt).run(), args_table);
				po::notify(args_table);
			}
			catch (const po::error &exp) {
				log_error(exp.what());
				show_help();
				return false;
			}
			return true;
		}

		bool parse(const std::string &cmd_line, po::variables_map &args_table)
		{
			std::vector<std::string> result;
			boost::escaped_list_separator<char> seperator("\\", "= ", "\"\'");
			boost::tokenizer<decltype(seperator)> tokens(cmd_line, seperator);
			for (auto &tok : tokens)
			{
				if (!tok.empty())
					result.push_back(tok);
			}
			try {
				po::store(po::command_line_parser(result).options(m_opt).positional(m_pos_opt).run(), args_table);
				po::notify(args_table);
			}
			catch (const po::error &exp) {
				log_error(exp.what());
				show_help();
				return false;
			}
			return true;
		}
	};
} // namespace wyc

#endif // WYC_SHELLCMD_IMPLEMENTATION
