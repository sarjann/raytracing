#include <SDL2/SDL.h>
// #include <ctime>
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

class RenderObject {
public:
    void set_position(Point position) { this->position = position; }
    void set_color(Color color) { this->color = color; }
    virtual Intercept trace(Point origin, Point viewport) {
        throw "Not Implemented";
    };
    void random_move() {
        float scalar = 0.01;
        this->position.x += (rand() % 10 - 5) * scalar;
        this->position.y += (rand() % 10 - 5) * scalar;
        this->position.z += (rand() % 10 - 5) * scalar;
    }

protected:
    Point position;
    Color color = Color{0, 0, 0, 255};
};

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
        // float sqrt_b24ac = sqrt(b24ac);
        float sqrt_b24ac = q_rsqrt(b24ac);

        if (b24ac >= 0) {
            float distance_plus = (-b + sqrt_b24ac) / (2 * a);
            if (distance_plus < intercept.distance) {
                intercept.intercepts = true;
                intercept.distance = distance_plus;
            }

            float distance_minus = (-b - sqrt_b24ac) / (2 * a);
            if (distance_minus < intercept.distance) {
                intercept.intercepts = true;
                intercept.distance = distance_minus;
            }
        }
        return intercept;
    };
    Sphere(Point position, float radius, Color color = Color{0, 0, 0, 255}) {
        this->position = position;
        this->radius = radius;
        this->color = color;
    }
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

Color raytrace(Point viewport, std::vector<RenderObject *> *render_objects) {
    Point origin = Point{0, 0, 0};
    Color color = Color{0, 0, 0}; // Black

    int distance = std::numeric_limits<int>::max();
    bool intercepts = false;

    for (int i = 0; i < render_objects->size(); i++) {
        Intercept intercept = render_objects->at(i)->trace(origin, viewport);

        if (intercept.intercepts and intercept.distance < distance) {
            distance = intercept.distance;
            color = intercept.color;
            intercepts = true;
        }
    }

    // if (intercepts) {
    //     std::cout << "Distance " << distance << std::endl;
    // }

    return color;
}

void render(SDL_Renderer *renderer,
            std::vector<RenderObject *> *render_objects) {
    for (int i = -SCREEN_WIDTH / 2; i < SCREEN_WIDTH / 2; i++) {
        for (int j = -SCREEN_HEIGHT / 2; j < SCREEN_HEIGHT / 2; j++) {
            CanvasPoint canvas = CanvasPoint{i, j, 0};
            Point viewport = canvas_to_view_transform(canvas);
            Color color = raytrace(viewport, render_objects);
            SDL_Draw(renderer, viewport, color,
                     change_to_matrix_coords(canvas));
        }
    }
}

void free_memory(std::vector<RenderObject *> *render_objects) {
    std::cout << "Freeing Memory" << std::endl;
    for (int i = 0; i < render_objects->size(); i++) {
        RenderObject *object = render_objects->at(i);
        free(object);
    }
}

Sphere *create_sphere(Point position, float radius,
                      Color color = Color{0, 0, 0, 255}) {
    Sphere *sphere = (Sphere *)malloc(sizeof(Sphere));
    sphere = new Sphere(position, radius, color);
    return sphere;
}

void update_state(SDL_Renderer *renderer,
                  std::vector<RenderObject *> *render_objects) {
    for (int i = 0; i < render_objects->size(); i++) {
        RenderObject *object = render_objects->at(i);
        object->random_move();
    }
}

int main(int argc, char *argv[]) {
    srand(1);
    // std::cout.precision(5);

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

    // Make renderable objects
    std::vector<RenderObject *> render_objects;
    render_objects.push_back(
            create_sphere(Point{0, -1, 3}, 1, Color{255, 0, 0}));
    render_objects.push_back(
            create_sphere(Point{2, 0, 4}, 1, Color{0, 0, 255}));
    render_objects.push_back(
            create_sphere(Point{-2, 0, 4}, 1, Color{0, 255, 0}));

    // int num_to_add = 10;
    // // Random colors
    // for (int i = 0; i < num_to_add; i++) {
    //     int r = rand() % 255;
    //     int g = rand() % 255;
    //     int b = rand() % 255;
    //     render_objects.push_back(
    //             create_sphere(Point{rand() % 10, rand() % 10, 50},
    //                           0.05 * (rand() % 10) + 1, Color{r, g, b}));
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

            render(renderer, &render_objects);

            SDL_RenderPresent(renderer);
        };

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
                free_memory(&render_objects);
            }
        }
    }
}
