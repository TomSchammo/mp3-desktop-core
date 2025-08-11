#ifndef ALBUM_ART_H
#define ALBUM_ART_H

#include <stdint.h>

typedef enum {
  OK,
  NO_ID3,
  NO_APIC,
  COULD_NOT_SEEK_TO_APIC,
  COULD_NOT_READ_APIC,
  COULD_NOT_ALLOC_APIC,
  COULD_NOT_SEEK_TO_POS,
  COULD_NOT_READ_HEADER,
  IMAGE_PROCESSING_ERROR,
} IO_ERROR;

IO_ERROR get_album_art(const char *file_path, uint8_t *rgb565_buffer);

#endif // ALBUM_ART_H
