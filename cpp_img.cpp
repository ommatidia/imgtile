#include "dirutil.h"

#include "math.h"
#include <time.h>
#include <string>
#include <iostream>

#include "CImg.h"
using namespace cimg_library;

typedef unsigned char channel;

#define BYTES_PER_CHANNEL sizeof(channel)
#define CHANNELS_PER_PIXEL 3
#define BLACK 0

#define PATH_BUFFER 2048

void slice_and_dice(CImg<channel>*);
void slice_and_dice(CImg<channel>*, unsigned int, unsigned int);
int getNativeZoomLevel(CImg<channel>*);
int getNativeZoomLevel(CImg<channel>*, unsigned int, unsigned int);

int dir_mask = S_IRWXU | S_IRWXG | S_IRWXO;
char outdir[PATH_BUFFER];

int main(int argc, const char* argv[]) {
  if(argc == 1) {
    std::cout << "Please supply an image file to tile\n";
    return 0;
  }

  char filename[PATH_BUFFER];
  strcpy(filename, argv[1]);

  if(argc == 3) {
    strcpy(outdir, argv[2]);
    mkpath(outdir, dir_mask);
  } else {
    strcpy(outdir, ".");
  }

  long int start = clock();
  CImg<channel> *image = new CImg<channel>(filename);
  long int end = clock();
  printf("total load time %.4f seconds\n", ((double)(end-start))/CLOCKS_PER_SEC);

  start = clock();
  slice_and_dice(image);
  end = clock();
  printf("total tile time %.4f seconds\n", ((double)(end-start))/CLOCKS_PER_SEC);

  delete image;
  return 0;
}

void slice_and_dice(CImg<channel>* img) {
  int w = img->width();
  int h = img->height();
  unsigned int nz = getNativeZoomLevel(img);
  int tiles = pow(2, nz);
  int tw = w/tiles;
  int th = h/tiles;
  slice_and_dice(img, tw, th);
}

void slice_and_dice(CImg<channel>* img, unsigned int tw, unsigned int th) {
  printf("Image(%d, %d)\n", img->width(), img->height());

  unsigned int nz = getNativeZoomLevel(img);
  printf("Native Zoom Level: %d\n", nz);

  int max_w = pow(2, nz) * tw, max_h = pow(2, nz) * th;
  float rw = ((float)img->width()/max_w), rh = ((float)img->height()/max_h);

  char path[PATH_BUFFER];
  char save_file[PATH_BUFFER];
  for(int lvl = 0; lvl < nz; lvl++) {
    sprintf(path, "%s/level_%i", outdir, lvl);
    mkdir(path, dir_mask);

    int tiles = pow(2, lvl);
    int scale_w = tiles * tw;
    int scale_h = tiles * th;
    //todo: ratio
    int sw = int(scale_w*rw);
    int sh = int(scale_h*rh);

    int offset_x = (scale_w - sw) / 2;
    int offset_y = (scale_h - sh) / 2;

    //resize for zoom level
    CImg<channel> canvas(scale_w, scale_h, 
        BYTES_PER_CHANNEL, CHANNELS_PER_PIXEL, BLACK);
    canvas.draw_image(offset_x, offset_y, img->get_resize(sw, sh));

    for(int x = 0; x < tiles; x++) {
      for(int y = 0; y < tiles; y++) {
	sprintf(save_file, "%s/%i_%i.png", path, x, y);
	int xpx = x*tw;
	int ypx = y*th;
	CImg<channel> tile = canvas.get_crop(xpx, ypx, xpx+tw, ypx+th);
	tile.save_png(save_file);
      }
    }
  }
}

int getNativeZoomLevel(CImg<channel>* img) {
  return getNativeZoomLevel(img, 256, 256);
}

int getNativeZoomLevel(CImg<channel>* img, unsigned int tile_w, unsigned int tile_h) {
  int w = img->width(), h = img->height();

  float base2 = log(2.0f);
  int horz = int(ceil(log(w/tile_w)/base2));
  int vert = int(ceil(log(h/tile_h)/base2));
  return cimg_library::cimg::max(horz, vert);
}
