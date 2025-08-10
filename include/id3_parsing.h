
#ifndef ID3_PARSING_H
#define ID3_PARSING_H

#include "../include/img_processing.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef enum { JPEG, PNG, LINK, OTHER } ImageType;

/**
 * Struct containing the data of the ID3 tag header.
 *
 * identifier:    3 bytes containing the letters 'I' 'D' and '3' or in hex 0x49 0x44 and 0x33
 * version:       Two bytes containing the major version and the revision
 * flags:         One byte containing the flags for the tag with the 4 lower bits always set to 0
 *                (0bxxxx0000).
 * size_bytes:    4 bytes containing the size of the tag as an unsigned, BE, syncsafe 32 bit integer
 */
typedef struct {
  const char identifier[3];
  const uint8_t version[2];
  const uint8_t flags;
  const uint8_t size[4];
} __attribute__((packed)) ID3TagHeader;

/**
 * Struct containing the data of a frame header
 *
 * id:              4 bytes containing the frame id
 * size;            4 bytes containing the size of the tag as an unsigned, syncsafe 32 bit integer
 * status_flags:    One byte containing the frame status flags where msb and the 4 lower bits are
 * always set to 0 (0b0xx0000)
 * format_flags:    One byte containing the frame format flags with the
 * msb and the 5th and 6th bit always set to 0 (0b0x00xxxx)
 *
 */
typedef struct {
  const char id[4];
  const uint8_t size[4];
  const uint8_t status_flags;
  const uint8_t format_flags;
} __attribute__((packed)) ID3FrameHeader;

#define ID3_TAG_HEADER_SIZE sizeof(ID3TagHeader)
#define ID3_FRAME_HEADER_SIZE sizeof(ID3FrameHeader)

_Static_assert((ID3_TAG_HEADER_SIZE == ID3_FRAME_HEADER_SIZE),
               "ID3 Tag and Frame headers are expected to have the same size!");

[[nodiscard]]
inline uint32_t convert_syncsafe_size(const uint8_t *size) {
  return ((size[0] & 0x7F) << 21) | ((size[1] & 0x7F) << 14) | ((size[2] & 0x7F) << 7) |
         (size[3] & 0x7F);
}

[[nodiscard]]
inline uint32_t convert_be32_size(const uint8_t *size) {
  return ((unsigned long)size[0] << 24) | ((unsigned long)size[1] << 16) |
         ((unsigned long)size[2] << 8) | (unsigned long)size[3];
}

[[nodiscard]]
inline uint32_t get_frame_size(const ID3FrameHeader *frame_header, uint8_t major_version) {
  return major_version == 4 ? convert_syncsafe_size(frame_header->size)
                            : convert_be32_size(frame_header->size);
}

[[nodiscard]]
inline bool is_id3_header(const ID3TagHeader *tag_header) {
  if (tag_header->identifier[0] == 'I' && tag_header->identifier[1] == 'D' &&
      tag_header->identifier[2] == '3') {
    return true;
  }

  printf("ID3 ID not correct: %c%c%c\n", tag_header->identifier[0], tag_header->identifier[1],
         tag_header->identifier[2]);
  return false;
}

[[nodiscard]]
inline bool is_apic(const ID3FrameHeader *frame_header) {

  return (frame_header->id[0] == 'A' && frame_header->id[1] == 'P' && frame_header->id[2] == 'I' &&
          frame_header->id[3] == 'C');
}

[[nodiscard]]
Image *get_image_data(uint8_t *frame_buffer, uint32_t frame_size);

#endif // ID3_PARSING_H
