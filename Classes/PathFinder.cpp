#include "PathFinder.h"

int g_goalX;
int g_goalY;
float distanceBetweenTwoCells(float c1X,float c1Y, float c2X, float c2Y){

    return sqrt(pow(c2X - c1X,2) + pow(c2Y - c1Y,2));
}
bool comparebyDistanceBetweenStartAndGoal(Cell *c1, Cell *c2){
    float distanceOfC1AndGoal = c1->getDistance() +
        distanceBetweenTwoCells((float)c1->getX(),(float)c1->getY(),(float)g_goalX,(float) g_goalY);

    float distanceOfC2AndGoal = c2->getDistance() +
        distanceBetweenTwoCells((float)c2->getX(),(float)c2->getY(),(float)g_goalX,(float) g_goalY);
    if(distanceOfC1AndGoal <= distanceOfC2AndGoal){
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
    cell->setWeight(passable == 1? 0 : 4);
    _pathMap.Set(x, y, cell);
}

void PathFinder::setStartpoint(int x, int y)
{
    _startposX = x;
    _startposY = y;
}

void PathFinder::setEndPoint(int x, int y)
{
    g_goalX = x;
    g_goalY = y;
}

void PathFinder::findPath()
{
    // cleanup cell's metadata
    for (int x = 0; x < _fieldWidth; x++)
        for (int y = 0; y < _fieldHeight; y++){
            auto cell = _pathMap.Get(x, y);
            cell->setDistance(0);
            cell->setLastX(-1);
            cell->setLastY(-1);
            cell->setMarked(false);
        }

    // array with cells to check
    vector<Cell*> checkCells;

    Cell *startCell = _pathMap.Get(_startposX, _startposY);
    checkCells.push_back(startCell);
    make_heap(checkCells.begin(), checkCells.end(), comparebyDistanceBetweenStartAndGoal);
    startCell->setMarked(true);

    while(checkCells.size() != 0){
        pop_heap(checkCells.begin(),checkCells.end(),comparebyDistanceBetweenStartAndGoal);
        Cell *currentCell = checkCells.back();
        checkCells.pop_back();

        if(currentCell->getX() == g_goalX && currentCell->getY() == g_goalY)    // reached endpoint
            break;

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
                push_heap(checkCells.begin(),checkCells.end(),comparebyDistanceBetweenStartAndGoal);
            }else{  // if find a lower distance, update it
                if(distance < cell->getDistance()){
                    cell->setDistance(distance);
                    cell->setLastX(currentCell->getX());
                    cell->setLastY(currentCell->getY());
                    make_heap(checkCells.begin(),checkCells.end(),comparebyDistanceBetweenStartAndGoal);    // distance change,so make heap again
                }
            }
        }
    }

    // generate path
    _path.clear();
    Cell* cell = _pathMap.Get(g_goalX,g_goalY);
    while(cell->getLastX() != -1){
        _path.push_front(cell);
        cell = _pathMap.Get(cell->getLastX(),cell->getLastY());
    }
    _path.push_front(_pathMap.Get(_startposX, _startposY));
}

