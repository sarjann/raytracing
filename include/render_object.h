#pragma once
#include <generic.h>
#include <iostream>
#include <math.h>

class RenderObject {
public:
    void set_position(Point position) { this->position = position; }
    Point get_position() { return this->position; }
    void set_color(Color color) { this->color = color; }
    Color get_color() { return this->color; }
    virtual Intercept trace(Point origin, Point viewport) {
        throw "Not Implemented";
    };
    virtual bool is_shadowed_directional(Point origin, Point direction) {
        throw "Not Implemented";
    }
    virtual bool is_shadowed_point(Point origin, Point point) {
        throw "Not Implemented";
    }
    virtual ColorIntensity get_directional_intensity(Point point,
                                                     Point direction) {
        throw "Not Implemented";
    };
    void update_state() {
        double scalar = 0.01;
        this->position.x += (rand() % 10 - 5) * scalar;
        this->position.y += (rand() % 10 - 5) * scalar;
        this->position.z += (rand() % 10 - 5) * scalar;
    };

protected:
    Point position;
    Color color = Color{0, 0, 0, 255};
    double specular = 0;
};

class Sphere : public RenderObject {
public:
    double radius;
    Intercept trace(Point origin, Point viewport) {
        Intercept intercept = Intercept{
                false, double(std::numeric_limits<int>::max()), this->color};
        Point direction = vector_sub(viewport, origin);
        direction = vector_div(direction, vector_mag(direction));

        double a = vector_dot(direction, direction);
        Point co = vector_sub(origin, this->position);
        double b = 2 * vector_dot(direction, co);
        double c = vector_dot(co, co) - this->radius * this->radius;
        double b24ac = b * b - 4 * a * c;
        double sqrt_b24ac = sqrt(b24ac);

        if (b24ac >= 0) {
            double distance_plus = (-b + sqrt_b24ac) / (2 * a);
            if (distance_plus < intercept.distance && distance_plus > 0.0) {
                intercept.intercepts = true;
                intercept.point = vector_add(
                        origin, vector_scalar(direction, distance_plus));
                intercept.distance = distance_plus;
            }

            double distance_minus = (-b - sqrt_b24ac) / (2 * a);
            if (distance_minus < intercept.distance && distance_minus > 0.0) {
                intercept.intercepts = true;
                intercept.distance = distance_minus;
                intercept.point = vector_add(
                        origin, vector_scalar(direction, distance_minus));
            }
            if (intercept.intercepts) {
                // std::cout << intercept.distance << std::endl;
            }
        }
        return intercept;
    };

    bool is_shadowed_directional(Point origin, Point light_direction) {
        double epsilon = 0.0001;
        Point norm_towards_light =
                vector_div(light_direction, -vector_mag(light_direction));

        origin = vector_add(origin, vector_scalar(norm_towards_light, epsilon));
        Point direction = norm_towards_light;

        double a = vector_dot(direction, direction);
        Point co = vector_sub(origin, this->position);
        double b = 2 * vector_dot(direction, co);
        double c = vector_dot(co, co) - this->radius * this->radius;
        double b24ac = b * b - 4 * a * c;
        double sqrt_b24ac = sqrt(b24ac);

        if (b24ac >= 0) {
            double distance_plus = (-b + sqrt_b24ac) / (2 * a);
            if (distance_plus > 0) {
                return true;
            }
            double distance_minus = (-b - sqrt_b24ac) / (2 * a);
            if (distance_minus > 0) {
                return true;
            }
        }
        return false;
    };

    bool is_shadowed_point(Point origin, Point light_source) {
        double epsilon = 0.0001;
        Point light_direction = vector_sub(origin, light_source);
        double light_distance = vector_mag(light_direction);
        Point norm_towards_light =
                vector_div(light_direction, -vector_mag(light_direction));

        origin = vector_add(origin, vector_scalar(norm_towards_light, epsilon));
        Point direction = norm_towards_light;

        double a = vector_dot(direction, direction);
        Point co = vector_sub(origin, this->position);
        double b = 2 * vector_dot(direction, co);
        double c = vector_dot(co, co) - this->radius * this->radius;
        double b24ac = b * b - 4 * a * c;
        double sqrt_b24ac = sqrt(b24ac);

        if (b24ac >= 0) {
            double distance_plus = (-b + sqrt_b24ac) / (2 * a);
            if (distance_plus > 0 && distance_plus < light_distance) {
                return true;
            }
            double distance_minus = (-b - sqrt_b24ac) / (2 * a);
            if (distance_minus > 0 && distance_minus < light_distance) {
                return true;
            }
        }
        return false;
    };

    ColorIntensity get_directional_intensity(Point point, Point direction) {
        double intensity = 0;

        // Common
        Point norm_vector = vector_sub(point, this->position);
        Point reflected_direction = vector_scalar(direction, -1);

        double norm_dot_direction =
                vector_dot(norm_vector, reflected_direction);

        // Diffuse
        if (norm_dot_direction > 0) {
            double norm_mag = vector_mag(norm_vector);
            double direction_mag = vector_mag(direction);

            double divisor = norm_mag * direction_mag;
            double diffuse_intensity = norm_dot_direction /= divisor;

            intensity += diffuse_intensity;
        }

        // Specular
        if (this->specular != 0) {
            Point reflection = vector_sub(
                    vector_scalar(norm_vector, 2 * norm_dot_direction),
                    reflected_direction);

            Point v = vector_sub(Point{0, 0, 0}, point);
            double v_mag = vector_mag(v);
            double reflect_product = vector_dot(reflection, v);
            double reflection_mag = vector_mag(reflection);
            double reflection_intensity = pow(
                    reflect_product / (reflection_mag * v_mag), this->specular);

            if (reflection_intensity > 0) {
                intensity += reflection_intensity;
            }
        }
        return ColorIntensity{intensity, intensity, intensity};
    };

    Sphere(Point position, double radius, Color color = Color{0, 0, 0, 255},
           double specular = 0) {
        this->position = position;
        this->radius = radius;
        this->color = color;
        this->specular = specular;
    }
};

Sphere *create_sphere(Point position, double radius,
                      Color color = Color{0, 0, 0, 255}, double specular = 0) {
    Sphere *sphere = (Sphere *)malloc(sizeof(Sphere));
    sphere = new Sphere(position, radius, color, specular);
    return sphere;
}
