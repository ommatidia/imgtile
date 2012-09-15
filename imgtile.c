#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <png.h>

#include "dirutil.h"
#include "tiffio.h"
#include "math.h"

typedef unsigned char channel;
typedef unsigned char uint8_t;
typedef unsigned char bool;

#define TIFFTAG_IMAGEWIDTH 256
#define TIFFTAG_IMAGEHEIGHT 257

#define true 1

#define LEN(X) (sizeof(X)/sizeof(*X))

#define BYTES_PER_CHANNEL sizeof(channel)
#define CHANNELS_PER_PIXEL 3
#define BLACK 0

#define PATH_BUFFER 2048

TIFF* fetchStripTiff(const char*);

static int save_raster_to_png(const uint32*, uint32, uint32, const char*, bool);
static int save_crop_raster_to_png(const uint32*, uint32, uint32, uint32, uint32, uint32, uint32, const char*, bool);

void strip_meta(const char*);

void tile(uint32*, uint8, uint8, uint8,  uint32, uint32, bool);

uint32 *resize(const uint32*, uint32, uint32, uint32, uint32);
void setComponents(const uint32*, uint32, uint8*);

void slice_and_dice(TIFF*);
void slice_and_dice2(TIFF*, unsigned int, unsigned int);

int dir_mask = S_IRWXU | S_IRWXG | S_IRWXO;
char outdir[PATH_BUFFER];

int main(int argc, const char* argv[]) {
  if(argc == 1) {
    printf("%s", "Please supply an image file to tile\n");
    return 1;
  }

  char filename[PATH_BUFFER];
  strcpy(filename, argv[1]);

  if(argc == 3) {
    strcpy(outdir, argv[2]);
    mkpath(outdir, dir_mask);
  } else {
    strcpy(outdir, ".");
  }

  TIFF* image = fetchStripTiff(filename);
  if(image == NULL) {
    printf("Could not read tiff image, %s, exiting with -1\n", filename);
    return -1;
  } else {
    printf("Opened strip copy of tiff image, %s\n", filename);
  }
  
  uint32 w, h;
  uint16 samples;
  size_t npixels;
  uint32* raster;

  TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &w);
  TIFFGetField(image, TIFFTAG_IMAGEHEIGHT, &h);
  TIFFGetField(image, 277, &samples);
  npixels = w * h;
  raster = (uint32*) _TIFFmalloc(npixels * sizeof(uint32));
  if(raster != NULL) {
    if(TIFFReadRGBAImage(image, w, h, raster, 0)) {
      //TODO: slice and dice
      printf("loaded data into memory\n");
      bool grayscale = 0;
      if(samples == 1) {
	grayscale = true;
      }
      char output[PATH_BUFFER];
      sprintf(output, "%s/%s", outdir, "test.png");


      uint32 *newraster = resize(raster, w, h, 1000, 800);

      //save_crop_raster_to_png(raster, 3000, 2000, 256, 256, w, h, output, grayscale);
      save_raster_to_png(newraster, 1000, 800, output, grayscale);
      _TIFFfree(newraster);
    }
    _TIFFfree(raster);
  }
  TIFFClose(image);

  /*start = clock();
  slice_and_dice(image);
  end = clock();
  printf("total tile time %.4f seconds\n", ((double)(end-start))/CLOCKS_PER_SEC);*/

  return 0;
}

TIFF* fetchStripTiff(const char *filepath) {
  char temp[PATH_BUFFER];
  strcpy(temp, "/tmp/copy_tiff-XXXXXXXXXXXXXX");
  int handle = mkstemp(temp);
  if(handle == -1) {
    printf("handle: %d, temppath: %s", handle, temp);
    return NULL;
  }
  close(handle);

  char command[PATH_BUFFER*2];
  sprintf(command, "tiffcp -c lzw:2 -r -1 -p contig -s %s %s", filepath, temp);
  int errcode = system(command);
  printf("%s\n", command);

  strip_meta(temp);

  TIFF* tiff = TIFFOpen(temp, "r");
  unlink(temp);

  return tiff;
}

