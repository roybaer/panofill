#include <algorithm>
#include <iostream>
#include <iomanip>
#include <math.h>
#include "tiffio.h"
#define PI 3.14159265

#include <locale>
#include <unistd.h>

using namespace std;

int verbosity = 0;

struct t_pixel
{
    unsigned char r, g, b ,a;
};

struct t_fpixel
{
    float r, g, b ,a;
};

class image
{
public:
    image(int width, int height, t_fpixel* data, bool manageData = false);
    image(char* s);
    ~image();
    int width;
    int height;
    bool manageData;
    t_fpixel* data;
    void saveToTIFF(char* s);
    image* blurredHalfSize();
    image* doubleSizeN();
    image* doubleSizeL();
    void alphaBlend(image* img);
    void correctAlpha();
    bool onlyTransparentPixels();
    bool noTransparentPixels();
};


image::image(int width, int height, t_fpixel* data, bool manageData)
{
    image::width = width;
    image::height = height;
    image::data = data;
    image::manageData = manageData;
}

image::image(char* s)
{
    data = NULL;
    TIFF* tif = TIFFOpen(s, "r");
    if (tif)
    {
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        data = new t_fpixel[width * height];
        t_pixel* temp = new t_pixel[width * height];
        TIFFReadRGBAImage(tif, width, height, (uint32*) temp, 0);
        TIFFClose(tif);
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                data[y * width + x].r = (float) temp[y * width + x].r / 255;
                data[y * width + x].g = (float) temp[y * width + x].g / 255;
                data[y * width + x].b = (float) temp[y * width + x].b / 255;
                data[y * width + x].a = (float) temp[y * width + x].a / 255;
            }
        }
        delete temp;
        manageData = true;
    }
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
    t_pixel* temp = new t_pixel[width];
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            temp[x].r = data[(height - y - 1) * width + x].r * 255;
            temp[x].g = data[(height - y - 1) * width + x].g * 255;
            temp[x].b = data[(height - y - 1) * width + x].b * 255;
            temp[x].a = data[(height - y - 1) * width + x].a * 255;
        }
        TIFFWriteScanline(tif2, temp, y, 0);
    }
    TIFFClose(tif2);
    delete temp;
}

image* image::blurredHalfSize()
{
    int w = (width + 1) / 2;
    int h = (height + 1) / 2;
    t_fpixel* temp = new t_fpixel[w * h];
    for (int y = 0; y < h; y++)
    {
        float fw = 4.0 / (cos((0.5 + (double)y - (double)h / 2) / (double)h * PI));
        //if (y % 50 == 0) clog << fw << " ";
        if (verbosity > 1)
            clog << "   " << fixed << setprecision(2) << (float)y / h * 100 << "%          \r";
        for (int x = 0; x < w; x++)
        {
            float r = 0;
            float g = 0;
            float b = 0;
            float a = 0;
            float total = 0;
            for (int v = y * 2 - 3; v <= y * 2 + 4; v++)
            {
                float dy = ((float)(v - y * 2) - 0.5) / 4;
                float weighty = 1 + dy*dy*dy*dy - 2*dy*dy;
                for (int u = x * 2 + 1 - fw; u <= x * 2 + fw; u += fw / 4)
                {
                    float dx = ((float)(u - x * 2) - 0.5) / fw;
                    float weightx = 1 + dx*dx*dx*dx - 2*dx*dx;
                    float weight = weightx * weighty;
                    int p = u;
                    while (p < 0)
                        p += width;
                    p = p % width;
                    int q = v;
                    if (q < 0)
                    {
                        q = -1 - q;
                        p = width - p - 1;
                    }
                    if (q >= height)
                    {
                        q = 2 * height - q - 1;
                        p = width - p - 1;
                    }
                    if (q < 0)
                        q = 0;
                    if (q >= height)
                        q = height - 1;
                    r += data[q * width + p].r * data[q * width + p].a * weight;
                    g += data[q * width + p].g * data[q * width + p].a * weight;
                    b += data[q * width + p].b * data[q * width + p].a * weight;
                    a += data[q * width + p].a * weight;
                    total += weight;
                }
            }
            temp[y * w + x].a = a / total;
            if (a != 0)
            {
                temp[y * w + x].r = r / a;
                temp[y * w + x].g = g / a;
                temp[y * w + x].b = b / a;
            }
            else
            {
                temp[y * w + x].r = 0;
                temp[y * w + x].g = 0;
                temp[y * w + x].b = 0;
            }
        }
    }
    return new image(w, h, temp, true);
}

