#include <iostream>

#include "../testlib.h"

//#include <ArcEngine/SceneManager.hpp>
#include "../../../core/inc/SceneManager.hpp"
#include "../../../core/inc/Logger.hpp"

class GameScene : public  arc::core::IScene {
    std::string& m_state;
public:
    explicit GameScene(std::string& _state_dst) : m_state(_state_dst) {};
    bool init(void) override {
        m_state = "init";
        return true;
    }
    bool update(void) override {
        m_state = "update";
        return true;
    }
    bool destroy(void) override {
        m_state = "destroy";
        return true;
    }
};

class MenuScene : public arc::core::IScene {
    std::string& m_state;
public:
    explicit MenuScene(std::string& _state_dst) : m_state(_state_dst) {};
    bool init(void) override {
        m_state = "init";
        return true;
    }
    bool update(void) override {
        m_state = "update";
        return true;
    }
    bool destroy(void) override {
        m_state = "destroy";
        return true;
    }
};

void
test_simple_scene(void)
{
  std::string state;
  GameScene s(state);

  s.init();
  TL_TEST(state == "init");
  s.update();
  TL_TEST(state == "update");
  s.update();
  TL_TEST(state == "update");
  s.update();
  TL_TEST(state == "update");
  s.destroy();
  TL_TEST(state == "destroy");
}

void
test_async_scenehandle(void)
{
    std::string menu_state = "";
    std::string game_state = "";
    arc::core::SceneHandle gameHandle;
    gameHandle.make<GameScene>(game_state);

    arc::core::SceneHandle menuHandle;
    menuHandle.make<GameScene>(menu_state);

    TL_TEST(menuHandle.can_update() == false);
    menuHandle.async_init();
    TL_TEST(menu_state == "init");
    TL_TEST(menuHandle.can_update() == true);
    

    TL_TEST(gameHandle.can_update() == false);
    gameHandle.async_init();
    TL_TEST(game_state == "init");
    TL_TEST(gameHandle.can_update() == true);


}

class TransitionScene : public arc::core::IScene {
    int m_curr_ticks{0};
    int m_max_ticks{0};
    std::shared_ptr<arc::core::Logger> m_logger;
    arc::core::SceneHandle* m_to;
public:
    
    explicit TransitionScene(std::shared_ptr<arc::core::Logger> logger, int ticks, arc::core::SceneHandle* to) {
        m_logger = logger;
        m_max_ticks = ticks;
        m_to = to;
    }

    bool init(void) override {
        m_logger->info("initialized transition scene");
        return true;
    }
    bool update(void) override {
        if (m_curr_ticks >= m_max_ticks) {
            if (m_to->can_update())
                return true;
            m_to->async_init();
        }
        m_logger->info("transition tick: " + std::to_string(m_curr_ticks++));
        return true;
    }
    bool destroy(void) override {
        m_logger->info("destroyed transition scene");
        return true;
    }
};

void
test_transitions(void)
{
    std::string game_state = "";
    std::shared_ptr<arc::core::Logger> logger = arc::core::Logger::make("log.log",
                                                                        10,
                                                                        arc::core::LOG_EVERYTHING);
    logger->clear_logfile();


    arc::core::SceneHandle gameHandle;
    gameHandle.make<GameScene>(game_state);

    arc::core::SceneHandle transitionHandle;
    transitionHandle.make<TransitionScene>(logger, 10, &gameHandle);
    
    for (int i = 0; i < 999; i++) {
        transitionHandle.scene()->update();
        if (gameHandle.can_update())
            break;
    }

    TL_TEST(gameHandle.can_update() == true);
}

/*
void
test_async_init(void)
{
    arc::Logger logger{};
    arc::LogConfig conf;
    conf.target = "log.txt";
    conf.buffersize = 3;
    logger.load_config(conf);
    logger.clear_log();
    arc::SceneManager scene_manager{};
    scene_manager.add_logger(&logger);


    std::string llgame_state = "";
    arc::SceneKey menu = scene_manager.add("menu", std::make_shared<SimpleMenuScene>());
    arc::SceneKey llgame = scene_manager.add("longload_game", 
                                             std::make_shared<LongLoadScene>(llgame_state));
    scene_manager.init(menu);
    TL_TEST(menu == 1);
    TL_TEST(llgame == 2);
    TL_TEST(scene_manager.top() == menu);

    scene_manager.init_async(llgame);
    TL_TEST(scene_manager.is_active(llgame) == false);
    TL_TEST(scene_manager.top() == menu);
    std::cout << scene_manager.info() << std::endl;

    while (!scene_manager.is_active(llgame)) {
        scene_manager.top_ptr()->update();
        //scene_manager.try_clean_threads();
    }
    std::cout << scene_manager.info() << std::endl;
    TL_TEST(llgame_state == "init");
    scene_manager.make_active(llgame);
    TL_TEST(scene_manager.top() == llgame);
    scene_manager.top_ptr()->update();
    TL_TEST(llgame_state == "update");
    std::cout << scene_manager.info() << std::endl;
    
    scene_manager.make_active(menu);
    TL_TEST(scene_manager.top() == menu);
    scene_manager.destroy_async(llgame);
    while (scene_manager.is_active(llgame)) {
        scene_manager.top_ptr()->update();
    }
    TL_TEST(scene_manager.top() == menu);
    TL_TEST(scene_manager.is_active(llgame) == false);

    std::cout << scene_manager.info() << std::endl;
}
*/

int
main(int argc, char** argv)
{
  (void)argc;
  (void)argv;
  TL(test_simple_scene());
  TL(test_async_scenehandle());
  TL(test_transitions());

  tl_summary();
}
