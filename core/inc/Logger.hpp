#pragma once

#include <string>
#include <deque>
#include <vector>
#include <functional>
#include <memory>
#include <tuple>

#include "Defs.hpp"

namespace arc {
namespace core {
    
enum LogID {
    LOG_EVERYTHING = 0,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL = 99,
};

const char *LogID_str(LogID id);

using LogHook = std::function<void(LogID, const std::string&)>;

class Logger {
    using Log = std::pair<LogID, std::string>;

public:
    static
    std::shared_ptr<Logger> make(const std::string &targetfile,
                                 std::size_t buffersize,
                                 LogID maxloglevel);
    ~Logger(void);
    void add_log_hook(LogHook _hook);
    void clear_logfile(void);
    bool log(LogID _type, const std::string& _msg, bool timestamp=true);
    bool info(const std::string& _msg, bool timestamp=true);
    bool debug(const std::string& _msg, bool timestamp=true);
    bool warn(const std::string& _msg, bool timestamp=true);
    bool error(const std::string& _msg, bool timestamp=true);
    bool fatal(const std::string& _msg, bool timestamp=true);
    [[nodiscard]] std::size_t buffer_size(void);

private:
    std::string generate_timestamp(void);
    void try_write_to_target(void);
    void buffer_put(Log log);

    std::string m_log_target{""};
    std::deque<Log> m_buffer{};
    std::vector<LogHook> m_log_hooks{};
    size_t m_max_buffer_size{0};
    LogID m_max_level{};
};
    
} /*ns*/
} /*ns*/
