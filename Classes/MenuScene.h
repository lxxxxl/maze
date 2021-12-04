#ifndef __MENU_SCENE_H__
#define __MENU_SCENE_H__

#include "cocos2d.h"

USING_NS_CC;

class MenuScene : public Scene
{
public:
    static Scene* createScene();

    virtual bool init();
    
    // implement the "static create()" method manually
    CREATE_FUNC(MenuScene);

    void displayFieldScene(bool useAi = false);
};

#endif // __MENU_SCENE_H__
