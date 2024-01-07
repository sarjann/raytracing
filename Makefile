CFLAGS := `sdl2-config --libs --cflags` -ggdb3 -O0 -Wall -lSDL2_image -lm
CFLAGS_RELEASE := `sdl2-config --libs --cflags` -ggdb3 -O3 -Wall -lSDL2_image -lm

run:
	@make clean
	@g++ ./src/main.cpp -o target/out $(CFLAGS)
	@./target/out

release:
	@make clean
	@g++ ./src/main.cpp -o target/out $(CFLAGS_RELEASE)
	@./target/out

clean:
	@rm -rf ./target/out

