#pragma once

#include "../inc/SceneManager.hpp"

namespace arc {
namespace core {

bool SceneHandle::async_init() {
    if (m_scene == nullptr)
        return false;

    m_ready.store(true);
    return true;
}

bool SceneHandle::async_destroy() {
    if (m_scene == nullptr)
        return false;
    m_ready.store(false);
  
    return true;
}

bool SceneHandle::can_update() {
    if (m_scene == nullptr)
        return false;
    return m_ready.load();
} 

}; /*ns*/
}; /*ns*/
