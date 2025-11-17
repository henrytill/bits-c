#include <png.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#include "printf.h"

#define WIDTH  2048
#define HEIGHT 2048

struct Context
{
    char const *filename;
    int nerrors;
    int nwarnings;
};

static void onerror(png_structp png_ptr, png_const_charp msg)
{
    struct Context *ctx = png_get_error_ptr(png_ptr);
    if (ctx != NULL)
    {
        eprintf("%s: libpng error: %s\n", ctx->filename, msg);
        ctx->nerrors++;
    }
    else
    {
        eprintf("libpng error: %s\n", msg);
    }

    longjmp(png_jmpbuf(png_ptr), 1);
}

static void onwarning(png_structp png_ptr, png_const_charp msg)
{
    struct Context *ctx = png_get_error_ptr(png_ptr);
    if (ctx != NULL)
    {
        eprintf("%s: libpng warning: %s\n", ctx->filename, msg);
        ctx->nwarnings++;
    }
    else
    {
        eprintf("libpng warning: %s\n", msg);
    }
}

int main(void)
{
    struct Context ctx;
    FILE *fp = NULL;
    png_structp png = NULL;
    png_infop info = NULL;
    int x, y, ret = 1;
    unsigned char row[WIDTH];

    ctx.filename = "avatar.png";
    ctx.nerrors = 0;
    ctx.nwarnings = 0;

    fp = fopen(ctx.filename, "wb");
    if (fp == NULL)
        return 1;

    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, &ctx, onerror, onwarning);
    if (png == NULL)
        goto fclosefp;

    info = png_create_info_struct(png);
    if (info == NULL)
        goto destroypng;

    if (setjmp(png_jmpbuf(png)))
        goto destroypng;

    png_init_io(png, fp);
    png_set_IHDR(png, info, WIDTH, HEIGHT, 8, PNG_COLOR_TYPE_GRAY,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    for (x = 0; x < WIDTH; ++x)
        row[x] = 128;

    for (y = 0; y < HEIGHT; ++y)
        png_write_row(png, row);

    png_write_end(png, NULL);

    ret = 0;

destroypng:
    png_destroy_write_struct(&png, &info);
fclosefp:
    (void)fclose(fp);
    return ret;
}
