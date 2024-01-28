#include "generic.h"
#include <cmath>

double vector_dot(Point v1, Point v2) {
    double dot_product = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
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

Point vector_scalar(Point v1, double scalar) {
    Point point = Point{v1.x * scalar, v1.y * scalar, v1.z * scalar};
    return point;
}

Point vector_div(Point v1, double scalar) {
    Point point = Point{v1.x / scalar, v1.y / scalar, v1.z / scalar};
    return point;
}

double vector_mag(Point v1) {
    double magnitude = sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z);
    return magnitude;
}

Color color_scalar(Color v1, double scalar) {
    Color color =
            Color{int(v1.r * scalar), int(v1.g * scalar), int(v1.b * scalar)};
    return color;
}

Color color_intensity_mul_to_col(Color v1, ColorIntensity intensity) {
    Color color = Color{int(v1.r * intensity.r), int(v1.g * intensity.g),
                        int(v1.b * intensity.b)};
    return color;
}

ColorIntensity color_intensity_mul(ColorIntensity i1, ColorIntensity i2) {
    ColorIntensity intensity =
            ColorIntensity{i1.r * i2.r, i1.g * i2.g, i1.b * i2.b};
    return intensity;
}

ColorIntensity color_intensity_add(ColorIntensity i1, ColorIntensity i2) {
    ColorIntensity intensity =
            ColorIntensity{i1.r + i2.r, i1.g + i2.g, i1.b + i2.b};
    return intensity;
}

ColorIntensity color_intensity_clamp(ColorIntensity intensity) {
    ColorIntensity new_intensity =
            ColorIntensity{intensity.r, intensity.g, intensity.b};
    if (new_intensity.r > 1) {
        new_intensity.r = 1;
    }
    if (new_intensity.g > 1) {
        new_intensity.g = 1;
    }
    if (new_intensity.b > 1) {
        new_intensity.b = 1;
    }
    return new_intensity;
}
