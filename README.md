
# MP3 Desktop Core

This is the core, platform independent functionality for the desktop applications that are supposed to interact with the MP3 player.
For now this only contains the image preprocessing code, but is a work in progress.

The native desktop applications should include this code in one form or another.


The current API expects the desktop application to call the `IO_ERROR get_album_art(const char *file_path, uint8_t *rgb565_buffer)`
function, giving it the path to an MP3 file and a buffer for the rgb565 image (dimensions are 200x200).
