#include "FieldScene.h"
#include <sys/timeb.h>

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
    int screenHeight = Director::getInstance()->getOpenGLView()->getFrameSize().height;
    _spriteSize = screenWidth / FIELD_WIDTH;

    // 3. generate maze
    mazeGenerate();
    mazeOptimize();
    mazeDraw();
    placeEndpoint();
    placeCheckpoints();

    // 4. add labels
    _checkpointsLabel = Label::createWithSystemFont(std::to_string(_checkpoints.size()), "Arial", 48);
    auto pos = Vec2(screenWidth - 40, screenHeight - 80);
    _checkpointsLabel->setPosition(pos);
    addChild(_checkpointsLabel);

    _levelLabel = Label::createWithSystemFont(std::to_string(_checkpoints.size()), "Arial", 48);
    pos = Vec2(screenWidth - 40, screenHeight - 40);
    _levelLabel->setPosition(pos);
    addChild(_levelLabel);
    _levelLabel->setString(to_string(1));


    // 5. add player
    _player = spriteFromTileset(Objects::Player);
    _player->setPosition(Vec2(0 * _spriteSize, (FIELD_HEIGHT-1) * _spriteSize));
    this->addChild(_player);

    // 6. add keyboard listener
    auto eventListener = EventListenerKeyboard::create();
    eventListener->onKeyPressed = CC_CALLBACK_2(FieldScene::onKeyPressed, this);
    eventListener->onKeyReleased = CC_CALLBACK_2(FieldScene::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, this);

    // 7. init some vars
    _pressedKey = EventKeyboard::KeyCode::KEY_NONE;
    _lastKeypress = 0;
    _aiActive = false;
    _level = 1;

    // 8. preload sounds
    AudioEngine::preload("flag.wav");
    AudioEngine::preload("win.wav");

    return true;
}

void FieldScene::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
    if (keyCode == EventKeyboard::KeyCode::KEY_CTRL){
        for (Sprite *sprite: _checkpointsHighlights){
            sprite->setVisible(false);
        }
    }
    _pressedKey = EventKeyboard::KeyCode::KEY_NONE;
}

void FieldScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
    _pressedKey = keyCode;
    keyhold(0);
}

void FieldScene::keyhold(float)
{
    if (_pressedKey != EventKeyboard::KeyCode::KEY_NONE){
        keypress(_pressedKey);
        this->scheduleOnce(CC_SCHEDULE_SELECTOR(FieldScene::keyhold), 0.1f);
    }
}

