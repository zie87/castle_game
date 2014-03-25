#ifndef DIRECTION_H_
#define DIRECTION_H_

#include <vector>
#include <iosfwd>
#include "point.h"

/**
 * Just regular counter-clockwise directions.
 */
enum class Direction : int {
    East,
    NorthEast,
    North,
    NorthWest,
    West,
    SouthWest,
    South,
    SouthEast
};

Direction RotatedLeft(const Direction &dir, int times = 1);
Direction RotatedRight(const Direction &dir, int times = 1);

int LeftRotates(const Direction &lhs, const Direction &rhs);
int RightRotates(const Direction &lhs, const Direction &rhs);

int MinRotates(const Direction &lhs, const Direction &rhs);

Direction GetOppositeDirection(const Direction &direction);

/**
 * \brief It maps regular 2D-plane on Direction.
 * \note The angle of 0 points on Direction::East
 */
Direction RadiansToDirection(double angle);
    
Direction PointsDirection(const Point &lhs, const Point &rhs);

bool operator==(const Direction &lhs, const Direction &rhs);

using DirectionSet = std::vector<Direction>;

const DirectionSet FullDirectionSet = DirectionSet {
    Direction::East,
    Direction::NorthEast,
    Direction::North,
    Direction::NorthWest,
    Direction::West,
    Direction::SouthWest,
    Direction::South,
    Direction::SouthEast
};
    
const DirectionSet MainDirectionSet = DirectionSet {
    Direction::East,
    Direction::North,
    Direction::West,
    Direction::South
};

const DirectionSet SingleDirectionSet = DirectionSet {
    Direction::South
};

Direction ClosestDirection(const DirectionSet &set, const Direction &dir);

std::ostream &operator<<(std::ostream &out, const Direction &dir);

#endif
