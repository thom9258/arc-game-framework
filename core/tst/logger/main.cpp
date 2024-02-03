#include <iostream>

#include "../testlib.h"

//#include <ArcCore/Logger.hpp>
#include "../../../core/inc/Logger.hpp"

void test_logger(void) {
    bool was_fatal = false;
    int total_logs = 0;
    auto logger = arc::core::Logger::make("logtest.txt", 3, arc::core::LOG_EVERYTHING);

    auto logprint = [&]() {
        std::cout << "buffered logs: " << logger->buffer_size() << "  total logs: " << total_logs << std::endl;
    };
    logger->clear_logfile();
    
    logger->add_log_hook([&](auto type, auto msg) {
        total_logs++;
        std::cout << "[" << arc::core::LogID_str(type) << "] " << msg << std::endl;
        if (type == arc::core::LOG_FATAL)
            was_fatal = true;});
    
    logger->log(arc::core::LOG_INFO, "something cool.");
    logprint();
    TL_TEST(logger->buffer_size() == 1);
    TL_TEST(total_logs == 1);
    
    logger->warn("shat almost broke!");
    logprint();
    TL_TEST(logger->buffer_size() == 2);
    TL_TEST(total_logs == 2);
    
    logger->debug("debugg!");
    logprint();
    TL_TEST(logger->buffer_size() == 0);
    TL_TEST(total_logs == 3);
    
    logger->log(arc::core::LOG_ERROR, "something broke!");
    logprint();
    TL_TEST(logger->buffer_size() == 1);
    TL_TEST(total_logs == 4);
    
    logger->fatal("Sadly we need to shutdown now..");
    logprint();
    TL_TEST(logger->buffer_size() == 2);
    TL_TEST(was_fatal == true);
    TL_TEST(total_logs == 5);
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    TL(test_logger());

    tl_summary();
}
