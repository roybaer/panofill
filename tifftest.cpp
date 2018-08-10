/*
 * panofill - fills transparent areas in panorama images.
 * Copyright (C) 2010  Benedikt Freisen
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <algorithm>
#include <iostream>
#include "tiffio.h"

using namespace std;

struct t_pixel
{
    unsigned char r, g, b ,a;
};

class image
{
public:
    image(int width, int height, t_pixel* data, bool manageData = false);
    image(char* s);
    ~image();
    int width;
    int height;
    bool manageData;
    t_pixel* data;
    void saveToTIFF(char* s);
    image* halfSize();
    image* blurred();
    image* doubleSize();
    void alphaBlend(image* img);
    void correctAlpha();
    bool onlyTransparentPixels();
    bool noTransparentPixels();
};


image::image(int width, int height, t_pixel* data, bool manageData)
{
    image::width = width;
    image::height = height;
    image::data = data;
    image::manageData = manageData;
}

image::image(char* s)
{
  TIFF* tif = TIFFOpen(s, "r");
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
  data = new t_pixel[width * height];
  TIFFReadRGBAImage(tif, width, height, (uint32*) data, 0);
  TIFFClose(tif);
  manageData = true;
}

image::~image()
{
  if (manageData)
    delete data;
}

void image::saveToTIFF(char* s)
{
  TIFF* tif2 = TIFFOpen(s, "w");
  TIFFSetField(tif2, TIFFTAG_IMAGEWIDTH, width);
  TIFFSetField(tif2, TIFFTAG_IMAGELENGTH, height);
  TIFFSetField(tif2, TIFFTAG_SAMPLESPERPIXEL, 4);
  TIFFSetField(tif2, TIFFTAG_BITSPERSAMPLE, 8);
  TIFFSetField(tif2, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(tif2, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(tif2, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  TIFFSetField(tif2, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif2, width * 4));
  for (int y = 0; y < height; y++)
    TIFFWriteScanline(tif2, &data[(height - y - 1) * width], y, 0);
  TIFFClose(tif2);
}

image* image::halfSize()
{
  int w = (width + 1) / 2;
  int h = (height + 1) / 2;
  t_pixel* temp = new t_pixel[w * h];
  for (int y = 0; y < h; y++)
  {
    for (int x = 0; x < w; x++)
    {
      int z = 0;
      int r = 0;
      int g = 0;
      int a = 0;
      int b = 0;
      if (data[(y * 2 * width) + (x * 2)].a != 0)
      {
	r = data[( y * 2 * width) + (x * 2)].r * data[(y * 2 * width) + (x * 2)].a;
	g = data[( y * 2 * width) + (x * 2)].g * data[(y * 2 * width) + (x * 2)].a;
	b = data[( y * 2 * width) + (x * 2)].b * data[(y * 2 * width) + (x * 2)].a;
	a = data[( y * 2 * width) + (x * 2)].a;
	z++;
      }
      if ((x * 2 + 1 <= width) && (data[(y * 2 * width) + (x * 2 + 1)].a != 0))
      {
	r += data[(y * 2 * width) + (x * 2 + 1)].r * data[(y * 2 * width) + (x * 2 + 1)].a;
	g += data[(y * 2 * width) + (x * 2 + 1)].g * data[(y * 2 * width) + (x * 2 + 1)].a;
	b += data[(y * 2 * width) + (x * 2 + 1)].b * data[(y * 2 * width) + (x * 2 + 1)].a;
	a += data[(y * 2 * width) + (x * 2 + 1)].a;
	z++;
      }
      if (y * 2 + 1 <= width)
      {
	if (data[(y * 2 * width) + (x * 2)].a != 0)
	{
	  r += data[(y * 2 * width + 1) + (x * 2)].r * data[(y * 2 * width + 1) + (x * 2)].a;
	  g += data[(y * 2 * width + 1) + (x * 2)].g * data[(y * 2 * width + 1) + (x * 2)].a;
	  b += data[(y * 2 * width + 1) + (x * 2)].b * data[(y * 2 * width + 1) + (x * 2)].a;
	  a += data[(y * 2 * width + 1) + (x * 2)].a;
	  z++;
	}
	if ((x * 2 + 1 <= width) && (data[(y * 2 * width) + (x * 2 + 1)].a != 0))
	{
	  r += data[(y * 2 * width + 1) + (x * 2 + 1)].r * data[(y * 2 * width + 1) + (x * 2 + 1)].a;
	  g += data[(y * 2 * width + 1) + (x * 2 + 1)].g * data[(y * 2 * width + 1) + (x * 2 + 1)].a;
	  b += data[(y * 2 * width + 1) + (x * 2 + 1)].b * data[(y * 2 * width + 1) + (x * 2 + 1)].a;
	  a += data[(y * 2 * width + 1) + (x * 2 + 1)].a;
	  z++;
	}
      }
      if (a != 0)
      {
	r /= a;
	g /= a;
	b /= a;
	a /= z;
      }
      temp[y * w + x].r = r;
      temp[y * w + x].g = g;
      temp[y * w + x].b = b;
      temp[y * w + x].a = a;
    }
  }
  return new image(w, h, temp, true);
}

image* image::blurred()
{
  t_pixel* temp = new t_pixel[width * height];
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      int a = 0;
      int b = 0;
      int g = 0;
      int r = 0;
      int z = 0;
      for (int v = y - 2; v <= y + 2; v++)
      {
	if ((v >= 0) && (v < height))
	  for (int u = x - 2; u <= x + 2; u++)
	    if ((u >= 0) && (u < width))
	    {
	      a += data[v * width + u].a;
	      b += data[v * width + u].b * data[v * width + u].a;
	      g += data[v * width + u].g * data[v * width + u].a;
	      r += data[v * width + u].r * data[v * width + u].a;
	      z++;
	    }
      }
      temp[y * width + x].a = a / z;
      if (a != 0)
      {
	temp[y * width + x].b = b / a;
	temp[y * width + x].g = g / a;
	temp[y * width + x].r = r / a;
      }
    }
  }
  return new image(width, height, temp, true);
}

image* image::doubleSize()
{
    int w = width * 2;
    int h = height * 2;
    t_pixel* temp = new t_pixel[w * h];
    for ( int y = 0; y < h; y++ )
    {
        for ( int x = 0; x < w; x++ )
        {
            temp[y * w + x].r = data[ ( y / 2 ) * width + ( x / 2 ) ].r;
            temp[y * w + x].g = data[ ( y / 2 ) * width + ( x / 2 ) ].g;
            temp[y * w + x].b = data[ ( y / 2 ) * width + ( x / 2 ) ].b;
            temp[y * w + x].a = data[ ( y / 2 ) * width + ( x / 2 ) ].a;
        }
    }
    return new image(w, h, temp, true);
}

/*image* image::doubleSize()
{
    int w = width * 2;
    int h = height * 2;
    t_pixel* temp = new t_pixel[w * h];
    for ( int y = 0; y < height; y++ )
    {
        for ( int x = 0; x < width; x++ )
        {
            temp[(y * 2) * w + x * 2].r = (data[y * width + x].r * 9 + data[max(0, y - 1) * width + x].r * 3 + data[max(0, y - 1) * width + max(0, x - 1)].r * 1 + data[y * width + max(0, x - 1)].r * 3) / 16;
            temp[(y * 2) * w + x * 2].g = (data[y * width + x].g * 9 + data[max(0, y - 1) * width + x].g * 3 + data[max(0, y - 1) * width + max(0, x - 1)].g * 1 + data[y * width + max(0, x - 1)].g * 3) / 16;
            temp[(y * 2) * w + x * 2].b = (data[y * width + x].b * 9 + data[max(0, y - 1) * width + x].b * 3 + data[max(0, y - 1) * width + max(0, x - 1)].b * 1 + data[y * width + max(0, x - 1)].b * 3) / 16;
            temp[(y * 2) * w + x * 2].a = (data[y * width + x].a * 9 + data[max(0, y - 1) * width + x].a * 3 + data[max(0, y - 1) * width + max(0, x - 1)].a * 1 + data[y * width + max(0, x - 1)].a * 3) / 16;

	    temp[(y * 2) * w + x  *2 + 1].r = (data[y * width + x].r * 9 + data[max(0, y - 1) * width + x].r * 3 + data[max(0, y - 1) * width + min(w - 1, x + 1)].r * 1 + data[y * width + min(w - 1, x + 1)].r * 3) / 16;
	    temp[(y * 2) * w + x  *2 + 1].g = (data[y * width + x].g * 9 + data[max(0, y - 1) * width + x].g * 3 + data[max(0, y - 1) * width + min(w - 1, x + 1)].g * 1 + data[y * width + min(w - 1, x + 1)].g * 3) / 16;
	    temp[(y * 2) * w + x  *2 + 1].b = (data[y * width + x].b * 9 + data[max(0, y - 1) * width + x].b * 3 + data[max(0, y - 1) * width + min(w - 1, x + 1)].b * 1 + data[y * width + min(w - 1, x + 1)].b * 3) / 16;
	    temp[(y * 2) * w + x  *2 + 1].a = (data[y * width + x].a * 9 + data[max(0, y - 1) * width + x].a * 3 + data[max(0, y - 1) * width + min(w - 1, x + 1)].a * 1 + data[y * width + min(w - 1, x + 1)].a * 3) / 16;

	    temp[(y * 2 + 1) * w + x * 2].r = (data[y * width + x].r * 9 + data[min(h - 1, y + 1) * width + x].r * 3 + data[min(h - 1, y + 1) * width + max(0, x - 1)].r * 1 + data[y * width + max(0, x - 1)].r * 3) / 16;
            temp[(y * 2 + 1) * w + x * 2].g = (data[y * width + x].g * 9 + data[min(h - 1, y + 1) * width + x].g * 3 + data[min(h - 1, y + 1) * width + max(0, x - 1)].g * 1 + data[y * width + max(0, x - 1)].g * 3) / 16;
            temp[(y * 2 + 1) * w + x * 2].b = (data[y * width + x].b * 9 + data[min(h - 1, y + 1) * width + x].b * 3 + data[min(h - 1, y + 1) * width + max(0, x - 1)].b * 1 + data[y * width + max(0, x - 1)].b * 3) / 16;
            temp[(y * 2 + 1) * w + x * 2].a = (data[y * width + x].a * 9 + data[min(h - 1, y + 1) * width + x].a * 3 + data[min(h - 1, y + 1) * width + max(0, x - 1)].a * 1 + data[y * width + max(0, x - 1)].a * 3) / 16;

	    temp[(y * 2 + 1) * w + x  *2 + 1].r = (data[y * width + x].r * 9 + data[min(h - 1, y + 1) * width + x].r * 3 + data[min(h - 1, y + 1) * width + min(w - 1, x + 1)].r * 1 + data[y * width + min(w - 1, x + 1)].r * 3) / 16;
	    temp[(y * 2 + 1) * w + x  *2 + 1].g = (data[y * width + x].g * 9 + data[min(h - 1, y + 1) * width + x].g * 3 + data[min(h - 1, y + 1) * width + min(w - 1, x + 1)].g * 1 + data[y * width + min(w - 1, x + 1)].g * 3) / 16;
	    temp[(y * 2 + 1) * w + x  *2 + 1].b = (data[y * width + x].b * 9 + data[min(h - 1, y + 1) * width + x].b * 3 + data[min(h - 1, y + 1) * width + min(w - 1, x + 1)].b * 1 + data[y * width + min(w - 1, x + 1)].b * 3) / 16;
	    temp[(y * 2 + 1) * w + x  *2 + 1].a = (data[y * width + x].a * 9 + data[min(h - 1, y + 1) * width + x].a * 3 + data[min(h - 1, y + 1) * width + min(w - 1, x + 1)].a * 1 + data[y * width + min(w - 1, x + 1)].a * 3) / 16;
        }
    }
    return new image(w, h, temp, true);
}*/

