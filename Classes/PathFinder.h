#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <cmath>
#include "cocos2d.h"
#include "Cell.h"
#include "Array2D.h"

/* A* path finding implementation
 * based on https://github.com/waitingfy/Cocos2d-x_PathFinding
 */

class PathFinder
{
public:
    PathFinder():_fieldWidth(0),_fieldHeight(0),_pathMap(_fieldWidth,_fieldHeight)
    {

    }
    // set dimensions of field
    void setDimensions(int width, int height);
    // set passable status for current coords
    void setPassable(int x, int y, int passable);
    // set start point
    void setStartpoint(int x, int y);
    // set end point
    void setEndPoint(int x, int y);
    // perform pathfinding
    void findPath();
    // get calculated path
    deque<Cell*> getPath() {
        return _path;
    }


private:
    int _fieldWidth;
    int _fieldHeight;

    int _startposX;
    int _startposY;

    const int DIRECTION[4][2]={
        {0,1},  //north
        {1,0},  //east
        {0,-1},  //south
        {-1,0},  //west
    };

    Array2D<Cell> _pathMap;
    deque<Cell*> _path;
};

#endif // PATHFINDER_H
