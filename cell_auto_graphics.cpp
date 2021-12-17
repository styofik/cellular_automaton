#include <SDL.h>
#include <iostream>

#include <bitset>
#include <unordered_map>
#include <random>

class Automaton {
private: 
    uint8_t rule; // Wolfram code of elementary cellular automaton 
    bool* cells;
    size_t size;
    bool cyclic;
    std::unordered_map<std::bitset<3>, bool> rule_map; // For three bits from 0 to 7 return new state of middle
public:
    Automaton(uint8_t rule, size_t size, bool cyclic): rule(rule), size(size), cells(new bool[size]), rule_map(), cyclic(cyclic) {
        for (size_t i = 0; i < size; ++i)
            cells[i] = 0;
        for (uint8_t i = 0; i < 8; ++i) {
            std::bitset<3> key = i; 
            bool mapped = (rule >> i) & 1;
            rule_map.emplace(key, mapped);
        }
    }
    ~Automaton() { delete[] cells; }

    bool* get_cells() { return cells; }
    size_t get_size() { return size; }
    bool& operator[](size_t pos) { return cells[pos]; }

    void flip(size_t pos) {
        cells[pos] = ~cells[pos];
    }

    void move_state() {
        bool* new_cells = new bool[size];
        std::bitset<3> tmp_key;
        for (size_t i = 1; i < size-1; ++i) { 
            tmp_key[0] = cells[i-1];
            tmp_key[1] = cells[i];
            tmp_key[2] = cells[i+1];
            new_cells[i] = rule_map[tmp_key];
        }
        if (cyclic) {
            tmp_key[0] = cells[size-2];
            tmp_key[1] = cells[0];
            tmp_key[2] = cells[1];
            new_cells[0] = rule_map[tmp_key];
            tmp_key[0] = cells[size-2];
            tmp_key[1] = cells[size-1];
            tmp_key[2] = cells[0];
            new_cells[size-1] = rule_map[tmp_key];
        }
        delete[] cells;
        cells = new_cells;
    }
    void show_rule() {
        std::cout << (int)rule << ':' << std::endl; 
        for (uint8_t i = 0; i < 8; ++i) {
            std::cout << rule_map.find(i)->first << " -> " << rule_map.find(i)->second << std::endl;
        }
    }
    void show_cells() {
        for (int i = 0; i < size; ++i)
            if (cells[i])
                std::cout << '*';
            else
                std::cout << ' ';
        std::cout << std::endl;
    }
    void randomize_state(float seed) {
        std::mt19937 eng(seed);
        std::bernoulli_distribution distr(0.5);
        for (int i = 0; i < size; ++i) {
            cells[i] = distr(eng);
        }
    }
    void clear_state() {
        for (int i = 0; i < size; ++i) {
            cells[i] = 0;
        }
    }
    void one_cell_state() {
        clear_state();
        cells[size/2] = 1;
    }
};

int main(int argc, char* argv[]) {
    int SCREEN_WIDTH = 1280;
    int SCREEN_HEIGHT = 720;

    uint8_t rule = 54; // Try 30, 90, 110, 120, 62, 73, 45, 57, 60, 68, 75; Favourite: 73
    //std::cin >> rule;
    size_t auto_size = 640;

    int CELL_SIZE = SCREEN_WIDTH/auto_size;

    std::cout << "Cell size: " << CELL_SIZE << std::endl;

    Automaton cell_auto(rule, auto_size, true);

    cell_auto.randomize_state(190.310);
    //cell_auto.one_cell_state();

    auto res = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_TIMER);
    if (0 != res)
        throw 1;

    SDL_Window *main_window = NULL;
    SDL_Renderer *main_window_renderer = NULL;

    main_window = SDL_CreateWindow(
            "Elementary Cellular Automaton",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH,SCREEN_HEIGHT,
            0
    );

    if (NULL == main_window) {
        std::cout << "Can't create main window" << SDL_GetError() << std::endl;
        throw 1;
    }

    main_window_renderer = SDL_CreateRenderer(
        main_window,
        -1,
        SDL_RENDERER_ACCELERATED
    );
    if (NULL == main_window_renderer) {
        std::cout << "Can't create main renderer" << SDL_GetError() << std::endl;
        throw 1;
    }

    SDL_Event event;

    bool is_done = false; 
    SDL_Rect rect{0, 0, 0, 0};
    while(true) {
        if (SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                break;
            }
            SDL_SetRenderDrawColor(main_window_renderer, 255, 255, 255, 255);

            if(!is_done) {
                for (int y = 0; y < SCREEN_HEIGHT/CELL_SIZE; ++y) {
                    for (int x = 0; x < auto_size; ++x) {
                        if (cell_auto[x])
                            rect = SDL_Rect{x*CELL_SIZE, y*CELL_SIZE, CELL_SIZE, CELL_SIZE};
                            SDL_RenderFillRect(main_window_renderer, &rect);
                            //SDL_RenderDrawPoint(main_window_renderer, x, y);
                    }
                    cell_auto.move_state();
                    SDL_RenderPresent(main_window_renderer);
                    SDL_Delay(5);
                }
                is_done = true;
            }
        }
        SDL_RenderPresent(main_window_renderer);
    }

    SDL_DestroyRenderer(main_window_renderer);
    SDL_DestroyWindow(main_window);
    SDL_Quit();
    return 0;
}
