#include "fasta_utils_lib/logging.h"

namespace fst { namespace ut { namespace log {

// Initialize console logger
void init_console_log() {
    // create sink to stdout
    boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();
    sink->locked_backend()->add_stream(
        boost::shared_ptr<std::ostream>(&std::cout, boost::null_deleter()));

    // flush
    sink->locked_backend()->auto_flush(true);

    // format sink
    //sink->set_formatter
    //(
        /// TODO: add custom formatting
    //);

    // filter
    // TODO: add any filters

    // register sink
    boost::log::core::get()->add_sink(sink);
}

// Initialize file logger
void init_file_log(const std::string& logfilename) {
    // create sink to logfile
    boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();
    sink->locked_backend()->add_stream(
        boost::make_shared<std::ofstream>(logfilename.c_str()));

    // flush
    sink->locked_backend()->auto_flush(true);

    // format sink
    //sink->set_formatter
    //(
        /// TODO: add custom formatting
    //);

    // filter
    // TODO: add any filters

    // register sink
    boost::log::core::get()->add_sink(sink);
}

}}} // namespace