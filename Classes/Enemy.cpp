#include "Enemy.h"

Enemy::Enemy() {}

Enemy::~Enemy() {}

Enemy* Enemy::create(string filename, Rect rect)
{
    Enemy *enemy = new (std::nothrow) Enemy();
    if (enemy && enemy->initWithFile(filename, rect))
    {
        enemy->autorelease();
        return enemy;
    }
    CC_SAFE_DELETE(enemy);
    return nullptr;
}

void Enemy::walk(int x, int y)
{
    if ((x == _playerX) && (y == _playerY))
        return;

    _playerX = x;
    _playerY = y;

    stopAllActions();

    // find path
    Vector<FiniteTimeAction *> aiMovement;

    _pathFinder->setStartpoint(getPositionX() / _spriteSize, getPositionY() / _spriteSize);
    _pathFinder->setEndPoint(_playerX, _playerY);
    _pathFinder->findPath();
    auto path = _pathFinder->getPath();
    for (auto it = path.begin(); it < path.end(); it++){
        Cell *c = *it;
        auto move = MoveTo::create(_speed, Vec2(c->getX() * _spriteSize, c->getY() * _spriteSize));
        aiMovement.pushBack(move);
        aiMovement.pushBack(_collideCallback->clone());
    }
    if (aiMovement.size() > 0)
        runAction(Sequence::create(aiMovement));
}
