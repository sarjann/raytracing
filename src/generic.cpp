#include "generic.h"
#include <cmath>


float vector_dot(Point v1, Point v2) {
    float dot_product = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    return dot_product;
}

Point vector_add(Point v1, Point v2) {
    Point point = Point{v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
    return point;
}

Point vector_sub(Point v1, Point v2) {
    Point point = Point{v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
    return point;
}

Point vector_scalar(Point v1, float scalar) {
    Point point = Point{v1.x * scalar, v1.y * scalar, v1.z * scalar};
    return point;
}

float vector_mag(Point v1) {
    float magnitude = sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);
    return magnitude;
}

Color color_scalar(Color v1, float scalar) {
    Color color =
            Color{int(v1.r * scalar), int(v1.g * scalar), int(v1.b * scalar)};
    return color;
}

