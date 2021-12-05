#include "FieldScene.h"
#include "MenuScene.h"

Scene *FieldScene::createScene()
{
    return FieldScene::create();
}

// on "init" you need to initialize your instance
bool FieldScene::init()
{
    // 1. super init
    if (!Scene::init())
    {
        return false;
    }

    // 2. generate maze
    mazeGenerate();
    mazeOptimize();
    mazeDraw();
    placeEndpoint();

    // 4. TODO

    return true;
}

void FieldScene::makeClear(int x, int y)
{

}

void FieldScene::mazeGenerate()
{
    // Prim's algo
    std::vector<Vec2> testpoints;
    memset(_mazeMap, WALL, sizeof(u_char) * FIELD_WIDTH * FIELD_HEIGHT);

    // get starting point at top left corner
    int x = 0;
    int y = FIELD_HEIGHT-1;

    _mazeMap[x][y] = CLEAR;
    // add cells that are two orthogonal spaces away from starting point
    if (y - 2 >= 0)
    {
        testpoints.push_back(Vec2(x, y - 2));
    }
    if (y + 2 < FIELD_HEIGHT)
    {
        testpoints.push_back(Vec2(x, y + 2));
    }
    if (x - 2 >= 0)
    {
        testpoints.push_back(Vec2(x - 2, y));
    }
    if (x + 2 < FIELD_WIDTH)
    {
        testpoints.push_back(Vec2(x + 2, y));
    }

    // While there are cells in growable array, choose choose one at random and clear it
    while (testpoints.size() > 0)
    {
        Vec2 tp = testpoints[RandomHelper::random_int(0, (int)testpoints.size() - 1)];
        x = tp.x;
        y = tp.y;
        _mazeMap[x][y] = CLEAR;
        testpoints.erase(std::remove(testpoints.begin(), testpoints.end(), tp));

        // The cell you just cleared needs to be connected with another clear cell.
        // Look two orthogonal spaces away from the cell you just cleared until you find one that is not a wall.
        // Clear the cell between them.
        std::vector<Direction> directions;
        directions.push_back(Direction::UP);
        directions.push_back(Direction::DOWN);
        directions.push_back(Direction::LEFT);
        directions.push_back(Direction::RIGHT);

        while (directions.size() > 0)
        {
            Direction d = directions[RandomHelper::random_int(0, (int)directions.size() - 1)];

            switch (d)
            {
            case Direction::UP:
                if ((y - 2 >= 0) && _mazeMap[x][y - 2] == CLEAR)
                {
                    _mazeMap[x][y - 1] = CLEAR;
                    directions.clear();
                }
                break;
            case Direction::DOWN:
                if ((y + 2 < FIELD_HEIGHT) && _mazeMap[x][y + 2] == CLEAR)
                {
                    _mazeMap[x][y + 1] = CLEAR;
                    directions.clear();
                }
                break;
            case Direction::LEFT:
                if ((x - 2 >= 0) && _mazeMap[x - 2][y] == CLEAR)
                {
                    _mazeMap[x - 1][y] = CLEAR;
                    directions.clear();
                }
                break;
            case Direction::RIGHT:
                if ((x + 2 < FIELD_WIDTH) && _mazeMap[x + 2][y] == CLEAR)
                {
                    _mazeMap[x + 1][y] = CLEAR;
                    directions.clear();
                }
                break;
            }
            if (directions.size() > 0)
                directions.erase(std::remove(directions.begin(), directions.end(), d));
        }

        // add cells that are two orthogonal spaces away from current point
        if ((y - 2 >= 0) && _mazeMap[x][y - 2] == WALL)
        {
            testpoints.push_back(Vec2(x, y - 2));
        }
        if ((y + 2 < FIELD_HEIGHT) && _mazeMap[x][y + 2] == WALL)
        {
            testpoints.push_back(Vec2(x, y + 2));
        }
        if ((x - 2 >= 0) && _mazeMap[x - 2][y] == WALL)
        {
            testpoints.push_back(Vec2(x - 2, y));
        }
        if ((x + 2 < FIELD_WIDTH) && _mazeMap[x + 2][y] == WALL)
        {
            testpoints.push_back(Vec2(x + 2, y));
        }
    }
}

void FieldScene::mazeOptimize()
{
    std::vector<Vec2> deadEnds;
    for (int i = 0; i < 4; i++){    // check for dead ends, 4 iterations
        for (int x = 0; x < FIELD_WIDTH; x++)
            for (int y = 0; y < FIELD_HEIGHT; y++){
                if (_mazeMap[x][y] != CLEAR)
                    continue;
                // skip startpoint
                if ((x == 0) && (y == FIELD_HEIGHT-1))
                    continue;

                int neighbors = 0;
                if ((x - 1 >= 0) && _mazeMap[x - 1][y] == CLEAR)
                    neighbors++;
                if ((x + 1 < FIELD_WIDTH) && _mazeMap[x + 1][y] == CLEAR)
                    neighbors++;
                if ((y - 1 >= 0) && _mazeMap[x][y - 1] == CLEAR)
                    neighbors++;
                if ((y + 1 < FIELD_HEIGHT) && _mazeMap[x][y + 1] == CLEAR)
                    neighbors++;

                if (neighbors <= 1)
                    _mazeMap[x][y] = WALL;
            }
    }
}

void FieldScene::mazeDraw()
{
    // tiles IDs with static objects
    // will use as obstacles
    // TODO add multi-tiled obstacles
    u_short _wallTiles[] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26 ,27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 164, 165, 166, 167, 188, 195, 202, 209, 216, 222, 223, 229, 230, 236, 237, 238, 239, 240, 241, 242, 243, 244};

    for (int x = 0; x < FIELD_WIDTH; x++)
        for (int y = 0; y < FIELD_HEIGHT; y++){
            // add ground tile
            Vec2 pos(x*SPRITE_SIZE, y * SPRITE_SIZE);
            auto sprite = spriteFromTileset(3);
            sprite->setPosition(pos);
            this->addChild(sprite);
            // add wall
            auto tile = _wallTiles[RandomHelper::random_int(0, (int)(sizeof(_wallTiles) / sizeof(u_short))-1)];
            if (_mazeMap[x][y] != WALL)
                continue;
            pos = Vec2(x*SPRITE_SIZE, y * SPRITE_SIZE);
            auto sprite2 = spriteFromTileset(tile);
            sprite2->setPosition(pos);
            this->addChild(sprite2);
        }
}

void FieldScene::placeEndpoint()
{
    for (int x = FIELD_WIDTH-1; x > 0; x--)
        for (int y = 0; y < FIELD_HEIGHT; y++){
            if (_mazeMap[x][y] == WALL)
                continue;

            _endpoint = Vec2(x*SPRITE_SIZE, y * SPRITE_SIZE);
            auto sprite = spriteFromTileset(207);
            sprite->setPosition(_endpoint);
            this->addChild(sprite);
            return;
        }
}

void FieldScene::setNewCoords(Sprite *sprite, int x, int y)
{
    auto action = MoveTo::create(0.05f, Vec2(x * SPRITE_SIZE, y * SPRITE_SIZE));
    sprite->runAction(action);
}

Sprite *FieldScene::spriteFromTileset(int gid)
{
    int y = gid / SPRITES_PER_LINE;
    int x = gid % SPRITES_PER_LINE;
    auto sprite = Sprite::create("toen.png", Rect(x * SPRITE_SIZE, y * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE));
    sprite->setAnchorPoint(Vec2(0, 0));
    return sprite;
}
