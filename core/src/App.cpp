#pragma once

#include "../arc/App.hpp"

namespace arc {

App::App(){};

void App::add_logger(Logger* _logger) { m_logger = _logger; m_scene_manager.add_logger(_logger); }

bool App::INFO(const std::string& _msg) {
    if (m_logger == nullptr)
        return false;
    return m_logger->timestamped_log("ARC::APP::INFO", _msg);
}

bool App::DEBUG(const std::string& _msg) {
    if (m_logger == nullptr)
        return false;
    return m_logger->timestamped_log("ARC::APP::DEBUG", _msg);
}

bool App::WARNING(const std::string& _msg) {
    if (m_logger == nullptr)
        return false;
    return m_logger->timestamped_log("ARC::APP::WARNING", _msg);
}

bool App::ERROR(const std::string& _msg) {
    if (m_logger == nullptr)
        return false;
    return m_logger->timestamped_log("ARC::APP::ERROR", _msg);
}

SceneKey App::scene_add(const std::string& _name,
                        std::shared_ptr<IScene> _scene) {
    return m_scene_manager.add(_name, _scene);
}

bool App::scene_init_async(SceneKey _scene) {
    return m_scene_manager.init_async(_scene);
}

bool App::scene_destroy_async(SceneKey _scene) {
    return m_scene_manager.destroy_async(_scene);
}

bool App::scene_is_inited(SceneKey _scene) {
    return m_scene_manager.is_active(_scene);
}

SceneKey App::scene_active_get(void) { return m_scene_manager.top(); }

std::shared_ptr<IScene> App::scene_active_get_ptr(void) {
    return m_scene_manager.top_ptr();
}

SceneKey App::scene_active_set(SceneKey _scene) {
    return m_scene_manager.make_active(_scene);
}

std::string App::scene_name(SceneKey _scene) {
    return m_scene_manager.name(_scene);
}

}; /*namespace arc*/