const int codes[6] = {
  269, //document name
  270, //
  271, //
  272, //
  285, //
  315  //artist
};
/**
 * Remove all ASCII metadata. Some images either have too much and cause
 * overflow exceptions when trying to load them, or have invalid characters
 * that aren't escaped propertly.
 */
void strip_meta(const char* path) {
  char command[PATH_BUFFER];
  int i;
  for(i = 0; i < LEN(codes); i++) {
    int code = codes[i];
    sprintf(command, "tiffset -s %i \'\' %s", code, path);
    system(command);
  }
}

void tile(uint32 *raster, uint8 native_zoom, uint8 tw, uint8 th, uint32 w, uint32 h, bool grayscale) {
  uint8 zoom;
  uint32 tiles, max_h, max_w, scale_w, scale_h, raster_sw, raster_sh, offset_x, offset_y;
  double ratio_w, ratio_h;
  tiles = pow(2, native_zoom);
  max_w = tiles * tw;
  max_h = tiles * th;
  ratio_w = (((float)w)/max_w);
  ratio_h = (((float)h)/max_h);
  
  char path[PATH_BUFFER], save[PATH_BUFFER];
  for(zoom = 0; zoom < native_zoom; zoom++) {
    sprintf(path, "%s/level_%i", outdir, zoom);
    mkdir(path, dir_mask);

    tiles = pow(2, zoom);
    scale_w = tiles * tw;
    scale_h = tiles * th;
    
    raster_sw = (int)(scale_w * ratio_w);
    raster_sh = (int)(scale_h * ratio_h);

    offset_x = (scale_w - raster_sw) / 2;
    offset_y = (scale_h - raster_sh) / 2;
    //allocate new resized raster

    
    //_TIFFfree(scale_raster);
  }
}

uint32* resize(const uint32* raster, uint32 origW, uint32 origH, uint32 newW, uint32 newH) {
  double scale_w = (double)origW / (double)newW,
    scale_h = (double)origH / (double)newH,
    ci, cj, xoff, yoff;
  uint32 *scale_raster = _TIFFmalloc(newW * newH * sizeof(uint32));
  uint32 x1, y1, x2, y2;
  uint8 c1_rgb[3], c2_rgb[3], c3_rgb[3], c4_rgb[3];
  uint32 index1, index2;
  uint32 c, i, j;
  uint32 r, g, b;


  index1 = 0;
  cj = 0;
  for(j=0; j<newH; j++) {
    y1 = cj;
    y2 = y1+1;
    yoff = cj - y1;
    index1 = y1*origW;
    index2 = y2*origW;

    ci = 0;
    for(i=0; i<newW; i++) {
      x1 = ci;
      x2 = x1+1;
      
      setComponents(raster, index1 + x1, c1_rgb);
      setComponents(raster, index1 + x2, c2_rgb);
      setComponents(raster, index2 + x1, c3_rgb);
      setComponents(raster, index2 + x2, c4_rgb);
      
      xoff = ci-x1;
      r = (c1_rgb[0]*(1-xoff) + (c2_rgb[0]*xoff))*(1-yoff) + (c3_rgb[0]*(1-xoff) + (c4_rgb[0]*xoff))*yoff;
      g = (c1_rgb[1]*(1-xoff) + (c2_rgb[1]*xoff))*(1-yoff) + (c3_rgb[1]*(1-xoff) + (c4_rgb[1]*xoff))*yoff;
      b = (c1_rgb[2]*(1-xoff) + (c2_rgb[2]*xoff))*(1-yoff) + (c3_rgb[2]*(1-xoff) + (c4_rgb[2]*xoff))*yoff;
      
      *(scale_raster + i + j*newW) = r << 16 | g << 8 | b;
      ci += scale_w;
    }
    cj += scale_h;
  }
  
  //memset(scale_raster, 0, scale_w * scale_h * sizeof(uint32));  
  return scale_raster;
}

