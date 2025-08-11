#include "../include/album_art.h"
#include "../include/id3_parsing.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

IO_ERROR get_album_art(const char *file_path, uint8_t *rgb565_buffer) {
  FILE *f = fopen(file_path, "rb");

  uint8_t buffer[ID3_TAG_HEADER_SIZE];

  if (fread(buffer, ID3_TAG_HEADER_SIZE, 1, f) != 0) {
    fprintf(stderr, "Could not read tag header!\n");
    return COULD_NOT_READ_HEADER;
  }

  ID3TagHeader *tag_header = (ID3TagHeader *)buffer;

  if (is_id3_header(tag_header)) {

    size_t biggest_apic_size = 0;
    size_t biggest_apic_pos = 0;
    size_t current_pos = 10;

    uint32_t tag_size = convert_syncsafe_size(tag_header->size);
    uint8_t major_version = tag_header->version[0];

    // looking for the biggest apic frame
    while (current_pos < tag_size) {
      if (fseek(f, current_pos, 0) != 0) {
        fprintf(stderr, "Could not seek to current pos!\n");
        fclose(f);
        break;
      }

      fread(buffer, ID3_FRAME_HEADER_SIZE, 1, f);

      ID3FrameHeader *frame_header = (ID3FrameHeader *)buffer;
      uint32_t frame_size = get_frame_size(frame_header, major_version);

      if (is_apic(frame_header)) {
        if (frame_size > biggest_apic_size) {
          biggest_apic_size = frame_size;
          biggest_apic_pos = current_pos;
        }
      }

      current_pos += ID3_FRAME_HEADER_SIZE + frame_size;
    }

    if (biggest_apic_size > 0) {
      if (fseek(f, biggest_apic_pos, 0) != 0) {
        fprintf(stderr, "Could not seek to APIC frame!\n");
        fclose(f);
        return COULD_NOT_SEEK_TO_APIC;
      }

      uint8_t *frame_buffer = malloc(biggest_apic_size);

      if (frame_buffer == NULL) {
        fprintf(stderr, "Error: allocation failed for APIC frame\n");
        fclose(f);
        return COULD_NOT_ALLOC_APIC;
      }

      if (fread(frame_buffer, biggest_apic_size, 1, 0) != 1) {
        fprintf(stderr, "Error: failed reading APIC frame body\n");
        free(frame_buffer);
        fclose(f);
        return COULD_NOT_READ_APIC;
      }

      bool result = get_image_data(frame_buffer, biggest_apic_size, rgb565_buffer);

      fclose(f);
      if (result)
        return OK;
      else
        return IMAGE_PROCESSING_ERROR;
    } else {
      fclose(f);
      return NO_APIC;
    }

  } else {
    fprintf(stderr, "No ID3 tag found in file: %s\n", file_path);
    return NO_ID3;
  }
}
