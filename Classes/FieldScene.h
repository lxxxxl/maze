#ifndef __FIELD_SCENE_H__
#define __FIELD_SCENE_H__

#include "cocos2d.h"

USING_NS_CC;

#define SPRITES_PER_LINE    7
#define SPRITE_SIZE         16
#define FIELD_WIDTH         40
#define FIELD_HEIGHT        30

class FieldScene : public Scene
{
public:

    enum Direction{
        UP,
        DOWN,
        LEFT,
        RIGHT
    };

    enum CellType{
        WALL,
        CLEAR
    };

    static Scene* createScene();
    virtual bool init();
    CREATE_FUNC(FieldScene);
    Sprite *spriteFromTileset(int gid);
    // move sprite to specified location.
    // x and y are in tileno, NOT screen coords
    void setNewCoords(Sprite *sprite, int x, int y);
    // mark selected point as CLEAR
    void makeClear(int x, int y);
    // generate maze
    void mazeGenerate();
    // optimize maze by removing dead ends
    void mazeOptimize();
    // draw maze on tilemap
    void mazeDraw();
    // locate and draw endpoint
    void placeEndpoint();

private:
    u_char _mazeMap[FIELD_WIDTH][FIELD_HEIGHT];
    Vec2 _endpoint;
};

#endif // __FIELD_SCENE_H__
