#include <SDL2/SDL.h>
#include <chrono>
#include <iostream>
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 1000
#define VIEW_WIDTH 1000
#define VIEW_HEIGHT 1000
#define VIEW_DISTANCE 1000

// Using the fast sqrt
float q_rsqrt(float number) {
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number;
    i = *(long *)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float *)&i;
    y = y * (threehalfs - (x2 * y * y));
    y = y * (threehalfs - (x2 * y * y));
    return y;
}

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
        // float sqrt_b24ac = q_rsqrt(b24ac);
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

Point canvas_to_view_transform(CanvasPoint canvas) {
    float x = canvas.x * VIEW_WIDTH / SCREEN_WIDTH;
    float y = canvas.y * VIEW_HEIGHT / SCREEN_HEIGHT;
    float z = VIEW_DISTANCE;
    Point point = Point{x, y, z};
    return point;
}

CanvasPoint view_to_canvas_transform(Point point) {
    int x = (point.x) * SCREEN_WIDTH / VIEW_WIDTH;
    int y = (point.y) * SCREEN_HEIGHT / VIEW_HEIGHT;
    CanvasPoint canvas = CanvasPoint{x, y, 0};
    return canvas;
}

CanvasPoint change_to_matrix_coords(CanvasPoint point) {
    CanvasPoint new_point = CanvasPoint{point.x + SCREEN_WIDTH / 2,
                                        -point.y + SCREEN_HEIGHT / 2, point.z};
    return new_point;
}

void SDL_Draw(SDL_Renderer *renderer, Point point, Color color,
              CanvasPoint canvas) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(renderer, canvas.x, canvas.y);
}

Color raytrace(Point viewport, std::vector<RenderObject *> *render_objects,
               std::vector<Light *> *lights) {
    Point origin = Point{0, 0, 0};
    Color color = Color{0, 0, 0}; // Black
    // Color color = Color{255, 255, 255}; // White

    int distance = std::numeric_limits<int>::max();
    bool intercepts = false;
    Point intercept_point = Point{0, 0, 0};
    RenderObject *closest_object = NULL;

    for (int i = 0; i < render_objects->size(); i++) {
        Intercept intercept = render_objects->at(i)->trace(origin, viewport);

        if (intercept.intercepts and intercept.distance < distance) {
            distance = intercept.distance;
            color = intercept.color;
            intercept_point = intercept.point;
            intercepts = true;
            closest_object = render_objects->at(i);
        }
    }

    if (intercepts) {
        // Lighting
        float intensity = 0;
        for (int i = 0; i < lights->size(); i++) {
            Light *light = lights->at(i);
            intensity += light->get_intensity(closest_object, intercept_point);
        }
        color = color_scalar(closest_object->get_color(), intensity);
    }
    return color;
}

void render(SDL_Renderer *renderer, std::vector<RenderObject *> *render_objects,
            std::vector<Light *> *lights) {
    for (int i = -SCREEN_WIDTH / 2; i < SCREEN_WIDTH / 2; i++) {
        for (int j = -SCREEN_HEIGHT / 2; j < SCREEN_HEIGHT / 2; j++) {
            CanvasPoint canvas = CanvasPoint{i, j, 0};
            Point viewport = canvas_to_view_transform(canvas);
            Color color = raytrace(viewport, render_objects, lights);
            SDL_Draw(renderer, viewport, color,
                     change_to_matrix_coords(canvas));
        }
    }
}

void free_memory(std::vector<RenderObject *> *render_objects,
                 std::vector<Light *> *lights) {
    std::cout << "Freeing Memory" << std::endl;
    for (int i = 0; i < render_objects->size(); i++) {
        RenderObject *object = render_objects->at(i);
        free(object);
    }
    for (int i = 0; i < lights->size(); i++) {
        Light *light = lights->at(i);
        free(light);
    }
}

Sphere *create_sphere(Point position, float radius,
                      Color color = Color{0, 0, 0, 255}, float specular = 0) {
    Sphere *sphere = (Sphere *)malloc(sizeof(Sphere));
    sphere = new Sphere(position, radius, color, specular);
    return sphere;
}

void update_state(SDL_Renderer *renderer,
                  std::vector<RenderObject *> *render_objects) {
    for (int i = 0; i < render_objects->size(); i++) {
        RenderObject *object = render_objects->at(i);
        object->update_state();
    }
}

int main(int argc, char *argv[]) {
    srand(1);

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("Error Initializing SDL: %s\n", SDL_GetError());
    }
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *surface;

    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window,
                                &renderer);

    surface = SDL_GetWindowSurface(window);

    bool close = false;
    // Make lights
    std::vector<Light *> lights;
    lights.push_back(new AmbientLight(0.2));
    lights.push_back(new PointLight(0.6, Point{2, 1, 0}));
    lights.push_back(new DirectionalLight(0.2, Point{1, 4, 4}));

    // Make renderable objects
    std::vector<RenderObject *> render_objects;
    render_objects.push_back(
            create_sphere(Point{0, -1, 3}, 1, Color{255, 0, 0}, 500));
    render_objects.push_back(
            create_sphere(Point{2, 0, 4}, 1, Color{0, 0, 255}, 500));
    render_objects.push_back(
            create_sphere(Point{-2, 0, 4}, 1, Color{0, 255, 0}, 10));
    // render_objects.push_back(
    //         create_sphere(Point{0, -5001, 0}, 5000, Color{255, 255, 0},
    //         1000));

    // int num_to_add = 10;
    // // Random colors
    // for (int i = 0; i < num_to_add; i++) {
    //     int r = rand() % 255;
    //     int g = rand() % 255;
    //     int b = rand() % 255;
    //     render_objects.push_back(
    //             create_sphere(Point{rand() % 10 - 5, rand() % 10 - 5, 50},
    //                           0.5 * (rand() % 10) + 1, Color{r, g, b}));
    // }

    auto start = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
    int frame_count = 0;
    while (!close) {
        if (true) {
            // Clear screen
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            update_state(renderer, &render_objects);

            render(renderer, &render_objects, &lights);

            SDL_RenderPresent(renderer);
        };
        for (int i = 0; i < lights.size(); i++) {
            lights[i]->custom();
        }

        frame_count++;
        if (frame_count % 100 == 0) {
            auto current =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now()
                                    .time_since_epoch());
            auto diff = current - start;
            start = current;

            float diff_float = diff.count();

            printf("fps: %f\r", frame_count / diff_float * 1000);
            std::flush(std::cout);
            frame_count = 0;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                close = true;
                free_memory(&render_objects, &lights);
            }
        }
    }
}
