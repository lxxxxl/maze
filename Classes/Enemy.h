#ifndef ENEMY_H
#define ENEMY_H

#include "cocos2d.h"
#include "PathFinder.h"

USING_NS_CC;
class Enemy: public Sprite
{
public:
    Enemy();
    ~Enemy();
    static Enemy* create(string filename, Rect rect);
//    Enemy(int x, int y);                                // create enemy in specified location
//    SYNTHESIZE(Sprite*, _sprite, Sprite);               // sprite of enemy
//    SYNTHESIZE(int, _x, X);                             // enemy's x coordinate
//    SYNTHESIZE(int, _y, Y);                             // enemy's y coordinate
    SYNTHESIZE(float, _speed, Speed);                           // enemy's speed
    SYNTHESIZE(int, _playerX, PlayerX);                         // player's x coordinate
    SYNTHESIZE(int, _playerY, PlayerY);                         // player's y coordinate
    SYNTHESIZE(int, _spriteSize, SpriteSize);                   // Size of sprites
    SYNTHESIZE(PathFinder*, _pathFinder, PathFinder);           // Pathfinder object
    SYNTHESIZE(CallFunc*, _collideCallback, CollideCallback);   // Pathfinder object
    void walk(int x, int y);                                    // walk to specified location

};

#endif // ENEMY_H
