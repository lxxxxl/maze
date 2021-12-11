#include "PathFinder.h"

Cell *startCell;
Cell *endCell;

float distanceBetweenTwoCells(float c1X,float c1Y, float c2X, float c2Y){

    return sqrt(pow(c2X - c1X,2) + pow(c2Y - c1Y,2));
}

bool comparebyDistanceBetweenStartAndCurrent(Cell *c1, Cell *c2){
    float distanceOfC1AndStart = c1->getDistance() +
        distanceBetweenTwoCells((float)c1->getX(),(float)c1->getY(),(float)startCell->getX(), (float)startCell->getY());

    float distanceOfC2AndStart = c2->getDistance() +
        distanceBetweenTwoCells((float)c2->getX(),(float)c2->getY(),(float)startCell->getX(), (float)startCell->getY());
    if(distanceOfC1AndStart <= distanceOfC2AndStart){
        return false;
    }else{
        return true;
    }
}

void PathFinder::setDimensions(int width, int height)
{
    _fieldWidth = width;
    _fieldHeight = height;
    _pathMap = Array2D<Cell>(_fieldWidth, _fieldHeight);
}

void PathFinder::setPassable(int x, int y, int passable)
{
    Cell *cell = new Cell();
    cell->setX(x);
    cell->setY(y);
    cell->setWeight(passable == 1? 1 : 4);
    _pathMap.Set(x, y, cell);
}

void PathFinder::setStartpoint(int x, int y)
{
    startCell = _pathMap.Get(x, y);
}

void PathFinder::setEndPoint(int x, int y)
{
    endCell = _pathMap.Get(x, y);
}

void PathFinder::setCheckpoint(int x, int y)
{
    Cell *cell = _pathMap.Get(x, y);
    _checkpoints.push_back(cell);
}

void PathFinder::savePathTo(int x, int y)
{
    deque<Cell *> path;
    // generate path
    Cell* c = _pathMap.Get(x, y);
    while(c->getLastX() != -1){
        path.push_front(c);
        c = _pathMap.Get(c->getLastX(),c->getLastY());
    }
    _path.insert(_path.end(), path.begin(), path.end());
}

void PathFinder::cleanup()
{
    for (int x = 0; x < _fieldWidth; x++)
        for (int y = 0; y < _fieldHeight; y++){
            auto cell = _pathMap.Get(x, y);
            cell->setDistance(0);
            cell->setLastX(-1);
            cell->setLastY(-1);
            cell->setMarked(false);
        }
}

void PathFinder::findPath()
{
    // cleanup cell's metadata
    cleanup();
    // cleanup path array
    _path.clear();

    // array with cells to check
    vector<Cell*> checkCells;

    checkCells.push_back(startCell);
    make_heap(checkCells.begin(), checkCells.end(), comparebyDistanceBetweenStartAndCurrent);
    startCell->setMarked(true);

    while(checkCells.size() != 0){
        pop_heap(checkCells.begin(),checkCells.end(),comparebyDistanceBetweenStartAndCurrent);
        Cell *currentCell = checkCells.back();
        checkCells.pop_back();

        if (_checkpoints.size() != 0){
            for (Cell* cell: _checkpoints){
                if (cell == currentCell){   // reached one of checkpoints
                    _checkpoints.erase(remove(_checkpoints.begin(), _checkpoints.end(), cell));
                    // save path to current checkpoint
                    savePathTo(cell->getX(), cell->getY());
                    // cleanup temp data
                    cleanup();
                    checkCells.clear();
                    // set current checkpoint as new startpoint
                    startCell = currentCell;
                    currentCell->setMarked(true);
                    //return;
                }
            }
        }
        else if(currentCell == endCell){    // reached endpoint
            savePathTo(currentCell->getX(), currentCell->getY());
            break;
        }

        for (int i = 0; i < 4; i++){ // check 4 directions
            int indexX = currentCell->getX() + DIRECTION[i][0];
            int indexY = currentCell->getY() + DIRECTION[i][1];

            // check if it is suitable cell
            if(indexX < 0 || indexX >= _fieldWidth ||
               indexY < 0 || indexY >= _fieldHeight ||
               !_pathMap.Get(indexX,indexY)->getPassable())
                continue;

            Cell *cell = _pathMap.Get(indexX,indexY);
            float distance = cell->getWeight() + _pathMap.Get(currentCell->getX(), currentCell->getY())->getDistance(); // calculate the distance
            if(cell->getMarked() == false){
                cell->setMarked(true);
                cell->setLastX(currentCell->getX());
                cell->setLastY(currentCell->getY());
                cell->setDistance(distance);
                checkCells.push_back(cell);     // only push the unmarked cell into the vector
                push_heap(checkCells.begin(),checkCells.end(),comparebyDistanceBetweenStartAndCurrent);
            }else{  // if find a lower distance, update it
                if(distance < cell->getDistance()){
                    cell->setDistance(distance);
                    cell->setLastX(currentCell->getX());
                    cell->setLastY(currentCell->getY());
                    make_heap(checkCells.begin(),checkCells.end(),comparebyDistanceBetweenStartAndCurrent);    // distance change,so make heap again
                }
            }
        }
    }
}

