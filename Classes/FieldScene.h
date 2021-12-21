#ifndef __FIELD_SCENE_H__
#define __FIELD_SCENE_H__

#include "cocos2d.h"
#include "audio/include/AudioEngine.h"
#include "PathFinder.h"
#include "Tiles.h"
#include "Enemy.h"

USING_NS_CC;

#define FIELD_WIDTH         50
#define FIELD_HEIGHT        28
#define CHECKPOINTS_COUNT   10
#define ENEMIES_SPEED       0.4f


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
    // locate and run enemies
    void placeEnemies(int count);
    // get wall tile acording to road position
    int getTileId(int x, int y);
    // keyboard event handlers
    void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
    void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);
    void keypress(EventKeyboard::KeyCode keyCode);
    void keyhold(float dt);
    // display win screen
    void win();
    // return to menu
    void exit(float);
    // find path to endpoint
    void findPath();
    // collision check for enemies
    void collisionCheck(Enemy* enemy);
    // update current level to calculate count of enemies
    // +1 enemy every 3 levels
    void updateLevel(int level);

private:
    u_char _mazeMap[FIELD_WIDTH][FIELD_HEIGHT];
    Sprite *_endpoint;
    Vector<Sprite *> _checkpoints;
    Vector<Sprite *> _checkpointsHighlights;
    Vector<Enemy *> _enemies;
    PathFinder _pathFinder;
    Sprite *_player;
    Label *_checkpointsLabel;
    Label *_levelLabel;
    int _spriteSize;
    int _checkpointsReached;
    EventKeyboard::KeyCode _pressedKey;
    time_t _lastKeypress;
    bool _aiActive;
    int _level;

};

#endif // __FIELD_SCENE_H__
