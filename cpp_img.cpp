#include "dirutil.h"

#include "math.h"
#include <time.h>
#include <string>
#include <iostream>
//#include <sys/stat.h>

#include "CImg.h"
using namespace cimg_library;

void slice_and_dice(CImg<unsigned char>*);
void slice_and_dice(CImg<unsigned char>*, unsigned int, unsigned int);
int getNativeZoomLevel(CImg<unsigned char>*);
int getNativeZoomLevel(CImg<unsigned char>*, unsigned int, unsigned int);

int dir_mask = S_IRWXU | S_IRWXG | S_IRWXO;
char outdir[256];

int main(int argc, const char* argv[]) {
  if(argc == 1) {
    std::cout << "Please supply an image file to tile\n";
    return 0;
  }

  char filename[256];
  strcpy(filename, argv[1]);

  if(argc == 3) {
    strcpy(outdir, argv[2]);
    mkpath(outdir, dir_mask);
  } else {
    strcpy(outdir, ".");
  }

  CImg<unsigned char> image(filename);

  long int start = clock();
  slice_and_dice(&image);
  long int end = clock();

  char output[128];
  sprintf(output, "total time %.4f seconds", ((double)(end-start))/CLOCKS_PER_SEC);
  std::cout << output << "\n";

  return 0;
}

void slice_and_dice(CImg<unsigned char>* img) {
  int w = img->width();
  int h = img->height();
  unsigned int nz = getNativeZoomLevel(img);
  int tiles = pow(2, nz);
  int tw = w/tiles;
  int th = h/tiles;
  slice_and_dice(img, tw, th);
}

void slice_and_dice(CImg<unsigned char>* img, unsigned int tw, unsigned int th) {
  std::cout << "Image (" << img->width() << ", " << img->height() << ")\n";
  std::cout << "Native Zoom Level given resolution: ";
  unsigned int nz = getNativeZoomLevel(img);
  std::cout << nz << "\n";  

  int max_w = pow(2, nz) * tw, max_h = pow(2, nz) * th;
  float rw = ((float)img->width()/max_w), rh = ((float)img->height()/max_h);

  for(int lvl = 0; lvl < nz; lvl++) {
    char path[512];
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

    CImg<unsigned char> canvas(scale_w, scale_h, 1, 3, 0);
    canvas.draw_image(offset_x, offset_y, img->get_resize(sw, sh));

    for(int x = 0; x < tiles; x++) {
      for(int y = 0; y < tiles; y++) {
	char save_file[1024];
	sprintf(save_file, "%s/%i_%i.png", path, x, y);
	int xpx = x*tw;
	int ypx = y*th;
	CImg<unsigned char> tile = canvas.get_crop(xpx, ypx, xpx+tw, ypx+th);
	tile.save_png(save_file);
	delete &tile;
      }
    }
  }

  //int px = 6000;
  //CImg<unsigned char> tile = img->get_crop(px, px, px+255, px+255, false);

  //tile.save_png("tile.png"); 
}

int getNativeZoomLevel(CImg<unsigned char>* img) {
  return getNativeZoomLevel(img, 256, 256);
}

int getNativeZoomLevel(CImg<unsigned char>* img, unsigned int tile_w, unsigned int tile_h) {
  int w = img->width(), h = img->height();

  float base2 = log(2.0f);
  int horz = int(ceil(log(w/tile_w)/base2));
  int vert = int(ceil(log(h/tile_h)/base2));
  return cimg_library::cimg::max(horz, vert);
}