void FieldScene::keypress(EventKeyboard::KeyCode keyCode)
{
    int x = _player->getPosition().x / _spriteSize;
    int y = _player->getPosition().y / _spriteSize;

    // fix to prevent walk through walls
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    long time_in_mill =  (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // co
    if ((time_in_mill - _lastKeypress) < 100)
        return;
    _lastKeypress = time_in_mill;

    auto winCallback = CallFunc::create(CC_CALLBACK_0(FieldScene::win, this));
    MoveBy *action = nullptr;

    switch(keyCode){
                case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
                    if ((x - 1 >= 0) && (_mazeMap[x - 1][y] == CLEAR) && (!_aiActive)){
                        action = MoveBy::create(0.08f, Vec2(-_spriteSize, 0));

                    }
                    break;
                case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
                    if ((x + 1 < FIELD_WIDTH) && (_mazeMap[x + 1][y] == CLEAR) && (!_aiActive)){
                        action = MoveBy::create(0.08f, Vec2(_spriteSize, 0));
                    }
                    break;
                case EventKeyboard::KeyCode::KEY_UP_ARROW:
                    if ((y + 1 < FIELD_HEIGHT) && (_mazeMap[x][y + 1] == CLEAR) && (!_aiActive)){
                        action = MoveBy::create(0.08f, Vec2(0, _spriteSize));
                    }
                    break;
                case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
                    if ((y - 1 >= 0) && (_mazeMap[x][y - 1] == CLEAR) && (!_aiActive)){
                        action = MoveBy::create(0.08f, Vec2(0, -_spriteSize));
                    }
                    break;

                case EventKeyboard::KeyCode::KEY_ESCAPE:
                    Director::getInstance()->end();

                case EventKeyboard::KeyCode::KEY_SPACE:
                    if (_aiActive){
                        _player->stopAllActions();
                        _aiActive = false;
                        // fix player position
                        x = _player->getPosition().x / _spriteSize;
                        y = _player->getPosition().y / _spriteSize;
                        _player->setPosition(Vec2(x * _spriteSize, y * _spriteSize));
                    }
                    else{
                        findPath();
                    }
                    return;

                 case EventKeyboard::KeyCode::KEY_CTRL:
                    for (Sprite *sprite: _checkpointsHighlights){
                        sprite->setVisible(true);
                    }
                    return;


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

            for (Sprite *sprite: _checkpointsHighlights){
                if (sprite->getPosition() == checkpoint->getPosition()){
                    sprite->removeFromParent();
                    _checkpointsHighlights.eraseObject(sprite);
                }
            }

            checkpoint->removeFromParent();
            _checkpoints.eraseObject(checkpoint);
            // play sound
            AudioEngine::play2d("flag.wav");
            _checkpointsLabel->setString(std::to_string(_checkpoints.size()));
            if (_checkpoints.size() == 0){
                _endpoint->setVisible(true);
            }
            break;
        }
    }
    // check if we reached endpoint
    if ((_player->getPosition().x == _endpoint->getPositionX()) &&
            (_player->getPosition().y == _endpoint->getPositionY()) &&
            _endpoint->isVisible()){
        // crutch to prevent movement after win
        _aiActive = true;
        // stop enemies
        for (auto enemy: _enemies)
            enemy->stopAllActions();
        // play sound
        AudioEngine::play2d("win.wav");
        int centerX = Director::getInstance()->getOpenGLView()->getFrameSize().width / 2;
        int centerY = Director::getInstance()->getOpenGLView()->getFrameSize().height / 2;
        auto sprite = Sprite::create("like.png");
        // make some animation
        sprite->setScale(1.0f);
        sprite->setPosition(Vec2(centerX, centerY));
        addChild(sprite);

        auto scale1 = ScaleTo::create(1.0f, RandomHelper::random_real(4.0f, 8.0f),
                                            RandomHelper::random_real(4.0f, 8.0f));
        auto scale2 = ScaleTo::create(1.0f, RandomHelper::random_real(1.0f, 4.0f),
                                            RandomHelper::random_real(1.0f, 4.0f));
        auto scale3 = ScaleTo::create(1.0f, 10.0f, 10.0f);
        auto exitCallback = CallFunc::create(CC_CALLBACK_0(FieldScene::exit, this, 0));
        sprite->runAction(Sequence::create(scale1, scale2, scale3, exitCallback, nullptr));
    }
}

void FieldScene::exit(float dt)
{
    auto scene = FieldScene::create();
    if (!dt)
        _level++;
    scene->updateLevel(_level);
    Director::getInstance()->replaceScene(scene);
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

            _endpoint = spriteFromTileset(Objects::Exit);
            _endpoint->setPosition(Vec2(x * _spriteSize, y * _spriteSize));
            this->addChild(_endpoint);
            _endpoint->setVisible(false);
            return;
        }
}