void image::alphaBlend ( image* img )
{
    for ( int y = 0; y < height; y++ )
    {
        for ( int x = 0; x < width; x++ )
        {
            int alpha = data[y * width + x].a;
            data[y * width + x].r = ( data[y * width + x].r * alpha + img->data[y * img->width + x].r * ( 255 - alpha ) ) / 255;
            data[y * width + x].g = ( data[y * width + x].g * alpha + img->data[y * img->width + x].g * ( 255 - alpha ) ) / 255;
            data[y * width + x].b = ( data[y * width + x].b * alpha + img->data[y * img->width + x].b * ( 255 - alpha ) ) / 255;
            data[y * width + x].a = alpha + ( img->data[y * img->width + x].a * ( 255 - alpha ) ) / 255;
        }
    }
}

void image::correctAlpha()
{
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      data[y * width + x].a = min(255, (int) data[y * width + x].a * 5);
    }
  }
}

bool image::onlyTransparentPixels()
{
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      if (data[y * width + x].a != 0)
	return false;
    }
  }
  return true;
}

bool image::noTransparentPixels()
{
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      if (data[y * width + x].a != 255)
	return false;
    }
  }
  return true;
}

//------------------------------

bool complete(image* img, int n = 0)
{
  clog << " + ";
  if (img->noTransparentPixels())
  {clog << " noTransparentPixels - "; return true;}
  image* tempImg = img->blurred();
  tempImg->correctAlpha();
  image* lowRes = tempImg->halfSize();
  tempImg->~image();
  //if (n == 7) lowRes->saveToTIFF("/home/benedikt/loch3.tif");
  complete(lowRes, n + 1);
  image* highRes = lowRes->doubleSize();
  //if (n == 7) highRes->saveToTIFF("/home/benedikt/loch3.tif");
  img->alphaBlend(highRes);
  //if (n == 7) img->saveToTIFF("/home/benedikt/loch3.tif");
  lowRes->~image();
  highRes->~image();
  clog << " - ";
  return true;
}

