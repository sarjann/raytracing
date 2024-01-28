#pragma once
struct Point {
    double x;
    double y;
    double z;
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

struct ColorIntensity {
    double r;
    double g;
    double b;
};

struct Intercept {
    bool intercepts;
    double distance;
    Color color;
    Point point;
};

double vector_dot(Point v1, Point v2);

Point vector_add(Point v1, Point v2);
Point vector_sub(Point v1, Point v2);
Point vector_scalar(Point v1, double scalar);
Point vector_div(Point v1, double scalar);
double vector_mag(Point v1);
Color color_scalar(Color v1, double scalar);
Color color_intensity_mul_to_col(Color v1, ColorIntensity intensity);
ColorIntensity color_intensity_mul(ColorIntensity i1, ColorIntensity i2);
ColorIntensity color_intensity_add(ColorIntensity i1, ColorIntensity i2);
ColorIntensity color_intensity_clamp(ColorIntensity intensity);
