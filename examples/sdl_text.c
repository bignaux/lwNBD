/*
 * WIP
 *
 *
 * create a subscriber.c and get message then print them.
 *
 * SDL2 has official PS2 port by fjtrujy https://wiki.libsdl.org/SDL2/README/ps2
 * see https://wiki.libsdl.org/SDL2/SDL_UserEvent
 */

#include <lwnbd/lwnbd.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <freetype/freetype.h>
#include <ft2build.h>

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 448
#define SUCCESS       0
#define FAILED        -1

/* use bin2c on font as PS2 dev does for irx */
extern const char Vera[];
extern const size_t size_Vera;

void SDL_PS2_SKIP_IOP_RESET() {};

int main(int argc, char **argv)
{
    SDL_bool quit = SDL_FALSE;
    SDL_Event event;
    Uint8 red = 0, green = 0, blue = 0;
    int32_t ret;
    char buf[512];

    //    lwnbd_plugin_h memplg;
    //    char *memtest;
    //
    //    memplg = lwnbd_plugin_init(memory_plugin_init);
    //    memtest = calloc(1, 512);
    //
    //    struct memory_config memh = {
    //        .base = (uint64_t)memtest,
    //        .name = "test",
    //        .size = 512,
    //        .desc = "",
    //    };
    //    lwnbd_plugin_new(memplg, &memh);

    init_scr();
    ret = init_platform();
    if (ret == 0)
        scr_printf("init success !\n");

    sleep(60); // some delay is required by usb mass storage driver

    FILE *file = fopen("mass:/sdl.log", "a");
    if (!file) {
        scr_printf("fopen() !\n");
        return -1;
    }

    ret = snprintf(buf, 512, "tst test tests\n");
    fwrite(&buf, ret, 1, file);
    fflush(file);
    fclose(file);

    /*
     * SDL_Init should do the patches stuff and reset IOP
     */
    // SDL_INIT_JOYSTICK SDL_INIT_GAMECONTROLLER
    if (SDL_Init(SDL_INIT_VIDEO) != SUCCESS) {
        ret = snprintf(buf, 512, "Failed to initialize SDL: %s\n",
                       SDL_GetError());
        fwrite(buf, ret, 1, file);
        fflush(file);
        goto close;
    }

    /* SDL_ttf has not PS2 specific code, this just failed */
    if (TTF_Init() == -1) {
        ret = snprintf(buf, 512, "Failed to initialize TTF: %s\n", TTF_GetError());
        fwrite(buf, ret, 1, file);
        fflush(file);
        goto close;
    }

    /* The FreeType font engine/library */
    //    static FT_Library library = NULL;
    //    //	static int TTF_initialized = 0;
    //
    //    FT_Error error = FT_Init_FreeType(&library);
    //    if (error) {
    //        //        TTF_SetFTError("Couldn't init FreeType engine", error);
    //        init_scr();
    //        scr_printf("Failed to initialize FreeType engine\n");
    //
    //        return -1;
    //    }

    // Create a window context
    SDL_Window *window = SDL_CreateWindow("main", SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT,
                                          SDL_WINDOW_FULLSCREEN);
    if (!window) {
        scr_printf("SDL_CreateWindow: %s\n", SDL_GetError());
        return (-1);
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        scr_printf("SDL_CreateRenderer: %s\n", SDL_GetError());
        return (-1);
    }

    // Load the font data into a memory buffer
    SDL_RWops *pFontMem = SDL_RWFromConstMem(Vera, size_Vera);

    if (!pFontMem) {
        goto quit;
    }

    // Load the font from the memory buffer
    TTF_Font *font = TTF_OpenFontRW(pFontMem, 1, 32);

    if (!font) {
        goto quit;
    }

    SDL_Color color = {255, 255, 255};

    SDL_Surface *surface = TTF_RenderText_Solid(font, "Hello world", color);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    int texW = 0;
    int texH = 0;
    SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
    SDL_Rect dstrect = {0, 0, texW, texH};

    /* main SDL loop */

    while (!quit) {
        SDL_SetRenderDrawColor(renderer, red, green, blue, 0xFF); // SDL_ALPHA_OPAQUE
        SDL_RenderClear(renderer);

        //        SDL_WaitEvent(&event);
        //
        //        switch (event.type) {
        //        	case SDL_JOYBUTTONDOWN:
        //        	case SDL_KEYDOWN:
        //        		red += 10;
        //
        //            case SDL_QUIT:
        //                quit = SDL_TRUE;
        //                break;
        //        }

        SDL_RenderCopy(renderer, texture, NULL, &dstrect);
        SDL_RenderPresent(renderer);
    }

/* unload */
quit:
    scr_printf("exit\n");
    //    SDL_DestroyTexture(texture);
    //    SDL_FreeSurface(surface);
    //    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    //    TTF_Quit();
    SDL_Quit();
close:
    fclose(file);
    return 0;
}

/*

 qdbus org.mpris.MediaPlayer2.clementine /org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Get org.mpris.MediaPlayer2.Player Metadata
 https://github.com/clementine-player/Clementine/wiki/Controlling-Clementine-from-the-commandline-with-DBus-and-MPRIS#using-python

 https://github.com/wmanley/http-dbus-bridge
 */