//not safe (offset value)
void setComponents(const uint32 *raster, uint32 offset, uint8 *components) {
  uint32 *px = raster + offset;
  components[0] = (uint8)((*px) & 0x00ff0000) >> 16;
  components[1] = (uint8)((*px) & 0x0000ff00) >> 8;
  components[2] = (uint8)((*px) & 0x000000ff);
}

void slice_and_dice2(TIFF *img, unsigned int tw, unsigned int th) {
  /*
  for(int lvl = 0; lvl < nz; lvl++) {
    int tiles = pow(2, lvl);

    //resize for zoom level
    C_IMG canvas(scale_w, scale_h, 
        BYTES_PER_CHANNEL, CHANNELS_PER_PIXEL, BLACK);
    canvas.draw_image(offset_x, offset_y, img->get_resize(sw, sh));

    for(int x = 0; x < tiles; x++) {
      for(int y = 0; y < tiles; y++) {
	sprintf(save_file, "%s/%i_%i.png", path, x, y);
	int xpx = x*tw;
	int ypx = y*th;
	C_IMG tile = canvas.get_crop(xpx, ypx, xpx+tw, ypx+th);
	tile.save_png(save_file);
      }
    }
    }*/
}

static int save_raster_to_png(const uint32 *raster, uint32 w, uint32 h, const char *path, bool grayscale) {
  return save_crop_raster_to_png(raster, 0, 0, w, h, w, h, path, grayscale);
}

static int save_crop_raster_to_png(const uint32 *raster, uint32 sx, uint32 sy, uint32 sw, uint32 sh, 
				   uint32 w, uint32 h, const char *path, bool grayscale) {
  int x, y;
  FILE * fp;
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  
  png_byte ** row_pointers = NULL;
  int status = -1;
  
  int pixel_size = 3;
  int depth = 8;
  
  fp = fopen (path, "wb");
  if (! fp) {
    goto fopen_failed;
  }
  
  png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) {
    goto png_create_write_struct_failed;
  }
  
  info_ptr = png_create_info_struct (png_ptr);
  if (info_ptr == NULL) {
    goto png_create_info_struct_failed;
  }
  
  /* Set up error handling. */
  
  if (setjmp (png_jmpbuf (png_ptr))) {
    goto png_failure;
  }
  
  /* Set image attributes. */
  
  png_set_IHDR (png_ptr,
		info_ptr,
		sw,
		sh,
		depth,
		PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
  
  /* Initialize rows of PNG. */
  
  row_pointers = png_malloc (png_ptr, sh * sizeof (png_byte *));
  for (y = sy; y < sh+sy; ++y) {
    png_byte *row = png_malloc (png_ptr, sizeof (uint8_t) * sw * pixel_size);
    row_pointers[y-sy] = row;
    for (x = sx; x < sw+sx; ++x) {
      uint32* pixel = raster+(y*w+x);
      if(grayscale) {
	uint8_t temp = (uint8_t)((*pixel) & 0xff);
	*row++ = temp;
	*row++ = temp;
	*row++ = temp;
      } else {
	*row++ = (uint8_t)((*pixel) & 0x000000ff) >> 0;
	*row++ = (uint8_t)((*pixel) & 0x0000ff00) >> 8;
	*row++ = (uint8_t)((*pixel) & 0x00ff0000) >> 16;
      }
    }
  }
  
  /* Write the image data to "fp". */
  
  png_init_io (png_ptr, fp);
  png_set_rows (png_ptr, info_ptr, row_pointers);
  png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
  
  /* The routine has successfully written the file, so we set
     "status" to a value which indicates success. */
  
  status = 0;
  
  for (y = 0; y < sh; y++) {
    png_free (png_ptr, row_pointers[y]);
  }
  png_free (png_ptr, row_pointers);
  
 png_failure:
 png_create_info_struct_failed:
  png_destroy_write_struct (&png_ptr, &info_ptr);
 png_create_write_struct_failed:
  fclose (fp);
 fopen_failed:
  return status;

}


