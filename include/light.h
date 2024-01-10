#pragma once

#include "generic.h"
#include "render_object.h"

class Light {
public:
    float intensity;
    Color color;
    virtual void custom() {}
    virtual float get_intensity(RenderObject *render_object, Point point) {
        throw "Not Implemented";
    };
};

class AmbientLight : public Light {
public:
    AmbientLight(float intensity, Color color = Color{255, 255, 255}) {
        this->intensity = intensity;
        this->color = color;
    };
    void custom() {}
    float get_intensity(RenderObject *render_object, Point point) {
        return this->intensity;
    };
};

class PointLight : public Light {
public:
    Point position;
    PointLight(float intensity, Point position,
               Color color = Color{255, 255, 255}) {
        this->intensity = intensity;
        this->position = position;
        this->color = color;
    };
    void custom() { this->update_state(); }
    void update_state() {
        float scalar = 0.05;
        this->position.x += (rand() % 10 - 5) * scalar;
        this->position.y += (rand() % 10 - 5) * scalar;
        this->position.z += (rand() % 10 - 5) * scalar;
    };
    float get_intensity(RenderObject *render_object, Point point) {
        Point direction = vector_sub(point, this->position);

        float intensity =
                render_object->get_directional_intensity(point, direction);
        intensity = intensity * this->intensity;
        return intensity;
    };
};

class DirectionalLight : public Light {
public:
    Point direction;
    DirectionalLight(float intensity, Point direction,
                     Color color = Color{255, 255, 255}) {
        this->intensity = intensity;
        this->direction = direction;
        this->color = color;
    };
    float get_intensity(RenderObject *render_object, Point point) {
        float intensity = render_object->get_directional_intensity(
                point, this->direction);
        intensity = intensity * this->intensity;
        return intensity;
    };
};