void FieldScene::placeCheckpoints()
{
    while (_checkpoints.size() < CHECKPOINTS_COUNT){
        int x = 0;
        int y = 0;
generateCoords:
        do{
            x = RandomHelper::random_int(0, FIELD_WIDTH - 1);
            y = RandomHelper::random_int(0, FIELD_HEIGHT - 1);
        }while(_mazeMap[x][y] != CLEAR);
        auto pos = Vec2(x * _spriteSize, y * _spriteSize);
        // check if checkpoint already placed here
        for (auto s: _checkpoints){
            if ((s->getPositionX() == pos.x) && (s->getPositionY() == pos.y))
                goto generateCoords;
        }
        auto sprite = spriteFromTileset(Checkpoints[RandomHelper::random_int(0, (int)Checkpoints.size() - 1)]);
        sprite->setPosition(pos);
        addChild(sprite);
        _checkpoints.pushBack(sprite);

        // highliter for checkpoint
        auto s = spriteFromTileset(Objects::CheckpointHighlighter);
        s->setPosition(sprite->getPosition());
        s->setVisible(false);
        _checkpointsHighlights.pushBack(s);
        addChild(s);
    }
}
void FieldScene::placeEnemies(int count)
{
    // init pathfinder
    _pathFinder.setDimensions(FIELD_WIDTH, FIELD_HEIGHT);
    for (int x = 0; x < FIELD_WIDTH; x++)
        for (int y = 0; y < FIELD_HEIGHT; y++)
            _pathFinder.setPassable(x, y, _mazeMap[x][y] == CLEAR);

    // create enemies
    for (int i = 0; i < count; i++){
        int x = 0;
        int y = 0;
        do{
            x = RandomHelper::random_int(0, FIELD_WIDTH - 1);
            y = RandomHelper::random_int(0, FIELD_HEIGHT - 1);
        }while(_mazeMap[x][y] != CLEAR);
        auto pos = Vec2(x * _spriteSize, y * _spriteSize);
        // check if checkpoint already placed here
        for (auto s: _enemies){
            if ((s->getPositionX() == pos.x) && (s->getPositionY() == pos.y)){
                i--;
                continue;
            }
        }

        // init enemy tile
        int tile = Enemies[RandomHelper::random_int(0, (int)Enemies.size() - 1)];
        auto enemy = Enemy::create(TilesFilename, Rect((tile % SPRITES_PER_LINE) * SPRITE_SIZE_TILESET, (tile / SPRITES_PER_LINE) * SPRITE_SIZE_TILESET, SPRITE_SIZE_TILESET, SPRITE_SIZE_TILESET));
        enemy->setScale(float(_spriteSize) / SPRITE_SIZE_TILESET);
        enemy->getTexture()->setAliasTexParameters();  // remove antialiasing because it corrupts tiles
        enemy->setAnchorPoint(Vec2(0, 0));
        enemy->setPosition(pos);
        addChild(enemy);
        _enemies.pushBack(enemy);

        // init enemy AI
        auto collisionCallback = CallFunc::create(CC_CALLBACK_0(FieldScene::collisionCheck, this, enemy));
        enemy->setSpriteSize(_spriteSize);
        enemy->setSpeed(ENEMIES_SPEED);
        enemy->setCollideCallback(collisionCallback);
        enemy->setPathFinder(&_pathFinder);
        enemy->walk(0, FIELD_HEIGHT-1);
    }
}

void FieldScene::updateLevel(int level){
    _level = level;
    _levelLabel->setString(to_string(_level));
    int enemiesCount = trunc(_level / 3);
    placeEnemies(enemiesCount);
}
void FieldScene::collisionCheck(Enemy* enemy)
{
    // check if player within 2 tiles from enemy
    if (_player->isVisible() &&
    (abs(_player->getPositionX() - enemy->getPositionX()) < _spriteSize * 2) &&
    (abs(_player->getPositionY() - enemy->getPositionY()) < _spriteSize * 2)){
        auto dead = spriteFromTileset(Objects::Dead);
        dead->setPosition(_player->getPosition());
        _player->stopAllActions();
        _player->setVisible(false);
        addChild(dead);
        AudioEngine::play2d("dead.wav");
        this->scheduleOnce(CC_SCHEDULE_SELECTOR(FieldScene::exit), 2.0f);
        return;
    }
    // recreate collision callback because it crashes on subsequent walk() calls
    // object deleted on from stack?
    auto collisionCallback = CallFunc::create(CC_CALLBACK_0(FieldScene::collisionCheck, this, enemy));
    enemy->setCollideCallback(collisionCallback);
    enemy->walk(_player->getPosition().x / _spriteSize, _player->getPosition().y / _spriteSize);
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
    // process endpoint
    pathFinder.setStartpoint(_player->getPositionX() / _spriteSize, _player->getPositionY() / _spriteSize);
    pathFinder.setEndPoint(_endpoint->getPositionX() / _spriteSize, _endpoint->getPositionY() / _spriteSize);
    for (Sprite *s: _checkpoints)
        pathFinder.setCheckpoint(s->getPositionX() / _spriteSize, s->getPositionY() / _spriteSize);
    pathFinder.findPath();
    auto path = pathFinder.getPath();
    for (auto it = path.begin(); it < path.end(); it++){
        Cell *c = *it;
        auto move = MoveTo::create(0.1, Vec2(c->getX() * _spriteSize, c->getY() * _spriteSize));
        aiMovement.pushBack(move);
        aiMovement.pushBack(winCallback->clone());
    }

    _aiActive = true;
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
