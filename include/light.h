#pragma once

#include "generic.h"
#include "render_object.h"

class Light {
public:
    ColorIntensity intensity;
    Color color;
    Point position;
    virtual bool is_shadowed(Point point, RenderObject *render_object) {
        throw "Not Implemented";
    };
    virtual void custom() {}
    virtual ColorIntensity get_intensity(RenderObject *render_object,
                                         Point point) {
        throw "Not Implemented";
    };
};

class AmbientLight : public Light {
public:
    AmbientLight(ColorIntensity intensity, Color color = Color{255, 255, 255}) {
        this->intensity = intensity;
        this->color = color;
    };
    Point position;
    bool is_shadowed(Point point, RenderObject *render_object) {
        return false;
    };
    void custom() {}
    ColorIntensity get_intensity(RenderObject *render_object, Point point) {
        return this->intensity;
    };
};

class PointLight : public Light {
public:
    Point position;
    PointLight(ColorIntensity intensity, Point position,
               Color color = Color{255, 255, 255}) {
        this->intensity = intensity;
        this->position = position;
        this->color = color;
    };
    bool is_shadowed(Point point, RenderObject *render_object) {
        bool intercepts =
                render_object->is_shadowed_point(point, this->position);
        return intercepts;
    };
    void custom() { this->update_state(); }
    void update_state() {
        double scalar = 0.1;
        this->position.x += (rand() % 10 - 5) * scalar;
        this->position.y += (rand() % 10 - 5) * scalar;
        this->position.z += (rand() % 10 - 5) * scalar;
    };
    ColorIntensity get_intensity(RenderObject *render_object, Point point) {
        Point direction = vector_sub(point, this->position);

        ColorIntensity intensity =
                render_object->get_directional_intensity(point, direction);
        intensity = color_intensity_mul(intensity, this->intensity);
        return intensity;
    };
};

class DirectionalLight : public Light {
public:
    Point direction;
    DirectionalLight(ColorIntensity intensity, Point direction,
                     Color color = Color{255, 255, 255}) {
        this->intensity = intensity;
        this->direction = direction;
        this->color = color;
    };
    bool is_shadowed(Point point, RenderObject *render_object) {
        bool intercepts =
                render_object->is_shadowed_directional(point, this->direction);
        return intercepts;
    };
    ColorIntensity get_intensity(RenderObject *render_object, Point point) {
        ColorIntensity intensity = render_object->get_directional_intensity(
                point, this->direction);
        intensity = color_intensity_mul(intensity, this->intensity);
        return intensity;
    };
};
