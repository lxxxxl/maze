#ifndef TILES_H
#define TILES_H

#include <vector>
#include <stdint.h>

using namespace std;

#define SPRITES_PER_LINE    7
#define SPRITE_SIZE_TILESET 16

enum Walls{
    UpDown = 226,
    LeftRight = 220,
    Cross = 227,
    DownLeft = 221,
    DownRight = 219,
    UpLeft = 235,
    UpRight = 233,
    VertLeft = 217,
    VertRight = 218,
    HorUp = 225,
    HorDown = 224
};

enum Objects{
    Grass = 3,
    Player = 250,
    Exit = 207,
    CheckpointHighlighter = 298
};

static vector<uint16_t> InnerObstacles = {7, 8, 9, 14, 15, 16, 17, 21, 34, 42, 43, 164, 188};
static vector<uint16_t> OuterObstacles = {4, 5, 6, 10, 11, 12, 13, 27, 202, 209, 238, 239, 237};
static vector<uint16_t> Checkpoints = {44, 45, 46, 47};

static string TilesFilename = "toen.png";

#endif // TILES_H
