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

void SDL_Draw(SDL_Renderer *renderer, Color color, CanvasPoint canvas) {
    canvas = change_to_matrix_coords(canvas);
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

        // Shadows
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
get_pixels(std::vector<std::pair<int, int>> coords,
           std::vector<RenderObject *> *render_objects,
           std::vector<Light *> *lights) {
    std::vector<std::pair<std::pair<int, int>, Color>> pixels;

    for (int i = 0; i < coords.size(); i++) {
        std::pair<int, int> coord = coords.at(i);
        CanvasPoint canvas = CanvasPoint{coord.first, coord.second, 0};
        Point viewport = canvas_to_view_transform(canvas);
        Color color = raytrace(viewport, render_objects, lights);
        pixels.push_back(std::make_pair(coord, color));
    }
    return pixels;
}

void render(SDL_Renderer *renderer, std::vector<RenderObject *> *render_objects,
            std::vector<Light *> *lights) {
    const int NUM_THREADS = 8;
    const int AREA = SCREEN_WIDTH * SCREEN_HEIGHT;
    const int BATCH_SIZE = AREA / NUM_THREADS;

    std::vector<std::pair<int, int>> coords;
    std::vector<std::future<std::vector<std::pair<std::pair<int, int>, Color>>>>
            futures;

    for (int i = -SCREEN_WIDTH / 2; i < SCREEN_WIDTH / 2; ++i) {
        for (int j = -SCREEN_HEIGHT / 2; j < SCREEN_HEIGHT / 2; ++j) {
            coords.push_back(std::make_pair(i, j));
            if (coords.size() == BATCH_SIZE) {
                futures.push_back(std::async(std::launch::async, get_pixels,
                                             coords, render_objects, lights));
                coords.clear();
            }
        }
    }
    if (!coords.empty()) {
        futures.push_back(std::async(std::launch::async, get_pixels, coords,
                                     render_objects, lights));
        coords.clear();
    }

    for (auto &future : futures) {
        future.wait();
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

    auto start = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
    int frame_count = 0;
    while (!close) {
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        update_state(renderer, &render_objects);

        render(renderer, &render_objects, &lights);

        SDL_RenderPresent(renderer);

        for (int i = 0; i < lights.size(); i++) {
            lights[i]->custom();
        }

        frame_count++;
        if (frame_count % 10 == 0) {
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
