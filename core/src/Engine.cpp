#pragma once

#include "../arc/Engine.hpp"

namespace arc {

Logger* Engine::logger(void) {return m_logger;};

void
Engine::start_window(WindowConfig* _config)
{
    if (_config == nullptr) {
        ERROR("no config provided to window");
        return;
    }

    InitWindow(_config->width, _config->height, _config->name.c_str());
    INFO("Window Created");
}
    
}; /*namespace arc*/
