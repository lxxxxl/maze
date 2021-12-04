#include "FieldScene.h"
#include "MenuScene.h"

Scene* FieldScene::createScene()
{
    return FieldScene::create();
}

// on "init" you need to initialize your instance
bool FieldScene::init()
{
    // 1. super init
    if ( !Scene::init() )
    {
        return false;
    }

    // 2. init tilemap
    _tileMap = new TMXTiledMap();
    _tileMap->initWithTMXFile("res/map.tmx");
    _background = _tileMap->getLayer("background");
    _maze = _tileMap->getLayer("maze");
    this->addChild(_tileMap, -1);


    // 3. generate maze

    // 4. add decorations

    return true;
}

void FieldScene::setNewCoords(Sprite *sprite, int x, int y)
{
    auto action = MoveTo::create(0.05f, Vec2(x*SPRITE_SIZE, y*SPRITE_SIZE));
    sprite->runAction(action);
}

Sprite *FieldScene::spriteFromTileset(int gid)
{
    int y = gid / SPRITES_PER_LINE;
    int x = gid % SPRITES_PER_LINE;
    auto sprite = Sprite::create("res/toen.png", Rect(x*SPRITE_SIZE,y*SPRITE_SIZE,SPRITE_SIZE,SPRITE_SIZE));
    sprite->setAnchorPoint(Vec2(0,0));
    return sprite;
    
}
