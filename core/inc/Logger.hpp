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
    
enum LogID : char {
    LOG_DEBUG   = 0,
    LOG_INFO    = 1,
    LOG_WARNING = 2,
    LOG_ERROR   = 3,
    LOG_FATAL   = 4,
};

const char *LogID_str(LogID ll);

using LogHook = std::function<void(LogID, const std::string&)>;

class Logger {
    using Log = std::pair<LogID, std::string>;
    using LogBuffer = std::deque<Log>;

public:
    static
    std::shared_ptr<Logger> make(const std::string &targetfile,
                                 LogID maxloglevel,
                                 std::size_t buffersize = 10);

    ~Logger(void);

    /*Disable unwanted mutability*/
    //Logger(const Logger& l) = delete;
    //Logger(Logger&& l) = delete;
    //bool operator=(const Logger& l) = delete;
    //bool operator==(const Logger& l) = delete;

    void add_log_hook(LogHook _hook);
    void clear_logfile(void);
    bool log(LogID _type, const std::string& _msg);
    bool timestamped_log(LogID _type, const std::string& _msg);
    [[nodiscard]] std::size_t buffer_size(void);

private:
    void try_write_to_target(void);
    void buffer_put(Log log);

    std::string m_log_target = "";
    LogBuffer m_buffer;
    std::vector<LogHook> m_log_hooks{};
    size_t m_max_buffer_size = 10;
    LogID m_max_level = LOG_INFO;
};
    
}; /*ns*/
}; /*ns*/
