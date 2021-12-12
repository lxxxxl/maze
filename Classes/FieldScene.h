#ifndef __FIELD_SCENE_H__
#define __FIELD_SCENE_H__

#include "cocos2d.h"
#include "PathFinder.h"
#include "Tiles.h"

USING_NS_CC;

#define FIELD_WIDTH         50
#define FIELD_HEIGHT        28
#define CHECKPOINTS_COUNT   5


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
    // count CLEAR neighbors of cell
    int countClearNeighbors(int x, int y);
    // optimize maze by removing dead ends
    void mazeOptimize();
    // draw maze on tilemap
    void mazeDraw();
    // locate and draw endpoint
    void placeEndpoint();
    // locate and draw checkpoints
    void placeCheckpoints();
    // get wall tile acording to road position
    int getTileId(int x, int y);
    // keyboard event handlers
    void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
    void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);
    void keypress(EventKeyboard::KeyCode keyCode);
    void keyhold(float);
    // display win screen
    void win();
    // return to menu
    void exit();
    // find path to endpoint
    void findPath();

private:
    u_char _mazeMap[FIELD_WIDTH][FIELD_HEIGHT];
    Sprite *_endpoint;
    Vector<Sprite *> _checkpoints;
    Sprite *_player;
    int _spriteSize;
    int _checkpointsReached;
    EventKeyboard::KeyCode _pressedKey;
    time_t _lastKeypress;

};

#endif // __FIELD_SCENE_H__
