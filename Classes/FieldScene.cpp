#include "FieldScene.h"

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

    // 2. init scaler
    int screenWidth = Director::getInstance()->getOpenGLView()->getFrameSize().width;
    _spriteSize = screenWidth / FIELD_WIDTH;


    // 2. generate maze
    mazeGenerate();
    mazeOptimize();
    mazeDraw();
    placeEndpoint();

    // 3. add player
    _player = spriteFromTileset(252);
    _player->setPosition(Vec2(0 * _spriteSize, (FIELD_HEIGHT-1) * _spriteSize));
    this->addChild(_player);

    //4. add keyboard listener
    auto eventListener = EventListenerKeyboard::create();
    eventListener->onKeyPressed = CC_CALLBACK_2(FieldScene::onKeyPressed, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, this);

    //5. run AI
    findPath();

    return true;
}

void FieldScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
    int x = _player->getPosition().x / _spriteSize;
    int y = _player->getPosition().y / _spriteSize;

    auto winCallback = CallFunc::create(CC_CALLBACK_0(FieldScene::win, this));
    MoveBy *action = nullptr;

    switch(keyCode){
                case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
                    if ((x - 1 >= 0) && _mazeMap[x - 1][y] == CLEAR){
                        action = MoveBy::create(0.1f, Vec2(-_spriteSize, 0));

                    }
                    break;
                case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
                    if ((x + 1 < FIELD_WIDTH) && _mazeMap[x + 1][y] == CLEAR){
                        action = MoveBy::create(0.1f, Vec2(_spriteSize, 0));
                    }
                    break;
                case EventKeyboard::KeyCode::KEY_UP_ARROW:
                    if ((y + 1 < FIELD_HEIGHT) && _mazeMap[x][y + 1] == CLEAR){
                        action = MoveBy::create(0.1f, Vec2(0, _spriteSize));
                    }
                    break;
                case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
                    if ((y - 1 >= 0) && _mazeMap[x][y - 1] == CLEAR){
                        action = MoveBy::create(0.1f, Vec2(0, -_spriteSize));
                    }
                    break;

                case EventKeyboard::KeyCode::KEY_ESCAPE:
                    Director::getInstance()->end();

                default:
                    return;
            }
    if (action != nullptr)
        _player->runAction(Sequence::create(action, winCallback, nullptr));
}

void FieldScene::win()
{
    if ((_player->getPosition().x == _endpoint.x) && (_player->getPosition().y == _endpoint.y)){
        int centerX = Director::getInstance()->getOpenGLView()->getFrameSize().width / 2;
        int centerY = Director::getInstance()->getOpenGLView()->getFrameSize().height / 2;
        auto sprite = Sprite::create("like.png");
        sprite->setScale(4.0f);
        sprite->setPosition(Vec2(centerX, centerY));
        addChild(sprite);
        this->scheduleOnce(CC_SCHEDULE_SELECTOR(FieldScene::exit), 3.0f);
    }
}

void FieldScene::exit(float)
{
    auto menu = FieldScene::create();
    Director::getInstance()->replaceScene(menu);
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
        for (int x = 0; x < FIELD_WIDTH; x++)
        for (int y = 0; y < FIELD_HEIGHT; y++){
            // add ground tile
            Vec2 pos(x * _spriteSize, y * _spriteSize);
            auto sprite = spriteFromTileset(3);
            sprite->setPosition(pos);
            this->addChild(sprite);
            // add wall
            auto tile = getTileId(x, y);
            if (_mazeMap[x][y] != WALL)
                continue;
            pos = Vec2(x * _spriteSize, y * _spriteSize);
            auto sprite2 = spriteFromTileset(tile);
            sprite2->setPosition(pos);
            this->addChild(sprite2);
        }
}

