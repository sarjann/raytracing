#pragma once
struct Point {
    float x;
    float y;
    float z;
};

struct CanvasPoint {
    int x;
    int y;
    int z;
};

struct Color {
    int r;
    int g;
    int b;
    int a = 255;
};

struct Intercept {
    bool intercepts;
    float distance;
    Color color;
    Point point;
};

float vector_dot(Point v1, Point v2);

Point vector_add(Point v1, Point v2);
Point vector_sub(Point v1, Point v2);
Point vector_scalar(Point v1, float scalar);
float vector_mag(Point v1);
Color color_scalar(Color v1, float scalar);
