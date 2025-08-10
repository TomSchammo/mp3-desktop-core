#ifndef ALBUM_ART_H
#define ALBUM_ART_H

typedef enum {
  NO_ID3,
  COULD_NOT_SEEK_TO_APIC,
  COULD_NOT_READ_APIC,
  COULD_NOT_MALLOC_APIC,
  COULD_NOT_SEEK_TO_POS,
  COULD_NOT_READ_HEADER,
} IO_ERROR;

void get_album_art(const char *file_path);

#endif // ALBUM_ART_H