int FieldScene::getTileId(int x, int y)
{

    // tiles IDs with static objects
    // will use as obstacles
    u_char _wallTiles[] = {7, 8, 9, 14, 15, 16, 17, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40};

    /* encode tile state into bitmask. neighbor: 0 - clear 1 - wall
     * bits:
     * 0 1 2
     * 7 X 3
     * 6 5 4
     */

    int wallMask = 0;

    if ((y + 1 < FIELD_HEIGHT) && (x - 1 >= 0))
        wallMask |= (_mazeMap[x - 1][y + 1] == WALL) << 0;
    if ((y + 1 < FIELD_HEIGHT))
        wallMask |= (_mazeMap[x][y + 1] == WALL) << 1;
    if ((y + 1 < FIELD_HEIGHT) && (x + 1 < FIELD_WIDTH))
        wallMask |= (_mazeMap[x + 1][y + 1] == WALL) << 2;
    if ((x + 1 < FIELD_WIDTH))
        wallMask |= (_mazeMap[x + 1][y] == WALL) << 3;
    if ((y - 1 >= 0) && (x + 1 < FIELD_WIDTH))
        wallMask |= (_mazeMap[x + 1][y - 1] == WALL) << 4;
    if ((y - 1 >= 0))
        wallMask |= (_mazeMap[x][y - 1] == WALL) << 5;
    if ((y - 1 >= 0) && (x - 1 >= 0))
        wallMask |= (_mazeMap[x - 1][y - 1] == WALL) << 6;
    if (x - 1 >= 0)
        wallMask |= (_mazeMap[x - 1][y] == WALL) << 7;


    switch (wallMask) {
    case 0b00001000:
    case 0b10000000:
    case 0b10001000:
    case 0b11001000:
    case 0b10011000:
    case 0b11111000:
    case 0b10001001:
    case 0b10001100:
    case 0b10011001:
    case 0b10011111:
    case 0b11111001:
    case 0b10001111:
    case 0b11111100:
    case 0b11001001:
    case 0b11001111:
    case 0b10011100:
    case 0b00001100:
    case 0b11001100:
    case 0b11011111:
    case 0b10001101:
    case 0b11111101:
    case 0b10011101:
    case 0b11011000:
    case 0b11011100:
    case 0b11001101:
    case 0b00011000:
    case 0b11011001:
    case 0b00011100:
    case 0b11011101:
        return 220;

    case 0b00100010:
    case 0b00111111:
    case 0b00111110:
    case 0b00000100:
    case 0b01000100:
    case 0b00000010:
    case 0b11110011:
    case 0b11100011:
    case 0b00110010:
    case 0b00100011:
    case 0b11100111:
    case 0b01100010:
    case 0b01100110:
    case 0b00100000:
    case 0b00110011:
    case 0b01111110:
    case 0b00100110:
    case 0b01100011:
    case 0b01110010:
    case 0b01111111:
    case 0b00100111:
    case 0b01100000:
    case 0b00110000:
    case 0b00110111:
    case 0b01110011:
    case 0b01100111:
    case 0b01110110:
    case 0b00110110:
    case 0b01110111:
    case 0b01110000:
        return 226;

    case 0b00101110:
    case 0b00111010:
    case 0b11101011:
    case 0b00101010:
        return 218;


    case 0b00111000:
    case 0b01111000:
    case 0b11101111:
    case 0b00101000:
    case 0b00111100:
        return 219;

    case 0b11100000:
    case 0b11110000:
    case 0b10111111:
    case 0b10100000:
        return 221;

    case 0b00001110:
    case 0b00011110:
    case 0b00001010:
    case 0b11111011:
        return 233;

    case 0b10000011:
    case 0b11111110:
    case 0b10000010:
        return 235;

    case 0b11101000:
    case 0b10111000:
    case 0b10101000:
        return 224;

    case 0b10001110:
    case 0b10101111:
    case 0b11111010:
    case 0b10001011:
    case 0b10001010:
        return 225;

    case 0b10111110:
    case 0b10100011:
    case 0b10100010:
    case 0b11100010:
        return 217;

    case 0b10111011:
    case 0b10101011:
    case 0b11101110:
    case 0b10111010:
    case 0b11101010:
    case 0b10101110:
    case 0b10101010:
        return 227;

    default:
        return _wallTiles[RandomHelper::random_int(0, (int)sizeof(_wallTiles)-1)];

    }
}

void FieldScene::placeEndpoint()
{
    for (int x = FIELD_WIDTH-1; x > 0; x--)
        for (int y = 0; y < FIELD_HEIGHT; y++){
            if (_mazeMap[x][y] == WALL)
                continue;

            _endpoint = Vec2(x * _spriteSize, y * _spriteSize);
            auto sprite = spriteFromTileset(207);
            sprite->setPosition(_endpoint);
            this->addChild(sprite);
            return;
        }
}

void FieldScene::findPath()
{
    PathFinder pathFinder;

    pathFinder.setDimensions(FIELD_WIDTH, FIELD_HEIGHT);
    pathFinder.setStartpoint(0, FIELD_HEIGHT - 1);
    pathFinder.setEndPoint(_endpoint.x / _spriteSize, _endpoint.y / _spriteSize);
    for (int x = 0; x < FIELD_WIDTH; x++)
        for (int y = 0; y < FIELD_HEIGHT; y++)
            pathFinder.setPassable(x, y, _mazeMap[x][y] == CLEAR);

    pathFinder.findPath();
    // execute found path
    Vector<FiniteTimeAction *> aiMovement;

    auto path = pathFinder.getPath();
    for (auto it = path.begin(); it < path.end(); it++){
        Cell *c = *it;
        auto move = MoveTo::create(0.1, Vec2(c->getX() * _spriteSize, c->getY() * _spriteSize));
        aiMovement.pushBack(move);

    }

    auto winCallback = CallFunc::create(CC_CALLBACK_0(FieldScene::win, this));
    aiMovement.pushBack(winCallback);
    _player->runAction(Sequence::create(aiMovement));
}

void FieldScene::setNewCoords(Sprite *sprite, int x, int y)
{
    auto action = MoveTo::create(0.05f, Vec2(x * _spriteSize, y * _spriteSize));
    sprite->runAction(action);
}

Sprite *FieldScene::spriteFromTileset(int gid)
{
    int y = gid / SPRITES_PER_LINE;
    int x = gid % SPRITES_PER_LINE;
    auto sprite = Sprite::create("toen.png", Rect(x * SPRITE_SIZE_TILESET, y * SPRITE_SIZE_TILESET, SPRITE_SIZE_TILESET, SPRITE_SIZE_TILESET));
    sprite->setScale(float(_spriteSize) / SPRITE_SIZE_TILESET);
    sprite->getTexture()->setAliasTexParameters();  // remove antialiasing because it corrupts tiles
    sprite->setAnchorPoint(Vec2(0, 0));
    return sprite;
}