//------------------------------

int main()
{
  /*TIFF* tif = TIFFOpen("/home/benedikt/loch.tif", "r");
  if (!tif)
    return 1;
  uint32 w, h;
  size_t npixels;
  uint32* raster;
  
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
  npixels = w * h;
  raster = (uint32*) _TIFFmalloc(npixels * sizeof(uint32));
  TIFFReadRGBAImage(tif, w, h, raster, 0);*/
  
  image* img = new image("/home/benedikt/schneepano/test4.tif");//new image((int) w, (int) h, (t_pixel*) raster, false);
  //image* img2 = img->blurred();
  //img2->correctAlpha();
  complete(img);
  img->saveToTIFF("/home/benedikt/test4.tif");
  
  /*TIFF* tif2 = TIFFOpen("/home/benedikt/loch2.tif", "w");
  TIFFSetField(tif2, TIFFTAG_IMAGEWIDTH, w);
  TIFFSetField(tif2, TIFFTAG_IMAGELENGTH, h);
  TIFFSetField(tif2, TIFFTAG_SAMPLESPERPIXEL, 4);
  TIFFSetField(tif2, TIFFTAG_BITSPERSAMPLE, 8);
  TIFFSetField(tif2, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(tif2, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(tif2, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  TIFFSetField(tif2, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif2, w * 4));
  for (int y = 0; y < h; y++)
    TIFFWriteScanline(tif2, &(img->data[(h - y - 1) * w]), y, 0);
  TIFFClose(tif2);
  
  _TIFFfree(raster);
  TIFFClose(tif);*/
}

