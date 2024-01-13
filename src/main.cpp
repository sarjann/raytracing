#include <SDL2/SDL.h>
#include <chrono>
#include <future>
#include <generic.h>
#include <iostream>
#include <light.h>
#include <limits>
#include <render_object.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <utility>
#include <vector>

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 1000
#define VIEW_WIDTH 1000
#define VIEW_HEIGHT 1000
#define VIEW_DISTANCE 1000

const int NUM_THREADS = 10;

Point canvas_to_view_transform(CanvasPoint canvas) {
    double x = canvas.x * VIEW_WIDTH / SCREEN_WIDTH;
    double y = canvas.y * VIEW_HEIGHT / SCREEN_HEIGHT;
    double z = VIEW_DISTANCE;
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

void SDL_Draw(SDL_Renderer *renderer, Color color, CanvasPoint canvas) {
    canvas = change_to_matrix_coords(canvas);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(renderer, canvas.x, canvas.y);
}

bool is_shadowed(Point point, Light *light,
                 std::vector<RenderObject *> *render_objects) {
    for (int i = 0; i < render_objects->size(); i++) {
        RenderObject *object = render_objects->at(i);
        bool intercepts = light->is_shadowed(point, object);
        if (intercepts) {
            return true;
        }
    }
    return false;
}

Color raytrace(Point viewport, std::vector<RenderObject *> *render_objects,
               std::vector<Light *> *lights) {
    Point origin = Point{0, 0, 0};
    // Color color = Color{0, 0, 0}; // Black
    Color color = Color{255, 255, 255}; // White

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

    bool shadows = true;
    // bool shadows = false;
    if (intercepts) {
        // Lighting
        double intensity = 0;
        for (int i = 0; i < lights->size(); i++) {
            Light *light = lights->at(i);
            // Check if shadowed
            if (shadows) {
                bool shadowed =
                        is_shadowed(intercept_point, light, render_objects);
                if (shadowed) {
                    // std::cout << "Shadowed" << std::endl;
                    continue;
                } else {
                    // std::cout << "Not Shadowed" << std::endl;
                }
            }
            // Calculate intensity
            intensity += light->get_intensity(closest_object, intercept_point);
        }
        color = color_scalar(closest_object->get_color(), intensity);
    }
    return color;
}

std::pair<std::pair<int, int>, Color>
get_pixel(int i, int j, std::vector<RenderObject *> *render_objects,
          std::vector<Light *> *lights) {
    CanvasPoint canvas = CanvasPoint{i, j, 0};
    Point viewport = canvas_to_view_transform(canvas);
    Color color = raytrace(viewport, render_objects, lights);
    return std::make_pair(std::make_pair(i, j), color);
}

std::vector<std::pair<std::pair<int, int>, Color>>
get_pixels(std::pair<int, int> width_range,
           std::vector<RenderObject *> *render_objects,
           std::vector<Light *> *lights) {
    std::vector<std::pair<std::pair<int, int>, Color>> pixels;

    for (int i = width_range.first; i < width_range.second; i++) {
        for (int j = -SCREEN_HEIGHT / 2; j < SCREEN_HEIGHT / 2; j++) {
            CanvasPoint canvas = CanvasPoint{i, j, 0};
            Point viewport = canvas_to_view_transform(canvas);
            Color color = raytrace(viewport, render_objects, lights);
            pixels.push_back(std::make_pair(std::make_pair(i, j), color));
        }
    }
    return pixels;
}

void render(SDL_Renderer *renderer, std::vector<RenderObject *> *render_objects,
            std::vector<Light *> *lights) {

    std::vector<std::future<std::vector<std::pair<std::pair<int, int>, Color>>>>
            futures;

    int batch_length = SCREEN_WIDTH / NUM_THREADS;

    for (int start = -SCREEN_WIDTH / 2; start < SCREEN_WIDTH / 2;
         start += batch_length) {
        int end = start + batch_length;

        std::pair<int, int> width_range = std::make_pair(start, end);
        futures.push_back(std::async(std::launch::async, get_pixels,
                                     width_range, render_objects, lights));
    }

    for (auto &future : futures) {
        auto result = future.get();
        int result_size = result.size();
        for (int k = 0; k < result_size; k++) {
            std::pair<std::pair<int, int>, Color> pixel = result.at(k);
            std::pair<int, int> coords = pixel.first;
            Color color = pixel.second;
            SDL_Draw(renderer, color,
                     CanvasPoint{coords.first, coords.second, 0});
        }
    }
    futures.clear();
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

void update_state(SDL_Renderer *renderer,
                  std::vector<RenderObject *> *render_objects,
                  std::vector<Light *> *lights) {
    // for (int i = 0; i < render_objects->size(); i++) {
    //     RenderObject *object = render_objects->at(i);
    //     object->update_state();
    // }
    for (int i = 0; i < lights->size(); i++) {
        Light *light = lights->at(i);
        light->custom();
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
    Point offset = Point{0, 0, 2};

    // Make lights
    std::vector<Light *> lights;
    // lights.push_back(new AmbientLight(0.2));
    // lights.push_back(new PointLight(0.6, vector_add(Point{2, 1, 0},
    //                 offset)));
    // lights.push_back(new DirectionalLight(0.2, Point{1, 4, 4}));


    // lights.push_back(new DirectionalLight(0.2, Point{1, 4, 4}));

    // lights.push_back(new PointLight(0.6, vector_add(Point{2, 1, 0},
    // offset)));


    lights.push_back(new PointLight(0.6, vector_add(Point{1, -5, 0},
                    offset)));
    // lights.push_back(new DirectionalLight(0.4, Point{1, -5, 0}));

    // Make renderable objects
    std::vector<RenderObject *> render_objects;
    // render_objects.push_back(
    //         create_sphere(Point{0, 0, 30}, 0.001, Color{255, 255, 255},
    //         1000));
    render_objects.push_back(
            create_sphere(Point{0, -1, 3}, 1, Color{255, 0, 0}, 500));
    render_objects.push_back(
            create_sphere(Point{2, 0, 4}, 1, Color{0, 0, 255}, 500));
    render_objects.push_back(
            create_sphere(Point{-2, 0, 4}, 1, Color{0, 255, 0}, 500));
    // render_objects.push_back(
    //         create_sphere(Point{0, 0, 60}, 50, Color{255, 255, 255}, 1000));
    render_objects.push_back(
            create_sphere(Point{0, -5001, 0}, 5000, Color{255, 255, 0}, 1000));

    for (int i = 0; i < render_objects.size(); i++) {
        RenderObject *rend = render_objects.at(i);
        Point position = rend->get_position();
        position = vector_add(position, offset);
        rend->set_position(position);
    }
    // render_objects.push_back(
    //         create_sphere(Point{1, -1.5, 5}, 1, Color{0, 255, 255}, 500));

    auto start = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
    int frame_count = 0;
    while (!close) {
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        update_state(renderer, &render_objects, &lights);

        render(renderer, &render_objects, &lights);

        SDL_RenderPresent(renderer);

        frame_count++;
        if (frame_count % 10 == 0) {
            auto current =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now()
                                    .time_since_epoch());
            auto diff = current - start;
            start = current;

            double diff_double = diff.count();

            printf("fps: %f\r", frame_count / diff_double * 1000);
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
