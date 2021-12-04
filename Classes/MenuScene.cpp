#include "MenuScene.h"
#include "FieldScene.h"

Scene* MenuScene::createScene()
{
    return MenuScene::create();
}

// on "init" you need to initialize your instance
bool MenuScene::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    /////////////////////////////
    // 2. create menu items
    auto item1 = MenuItemFont::create("Play", [&](Ref* sender){
        displayFieldScene();
    });

    //////////////////////////////
    // 3. create menu
    auto menu = Menu::create(item1,nullptr);
    menu->alignItemsVertically();
    this->addChild(menu, 1);

    return true;
}

void MenuScene::displayFieldScene(bool useAi)
{
    auto scene = FieldScene::create();
    if (useAi){
        // TODO turn on pathfinding algo
    }
    Director::getInstance()->replaceScene(scene);
}