image* image::doubleSizeN()
{
    int w = width * 2;
    int h = height * 2;
    t_fpixel* temp = new t_fpixel[w * h];
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            temp[y * w + x].r = data[(y / 2) * width + (x / 2)].r;
            temp[y * w + x].g = data[(y / 2) * width + (x / 2)].g;
            temp[y * w + x].b = data[(y / 2) * width + (x / 2)].b;
            temp[y * w + x].a = data[(y / 2) * width + (x / 2)].a;
        }
    }
    return new image(w, h, temp, true);
}

image* image::doubleSizeL()
{
    int w = width * 2;
    int h = height * 2;
    t_fpixel* temp = new t_fpixel[w * h];
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
	    int xm1 = (x + width - 1) % width;
	    int xp1 = (x + 1) % width;
            temp[(y * 2) * w + x * 2].r = (data[y * width + x].r * 9 + data[max(0, y - 1) * width + x].r * 3 + data[max(0, y - 1) * width + xm1].r * 1 + data[y * width + xm1].r * 3) / 16;
            temp[(y * 2) * w + x * 2].g = (data[y * width + x].g * 9 + data[max(0, y - 1) * width + x].g * 3 + data[max(0, y - 1) * width + xm1].g * 1 + data[y * width + xm1].g * 3) / 16;
            temp[(y * 2) * w + x * 2].b = (data[y * width + x].b * 9 + data[max(0, y - 1) * width + x].b * 3 + data[max(0, y - 1) * width + xm1].b * 1 + data[y * width + xm1].b * 3) / 16;
            temp[(y * 2) * w + x * 2].a = (data[y * width + x].a * 9 + data[max(0, y - 1) * width + x].a * 3 + data[max(0, y - 1) * width + xm1].a * 1 + data[y * width + xm1].a * 3) / 16;

	    temp[(y * 2) * w + x * 2 + 1].r = (data[y * width + x].r * 9 + data[max(0, y - 1) * width + x].r * 3 + data[max(0, y - 1) * width + xp1].r * 1 + data[y * width + xp1].r * 3) / 16;
	    temp[(y * 2) * w + x * 2 + 1].g = (data[y * width + x].g * 9 + data[max(0, y - 1) * width + x].g * 3 + data[max(0, y - 1) * width + xp1].g * 1 + data[y * width + xp1].g * 3) / 16;
	    temp[(y * 2) * w + x * 2 + 1].b = (data[y * width + x].b * 9 + data[max(0, y - 1) * width + x].b * 3 + data[max(0, y - 1) * width + xp1].b * 1 + data[y * width + xp1].b * 3) / 16;
	    temp[(y * 2) * w + x * 2 + 1].a = (data[y * width + x].a * 9 + data[max(0, y - 1) * width + x].a * 3 + data[max(0, y - 1) * width + xp1].a * 1 + data[y * width + xp1].a * 3) / 16;

	    temp[(y * 2 + 1) * w + x * 2].r = (data[y * width + x].r * 9 + data[min(height - 1, y + 1) * width + x].r * 3 + data[min(height - 1, y + 1) * width + xm1].r * 1 + data[y * width + xm1].r * 3) / 16;
            temp[(y * 2 + 1) * w + x * 2].g = (data[y * width + x].g * 9 + data[min(height - 1, y + 1) * width + x].g * 3 + data[min(height - 1, y + 1) * width + xm1].g * 1 + data[y * width + xm1].g * 3) / 16;
            temp[(y * 2 + 1) * w + x * 2].b = (data[y * width + x].b * 9 + data[min(height - 1, y + 1) * width + x].b * 3 + data[min(height - 1, y + 1) * width + xm1].b * 1 + data[y * width + xm1].b * 3) / 16;
            temp[(y * 2 + 1) * w + x * 2].a = (data[y * width + x].a * 9 + data[min(height - 1, y + 1) * width + x].a * 3 + data[min(height - 1, y + 1) * width + xm1].a * 1 + data[y * width + xm1].a * 3) / 16;

	    temp[(y * 2 + 1) * w + x  *2 + 1].r = (data[y * width + x].r * 9 + data[min(height - 1, y + 1) * width + x].r * 3 + data[min(height - 1, y + 1) * width + xp1].r * 1 + data[y * width + xp1].r * 3) / 16;
	    temp[(y * 2 + 1) * w + x  *2 + 1].g = (data[y * width + x].g * 9 + data[min(height - 1, y + 1) * width + x].g * 3 + data[min(height - 1, y + 1) * width + xp1].g * 1 + data[y * width + xp1].g * 3) / 16;
	    temp[(y * 2 + 1) * w + x  *2 + 1].b = (data[y * width + x].b * 9 + data[min(height - 1, y + 1) * width + x].b * 3 + data[min(height - 1, y + 1) * width + xp1].b * 1 + data[y * width + xp1].b * 3) / 16;
	    temp[(y * 2 + 1) * w + x  *2 + 1].a = (data[y * width + x].a * 9 + data[min(height - 1, y + 1) * width + x].a * 3 + data[min(height - 1, y + 1) * width + xp1].a * 1 + data[y * width + xp1].a * 3) / 16;
        }
    }
    return new image(w, h, temp, true);
}

