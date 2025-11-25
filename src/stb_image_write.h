/* stb_image_write - v1.16 - public domain - http://nothings.org/stb
   writes out PNG/BMP/TGA/JPEG/HDR images to C stdio - Sean Barrett 2010-2015
   no warranty implied; use at your own risk
*/

#ifndef STB_IMAGE_WRITE_H
#define STB_IMAGE_WRITE_H

#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Public prototype for PNG writer. Implementation is included when
// `STB_IMAGE_WRITE_IMPLEMENTATION` is defined in exactly one translation unit.
int stbi_write_png(const char *filename, int w, int h, int comp, const void *data, int stride_in_bytes);

// Note: the implementation is included when STB_IMAGE_WRITE_IMPLEMENTATION is defined

#ifdef __cplusplus
}
#endif

/* Implementation */
#ifdef STB_IMAGE_WRITE_IMPLEMENTATION

#include <string.h>

static void write_uint32_le(unsigned int x, FILE *f) {
    unsigned char b[4];
    b[0] = x & 0xff;
    b[1] = (x >> 8) & 0xff;
    b[2] = (x >> 16) & 0xff;
    b[3] = (x >> 24) & 0xff;
    fwrite(b, 1, 4, f);
}

/* Minimal PNG writer for RGBA8 data. This is a tiny, limited implementation
   tailored for our use-case: writing small non-interlaced 8-bit RGBA images.
   It avoids zlib by using uncompressed IDAT blocks (not efficient but small).
*/

static unsigned int crc_table[256];
static void make_crc_table(void) {
    unsigned int c;
    for (unsigned int n = 0; n < 256; n++) {
        c = n;
        for (unsigned int k = 0; k < 8; k++) {
            if (c & 1) c = 0xedb88320u ^ (c >> 1);
            else c = c >> 1;
        }
        crc_table[n] = c;
    }
}

static unsigned int update_crc(unsigned int crc, unsigned char *buf, int len) {
    unsigned int c = crc;
    for (int n = 0; n < len; n++)
        c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    return c;
}

static unsigned int crc(unsigned char *buf, int len) {
    return update_crc(0xffffffffu, buf, len) ^ 0xffffffffu;
}

int stbi_write_png(const char *filename, int w, int h, int comp, const void *data, int stride_in_bytes) {
    if (comp < 3) return 0;
    FILE *f = fopen(filename, "wb");
    if (!f) return 0;

    if (crc_table[1] == 0) make_crc_table();

    // PNG signature
    unsigned char pngsig[8] = {137,80,78,71,13,10,26,10};
    fwrite(pngsig, 1, 8, f);

    // IHDR
    unsigned char ihdr[13];
    ihdr[0] = (w >> 24) & 0xff; ihdr[1] = (w >> 16) & 0xff; ihdr[2] = (w >> 8) & 0xff; ihdr[3] = w & 0xff;
    ihdr[4] = (h >> 24) & 0xff; ihdr[5] = (h >> 16) & 0xff; ihdr[6] = (h >> 8) & 0xff; ihdr[7] = h & 0xff;
    ihdr[8] = 8; // bit depth
    ihdr[9] = (comp == 4) ? 6 : 2; // color type: 6=RGBA,2=RGB
    ihdr[10] = 0; ihdr[11] = 0; ihdr[12] = 0;

    write_uint32_le(13, f);
    fwrite("IHDR", 1, 4, f);
    fwrite(ihdr, 1, 13, f);
    unsigned int crc_val = crc((unsigned char*)"IHDR", 4);
    crc_val = update_crc(crc_val, ihdr, 13);
    write_uint32_le(crc_val, f);

    // IDAT - we'll write uncompressed DEFLATE blocks (store) - not efficient but simple
    // Prepare raw image data with filter byte 0 per row
    int bytes_per_pixel = comp;
    int raw_row_bytes = w * bytes_per_pixel + 1;
    int raw_size = raw_row_bytes * h;
    unsigned char *raw = (unsigned char*)malloc(raw_size);
    if (!raw) { fclose(f); return 0; }
    unsigned char *ptr = raw;
    const unsigned char *src = (const unsigned char*)data;
    for (int y = 0; y < h; ++y) {
        *ptr++ = 0; // no filter
        const unsigned char *row = src + y * stride_in_bytes;
        memcpy(ptr, row, w * bytes_per_pixel);
        ptr += w * bytes_per_pixel;
    }

    // zlib header (CMF/FLG) for no compression
    unsigned char zlib_header[2] = {0x78, 0x01};
    // compute Adler-32
    unsigned int s1 = 1, s2 = 0;
    for (int i = 0; i < raw_size; ++i) { s1 = (s1 + raw[i]) % 65521; s2 = (s2 + s1) % 65521; }
    unsigned int adler = (s2 << 16) | s1;

    // We will write a single uncompressed DEFLATE block containing raw
    // Compute IDAT size
    unsigned int idat_content_size = 2 + 5 + raw_size + 4; // zlib header + block header + data + adler

    // create IDAT chunk data in memory
    unsigned char *idat = (unsigned char*)malloc(idat_content_size);
    if (!idat) { free(raw); fclose(f); return 0; }
    unsigned char *q = idat;
    *q++ = zlib_header[0]; *q++ = zlib_header[1];
    // DEFLATE uncompressed block header
    unsigned int len = raw_size;
    *q++ = 0x01; // final block, no compression
    *q++ = len & 0xff; *q++ = (len >> 8) & 0xff;
    *q++ = (~len) & 0xff; *q++ = ((~len) >> 8) & 0xff;
    // copy raw
    memcpy(q, raw, raw_size); q += raw_size;
    // adler32
    *q++ = (adler >> 24) & 0xff; *q++ = (adler >> 16) & 0xff; *q++ = (adler >> 8) & 0xff; *q++ = adler & 0xff;

    write_uint32_le(idat_content_size, f);
    fwrite("IDAT", 1, 4, f);
    fwrite(idat, 1, idat_content_size, f);
    unsigned int crc_idat = crc((unsigned char*)"IDAT", 4);
    crc_idat = update_crc(crc_idat, idat, idat_content_size);
    write_uint32_le(crc_idat, f);

    free(idat);
    free(raw);

    // IEND
    write_uint32_le(0, f);
    fwrite("IEND", 1, 4, f);
    unsigned int crc_iend = crc((unsigned char*)"IEND", 4);
    write_uint32_le(crc_iend, f);

    fclose(f);
    return 1;
}

#endif // STB_IMAGE_WRITE_IMPLEMENTATION

#endif // STB_IMAGE_WRITE_H
