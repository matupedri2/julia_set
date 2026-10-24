#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include <SDL/SDL.h>

#include "chrono.h"

// ---------------------------------------------------------------------------

static int cpu_mhz = 0;
static int dump = 0;

#define MAX_ITER 64  //si esto es mas alto mas detalle pero mas lento 
static unsigned int palette[MAX_ITER + 1];  //+1 entradas, -1 borde, 0 interior

#define TILE_SIZE  8  //tamaño tile
#define DEBUG_TILES 0  //0 sin bordes, 1 con 

static void ChronoShow(char* name, int computations)
{
  float ms = ChronoWatchReset();
  float cycles = ms * (1000000.0f / 1000.0f) * (float)cpu_mhz;
  float cyc_per_comp = cycles / (float)computations;
  if ((dump & 15) == 0)
    fprintf(stdout, "%s: %f ms, %d cycles, %f cycles/loop\n", name, ms, (int)cycles, cyc_per_comp);
}


// // Limit framerate and return any remaining time to the OS 

static void FramerateLimit(int max_fps)
{
  static unsigned int frame_time = 0;

  unsigned int t = GetMsTime();
  unsigned int elapsed = t - frame_time;
  // El tiempo sobrante se "regala" al sistema operativo con la funcion POSIX sleep
  const unsigned int limit = 1000 / max_fps;  // 1000 miliseconds / framerate minimo deseado 
  if (elapsed < limit)
    usleep((limit - elapsed) * 1000); // arg in microseconds
  frame_time = GetMsTime();
}

// ---------------------------------------------------------------------------

static inline int JuliaIter(float zr, float zi, float cr, float ci)
{
  int iter = 0;
  while (iter < MAX_ITER) {
    float zr2 = zr * zr;
    float zi2 = zi * zi;
    if (zr2 + zi2 > 4.0f)   // modulo > 2  =>  escapa
      break;
    float tmp = zr2 - zi2 + cr;   // parte real de z²+c
    zi = 2.0f * zr * zi + ci;     // parte imaginaria
    zr = tmp;
    iter++;
  }
  return iter;
}

// ---------------------------------------------------------------------------

static void FillTile(unsigned int* pixels, int px0, int py0,
  int stride, unsigned int color)
{
  for (int y = py0; y < py0 + TILE_SIZE; y++) {
    unsigned int* row = pixels + y * stride + px0;
    for (int x = 0; x < TILE_SIZE; x++)
      row[x] = color;
  }
}

// ---------------------------------------------------------------------------

#if DEBUG_TILES
static void DrawTileBorder(unsigned int* pixels, int px0, int py0,
  int stride, unsigned int color)
{
  // Fila superior e inferior
  for (int x = px0; x < px0 + TILE_SIZE; x++) {
    pixels[x + py0 * stride] = color;
    pixels[x + (py0 + TILE_SIZE - 1) * stride] = color;
  }
  // Columnas izquierda y derecha 
  for (int y = py0 + 1; y < py0 + TILE_SIZE - 1; y++) {
    pixels[px0 + y * stride] = color;
    pixels[px0 + TILE_SIZE - 1 + y * stride] = color;
  }
}
#endif
// ---------------------------------------------------------------------------

static void JuliaSet(unsigned int* pixels, int w, int h, int stride, int frame)
{

  assert(pixels != NULL);
  assert(w > 0 && h > 0 && stride >= w);
  assert(w % TILE_SIZE == 0);
  assert(h % TILE_SIZE == 0);
  assert(TILE_SIZE > 1);

  float t = (float)frame * 0.02f;
  float cr = 0.7885f * cosf(t);
  float ci = 0.7885f * sinf(t);

  float scale = 3.5f / (float)w;
  float half_w = (float)w * 0.5f;
  float half_h = (float)h * 0.5f;

  int tiles_x = w / TILE_SIZE;
  int tiles_y = h / TILE_SIZE;

  /*
  un chrono x cada par de for

  for(y+=8)
    for(x++)
      filas

  for(x+=8)
    for(y++)
      columnas

  for(y+=8)
    for(x+=8)
      if()
      else

  */

  for (int ty = 0; ty < tiles_y; ty++) {
    int py0 = ty * TILE_SIZE;

    for (int tx = 0; tx < tiles_x; tx++) {
      int px0 = tx * TILE_SIZE;

      for (int y = py0; y < py0 + TILE_SIZE; y++) {   // aca calcula lo que no es igual
        float zi = ((float)y - half_h) * scale;
        unsigned int* row = pixels + y * stride;
        for (int x = px0; x < px0 + TILE_SIZE; x++) {
          float zr = ((float)x - half_w) * scale;
          row[x] = palette[JuliaIter(zr, zi, cr, ci)];
        }
      }
    }
  }
}

// ---------------------------------------------------------------------------

static void BuildPalette(void)
{
  for (int i = 0; i < MAX_ITER; i++) {
    unsigned char r = 0.0f;
    unsigned char g = 200.0f;
    unsigned char b = 120.0f;

    palette[i] = 0;
    palette[MAX_ITER] = ((unsigned int)r << 16) | ((unsigned int)g << 8) | (unsigned int)b;
  }

}

// ---------------------------------------------------------------------------

int main(int argc, char** argv)
{
  int end = 0;
  SDL_Surface* g_SDLSrf;
  int req_w = 1024;
  int req_h = 768;

  if (argc < 2) { fprintf(stderr, "I need the cpu speed in Mhz!\n"); exit(0); }
  cpu_mhz = atoi(argv[1]);
  assert(cpu_mhz > 0);
  fprintf(stdout, "Cycle times for a %d Mhz cpu\n", cpu_mhz);

  // Init SDL and screen
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Can't Initialise SDL: %s\n", SDL_GetError());
    exit(1);
  }
  if (0 == SDL_SetVideoMode(req_w, req_h, 32, SDL_HWSURFACE | SDL_DOUBLEBUF)) {
    printf("Couldn't set %dx%dx32 video mode: %s\n", req_w, req_h, SDL_GetError());
    return 0;
  }

  BuildPalette();

  // Main loop
  g_SDLSrf = SDL_GetVideoSurface();
  assert(g_SDLSrf != NULL);
  while (!end) {

    SDL_Event event;

    // Your gfx effect goes here

    ChronoWatchReset();

    SDL_LockSurface(g_SDLSrf);

    JuliaSet((unsigned int*)g_SDLSrf->pixels,
      g_SDLSrf->w,
      g_SDLSrf->h,
      g_SDLSrf->pitch >> 2,
      dump);

    SDL_UnlockSurface(g_SDLSrf);

    int n_tiles = (g_SDLSrf->w / TILE_SIZE) * (g_SDLSrf->h / TILE_SIZE);
    //ChronoShow("Julia paso4", n_tiles); 
    ChronoShow("Julia 2", g_SDLSrf->w * g_SDLSrf->h);
    SDL_Flip(g_SDLSrf);

    //FramerateLimit (60);

    dump++;

    // Recoger eventos de la ventana
    while (SDL_PollEvent(&event))
    {
      switch (event.type) {
      case SDL_QUIT: end = 1; break;
      default:       break;
      }
    }
  }

  return 1;
}
