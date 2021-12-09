#ifndef __FIELD_SCENE_H__
#define __FIELD_SCENE_H__

#include "cocos2d.h"
#include "PathFinder.h"
USING_NS_CC;

#define SPRITES_PER_LINE    7
#define SPRITE_SIZE_TILESET 16
#define FIELD_WIDTH         50
#define FIELD_HEIGHT        28


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
    // generate maze
    void mazeGenerate();
    // optimize maze by removing dead ends
    void mazeOptimize();
    // draw maze on tilemap
    void mazeDraw();
    // locate and draw endpoint
    void placeEndpoint();
    // get wall tile acording to road position
    int getTileId(int x, int y);
    // keyboard event handler
    void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
    // display win screen
    void win();
    // return to menu
    void exit(float);
    // find path to endpoint
    void findPath();

private:
    u_char _mazeMap[FIELD_WIDTH][FIELD_HEIGHT];
    Vec2 _endpoint;
    Sprite *_player;
    int _spriteSize;

};

#endif // __FIELD_SCENE_H__
