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
    placeCheckpoints();

    // 3. add player
    _player = spriteFromTileset(Objects::Player);
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
    // check if we reached checkpoint
    for (int i = 0; i < _checkpoints.size(); i++){
        Sprite *checkpoint = _checkpoints.at(i);
        if ((_player->getPosition().x == checkpoint->getPositionX()) &&
                (_player->getPosition().y == checkpoint->getPositionY())){

            //checkpoint->setVisible(false);
            checkpoint->removeFromParent();
            _checkpoints.eraseObject(checkpoint);
            //checkpoint->removeFromParent();
            if (_checkpoints.size() == 0){
                auto pos = _endpoint->getPosition();
                //_endpoint->setVisible(false);
                _endpoint->removeFromParent();
                _endpoint = spriteFromTileset(Objects::ExitOpen);
                _endpoint->setPosition(pos);
                this->addChild(_endpoint);
                _endpoint->setTag(EXIT_OPEN);
            }
            break;
        }
    }
    // check if we reached endpoint
    if ((_player->getPosition().x == _endpoint->getPositionX()) &&
            (_player->getPosition().y == _endpoint->getPositionY()) &&
            (_endpoint->getTag() == EXIT_OPEN)){
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

int FieldScene::countClearNeighbors(int x, int y)
{
    int neighbors = 0;
    if ((x - 1 >= 0) && _mazeMap[x - 1][y] == CLEAR)
        neighbors++;
    if ((x + 1 < FIELD_WIDTH) && _mazeMap[x + 1][y] == CLEAR)
        neighbors++;
    if ((y - 1 >= 0) && _mazeMap[x][y - 1] == CLEAR)
        neighbors++;
    if ((y + 1 < FIELD_HEIGHT) && _mazeMap[x][y + 1] == CLEAR)
        neighbors++;

    return neighbors;
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



                if (countClearNeighbors(x, y) <= 1)
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
            auto sprite = spriteFromTileset(Objects::Grass);
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
        return Walls::LeftRight;

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
    case 0b11110111:
        return Walls::UpDown;

    case 0b00101110:
    case 0b00111010:
    case 0b11101011:
    case 0b00101010:
        return Walls::VertRight;


    case 0b00111000:
    case 0b01111000:
    case 0b11101111:
    case 0b00101000:
    case 0b00111100:
        return Walls::DownRight;

    case 0b11100000:
    case 0b11110000:
    case 0b10111111:
    case 0b10100000:
        return Walls::DownLeft;

    case 0b00001110:
    case 0b00011110:
    case 0b00001010:
    case 0b11111011:
    case 0b00001111:
        return Walls::UpRight;

    case 0b10000011:
    case 0b11111110:
    case 0b10000010:
        return Walls::UpLeft;

    case 0b11101000:
    case 0b10111000:
    case 0b10101000:
    case 0b10101111:
        return Walls::HorDown;

    case 0b10001110:
    case 0b11111010:
    case 0b10001011:
    case 0b10001010:
        return Walls::HorUp;

    case 0b10111110:
    case 0b10100011:
    case 0b10100010:
    case 0b11100010:
        return Walls::VertLeft;

    case 0b10111011:
    case 0b10101011:
    case 0b11101110:
    case 0b10111010:
    case 0b11101010:
    case 0b10101110:
    case 0b10101010:
        return Walls::Cross;

    default:
        return countClearNeighbors(x, y) == 0 ? InnerObstacles[RandomHelper::random_int(0, (int)InnerObstacles.size() - 1)]
                                              : OuterObstacles[RandomHelper::random_int(0, (int)OuterObstacles.size() - 1)];
    }
}

void FieldScene::placeEndpoint()
{
    for (int x = FIELD_WIDTH-1; x > 0; x--)
        for (int y = 0; y < FIELD_HEIGHT; y++){
            if (_mazeMap[x][y] == WALL)
                continue;

            _endpoint = spriteFromTileset(Objects::ExitClosed);
            _endpoint->setPosition(Vec2(x * _spriteSize, y * _spriteSize));
            this->addChild(_endpoint);
            _endpoint->setTag(EXIT_CLOSED);
            return;
        }
}

void FieldScene::placeCheckpoints()
{
    for (int i = 0; i < CHECKPOINTS_COUNT; i++){
        int x = 0;
        int y = 0;
        do{
            x = RandomHelper::random_int(0, FIELD_WIDTH - 1);
            y = RandomHelper::random_int(0, FIELD_HEIGHT - 1);
        }while(_mazeMap[x][y] != CLEAR);
        auto pos = Vec2(x * _spriteSize, y * _spriteSize);
        // check if checkpoint already placed here
        for (auto s: _checkpoints){
            if ((s->getPositionX() == pos.x) && (s->getPositionY() == pos.y)){
                i--;
                continue;
            }
        }
        auto sprite = spriteFromTileset(Checkpoints[RandomHelper::random_int(0, (int)Checkpoints.size() - 1)]);
        sprite->setPosition(pos);
        this->addChild(sprite);
        _checkpoints.pushBack(sprite);
    }
}

void FieldScene::findPath()
{
    PathFinder pathFinder;
    Vector<FiniteTimeAction *> aiMovement;
    auto winCallback = CallFunc::create(CC_CALLBACK_0(FieldScene::win, this));

    pathFinder.setDimensions(FIELD_WIDTH, FIELD_HEIGHT);
    for (int x = 0; x < FIELD_WIDTH; x++)
        for (int y = 0; y < FIELD_HEIGHT; y++)
            pathFinder.setPassable(x, y, _mazeMap[x][y] == CLEAR);

    Sprite *startPoint = _player;
    // process checkpoints
    for (Sprite *s: _checkpoints){
        pathFinder.setStartpoint(startPoint->getPositionX() / _spriteSize, startPoint->getPositionY() / _spriteSize);
        pathFinder.setEndPoint(s->getPositionX() / _spriteSize, s->getPositionY() / _spriteSize);
        pathFinder.findPath();
        auto path = pathFinder.getPath();
        for (auto it = path.begin(); it < path.end(); it++){
            Cell *c = *it;
            auto move = MoveTo::create(0.1, Vec2(c->getX() * _spriteSize, c->getY() * _spriteSize));
            aiMovement.pushBack(move);
        }
        aiMovement.pushBack(winCallback->clone());
        startPoint = s;
    }

    // process endpoint
    pathFinder.setStartpoint(startPoint->getPositionX() / _spriteSize, startPoint->getPositionY() / _spriteSize);
    pathFinder.setEndPoint(_endpoint->getPositionX() / _spriteSize, _endpoint->getPositionY() / _spriteSize);
    pathFinder.findPath();
    auto path = pathFinder.getPath();
    for (auto it = path.begin(); it < path.end(); it++){
        Cell *c = *it;
        auto move = MoveTo::create(0.1, Vec2(c->getX() * _spriteSize, c->getY() * _spriteSize));
        aiMovement.pushBack(move);
    }
    aiMovement.pushBack(winCallback->clone());

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
    auto sprite = Sprite::create(TilesFilename, Rect(x * SPRITE_SIZE_TILESET, y * SPRITE_SIZE_TILESET, SPRITE_SIZE_TILESET, SPRITE_SIZE_TILESET));
    sprite->setScale(float(_spriteSize) / SPRITE_SIZE_TILESET);
    sprite->getTexture()->setAliasTexParameters();  // remove antialiasing because it corrupts tiles
    sprite->setAnchorPoint(Vec2(0, 0));
    return sprite;
}
