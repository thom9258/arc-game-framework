#include <iostream>
#include <fstream>
#include <sstream>

#include "../inc/Logger.hpp"

namespace arc {
namespace core {

const char*
LogID_str(LogID id) {
    switch (id) {
    case LOG_INFO:    return "INFO   ";
    case LOG_DEBUG:   return "DEBUG  ";
    case LOG_WARNING: return "WARNING";
    case LOG_ERROR:   return "ERROR  ";
    case LOG_FATAL:   return "FATAL  ";
    default:
    return                   "UNKNOWN";
    }
}

std::shared_ptr<Logger> Logger::make(const std::string &targetfile,
                                     std::size_t buffersize,
                                     LogID maxloglevel)
{
    auto ins = std::make_shared<Logger>();
    ins->m_log_target = targetfile;
    ins->m_max_buffer_size = buffersize;
    ins->m_max_level = maxloglevel;
    return ins;
}

void Logger::try_write_to_target(void) {
    if (m_log_target == "")
        return;
    std::ofstream file;
    file.open(m_log_target, std::ios::out | std::ios::app);
    if (file.fail())
        return;
    while (m_buffer.size() > 0) {
        //Log popped = m_buffer.front();
        auto [id, msg] = m_buffer.front();
        file << "[" <<  LogID_str(id) << "] " << msg << std::endl;
        m_buffer.pop_front();
        //file << "[" <<  LogID_str(popped.first) << "] " << popped.second << std::endl;
    }
}

Logger::~Logger(void) { try_write_to_target(); }

void Logger::clear_logfile(void) {
    m_buffer = {};
    if (m_log_target == "")
        return;
    std::ofstream file;
    file.open(m_log_target, std::ofstream::out | std::ofstream::trunc);
}

void Logger::add_log_hook(LogHook _hook) { m_log_hooks.push_back(_hook); }

void Logger::buffer_put(Log _log) {
    m_buffer.push_back(_log);
    if (m_buffer.size() >= m_max_buffer_size) {
        try_write_to_target();
    }
}

std::string Logger::generate_timestamp(void) {
    std::stringstream ss{};
    auto t = time(0);
    tm* now = localtime(&t);
    ss << "(" << 1900 + now->tm_year << "/" << 1 + now->tm_mon << "/"
       << now->tm_mday << " " << 5 + now->tm_hour << ":" << 30 + now->tm_min
       << ":" << now->tm_sec << ")";
    return ss.str();
}

size_t Logger::buffer_size(void) { return m_buffer.size(); }

bool Logger::log(LogID _id, const std::string& _msg, bool timestamp) {
    std::string msg{};
    if (_id < m_max_level)
        return false;
    if (timestamp)
        msg += generate_timestamp() + "  ";
    msg += _msg;
    for (auto hook : m_log_hooks)
        hook(_id, msg);
    buffer_put({_id, msg});
    return true;
}

bool Logger::info(const std::string& _msg, bool timestamp) {
    return log(LOG_INFO, _msg, timestamp);
}

bool Logger::debug(const std::string& _msg, bool timestamp) {
    return log(LOG_DEBUG, _msg, timestamp);
}

bool Logger::warn(const std::string& _msg, bool timestamp) {
    return log(LOG_WARNING, _msg, timestamp);
}

bool Logger::error(const std::string& _msg, bool timestamp) {
    return log(LOG_ERROR, _msg, timestamp);
}

bool Logger::fatal(const std::string& _msg, bool timestamp) {
    return log(LOG_FATAL, _msg, timestamp);
}

} /*ns*/
} /*ns*/
