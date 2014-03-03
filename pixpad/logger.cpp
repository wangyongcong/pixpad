#include <boost/move/utility.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include "logger.h"

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(app_logger, src::logger_mt)

namespace wyc
{

void init_logger(const char *log_file_name)
{
	logging::add_file_log
	(
		keywords::file_name = "sample_%N.log",											/*< file name pattern >*/
		keywords::rotation_size = 10 * 1024 * 1024,										/*< rotate files every 10 MiB... >*/
		keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),	/*< ...or at midnight >*/
		keywords::format = "[%TimeStamp%]: %Message%"									/*< log record format >*/
	);

/*	logging::core::get()->set_filter
	(
		logging::trivial::severity >= logging::trivial::info
	);*/

	logging::add_common_attributes();
}

void print(const char *message)
{
	src::logger lg;
	BOOST_LOG(lg) << message;
}

}; // end of namespace wyc