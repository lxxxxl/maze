#ifndef __FIELD_SCENE_H__
#define __FIELD_SCENE_H__

#include "cocos2d.h"

USING_NS_CC;

#define SPRITES_PER_LINE    7
#define SPRITE_SIZE         16
#define FIELD_WIDTH         30
#define FIELD_HEIGHT        20

class FieldScene : public Scene
{
public:

    static Scene* createScene();
    virtual bool init();
    CREATE_FUNC(FieldScene);
    Sprite *spriteFromTileset(int gid);
    // move sprite to specified location.
    // x and y are in tileno, NOT screen coords
    void setNewCoords(Sprite *sprite, int x, int y);

private:
    TMXTiledMap *_tileMap;
    TMXLayer *_background;
    TMXLayer *_maze;
};

#endif // __FIELD_SCENE_H__