void image::alphaBlend(image* img)
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int alpha = data[y * width + x].a;
            data[y * width + x].r = data[y * width + x].r * alpha + img->data[y * img->width + x].r * (1.0 - alpha);
            data[y * width + x].g = data[y * width + x].g * alpha + img->data[y * img->width + x].g * (1.0 - alpha);
            data[y * width + x].b = data[y * width + x].b * alpha + img->data[y * img->width + x].b * (1.0 - alpha);
            data[y * width + x].a = alpha + img->data[y * img->width + x].a * (1.0 - alpha);
        }
    }
}

void image::correctAlpha()
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            data[y * width + x].a = min(1.0f, data[y * width + x].a * 5);
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
            if (data[y * width + x].a != 1.0)
                return false;
        }
    }
    return true;
}

//------------------------------

int interpolator = 1;

bool complete(image* img, int n = 0)
{
    if (verbosity > 0)
        clog << " + Tiefe " << n << " (" << img->width << " x " << img->height << ")" << endl;
    if (img->noTransparentPixels())
    {
        if (verbosity > 0)
            clog << " noTransparentPixels" << endl;
    }
    else
    {
        image* lowRes = img->blurredHalfSize();
        lowRes->correctAlpha();
        complete(lowRes, n + 1);
        image* highRes;
        if (interpolator == 1)
            highRes = lowRes->doubleSizeL();
        else
            highRes = lowRes->doubleSizeN();
        img->alphaBlend(highRes);
        lowRes->~image();
        highRes->~image();
    }
    if (verbosity > 0)
        clog << " - Tiefe " << n << endl;
    return true;
}

//------------------------------

int main(int argc, char** argv)
{
    locale loc("");
    clog.imbue(loc);

    char* iname = NULL;
    char* oname = NULL;
    int c;

    opterr = 0;

    while ((c = getopt(argc, argv, "o:hnvq")) != -1)
    {
        switch (c)
        {
        case 'o':
            oname = optarg;
            break;
        case 'h':
            cout << "help";
            return 0;
        case 'n':
            interpolator = 0;
            break;
        case 'v':
            verbosity++;
            break;
        case 'q':
            verbosity = 0;
            break;
        }
    }
    if (optind <= argc)
    {
        iname = argv[optind];
    }

    if (iname == NULL)
    {
        cerr << "iname == NULL";
        return 1;
    }
    if (oname == NULL)
    {
        cerr << "oname == NULL";
        return 1;
    }

    image* img = new image(iname);
    if (img->data == NULL)
    {
        cerr << "img->data == NULL";
        return 1;
    }
    if (img->onlyTransparentPixels())
    {
        cerr << "img->onlyTransparentPixles()";
        return 1;
    }
    complete(img);
    img->saveToTIFF(oname);

}
