#pragma once
#include <generic.h>
#include <math.h>

class RenderObject {
public:
    void set_position(Point position) { this->position = position; }
    void set_color(Color color) { this->color = color; }
    Color get_color() { return this->color; }
    virtual Intercept trace(Point origin, Point viewport) {
        throw "Not Implemented";
    };
    virtual float get_directional_intensity(Point point, Point direction) {
        throw "Not Implemented";
    };
    void update_state() {
        float scalar = 0.001;
        this->position.x += (rand() % 10 - 5) * scalar;
        this->position.y += (rand() % 10 - 5) * scalar;
        this->position.z += (rand() % 10 - 5) * scalar;
    };

protected:
    Point position;
    Color color = Color{0, 0, 0, 255};
    float specular = 0;
};

class Sphere : public RenderObject {
public:
    float radius;
    Intercept trace(Point origin, Point viewport) {
        Intercept intercept = Intercept{
                false, float(std::numeric_limits<int>::max()), this->color};
        Point direction = vector_sub(viewport, origin);

        float a = vector_dot(direction, direction);
        Point co = vector_sub(origin, this->position);
        float b = 2 * vector_dot(direction, co);
        float c = vector_dot(co, co) - this->radius * this->radius;
        float b24ac = b * b - 4 * a * c;
        float sqrt_b24ac = sqrt(b24ac);

        if (b24ac >= 0) {
            float distance_plus = (-b + sqrt_b24ac) / (2 * a);
            if (distance_plus < intercept.distance) {
                intercept.intercepts = true;
                intercept.point = vector_add(
                        origin, vector_scalar(direction, distance_plus));
                intercept.distance = distance_plus;
            }

            float distance_minus = (-b - sqrt_b24ac) / (2 * a);
            if (distance_minus < intercept.distance) {
                intercept.intercepts = true;
                intercept.distance = distance_minus;
                intercept.point = vector_add(
                        origin, vector_scalar(direction, distance_minus));
            }
        }
        return intercept;
    };

    float get_directional_intensity(Point point, Point direction) {
        float intensity = 0;

        // Common
        Point norm_vector = vector_sub(point, this->position);
        float norm_dot_direction = vector_dot(norm_vector, direction);

        // Diffuse
        if (norm_dot_direction > 0) {
            float norm_mag = vector_mag(norm_vector);
            float direction_mag = vector_mag(direction);

            float divisor = norm_mag * direction_mag;
            float diffuse_intensity = norm_dot_direction /= divisor;

            intensity += diffuse_intensity;
        }

        // Specular
        if (this->specular != 0) {
            Point reflection = vector_sub(
                    vector_scalar(norm_vector, 2 * norm_dot_direction),
                    direction);

            Point v = vector_sub(Point{0, 0, 0}, point);
            float v_mag = vector_mag(v);
            float reflect_product = vector_dot(reflection, v);
            float reflection_mag = vector_mag(reflection);
            float reflection_intensity = pow(
                    reflect_product / (reflection_mag * v_mag), this->specular);

            if (reflection_intensity > 0) {
                intensity += reflection_intensity;
            }
        }
        if (intensity > 1) {
            intensity = 1;
        }
        if (intensity < 0) {
            intensity = 0;
        }
        return intensity;
    };

    Sphere(Point position, float radius, Color color = Color{0, 0, 0, 255},
           float specular = 0) {
        this->position = position;
        this->radius = radius;
        this->color = color;
        this->specular = specular;
    }
};

Sphere *create_sphere(Point position, float radius,
                      Color color = Color{0, 0, 0, 255}, float specular = 0) {
    Sphere *sphere = (Sphere *)malloc(sizeof(Sphere));
    sphere = new Sphere(position, radius, color, specular);
    return sphere;
}
