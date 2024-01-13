CFLAGS := `sdl2-config --libs --cflags` -ggdb3 -O0 -Wall -lSDL2_image -lm -std=c++20
CFLAGS_RELEASE := `sdl2-config --libs --cflags` -ggdb3 -O3 -Wall -lSDL2_image -lm  -std=c++20
LOCAL_INCLUDE = -I./include
CPP_FILES := $(wildcard ./src/*.cpp)

run:
	@make clean
	@g++ $(CPP_FILES) $(LOCAL_INCLUDE) -o target/out $(CFLAGS)
	@./target/out

release:
	@make clean
	@g++ $(CPP_FILES) $(LOCAL_INCLUDE) -o target/out $(CFLAGS_RELEASE)
	@./target/out

clean:
	@rm -rf ./target/out

