#include <vector>
#include <string>
#include <iostream>
#include "common.h"

using namespace std;

vector<string> split(const string &s, const string &seperator)
{
  vector<string> result;
  typedef string::size_type string_size;
  string_size i = 0;

  while (i != s.size())
  {
    int flag = 0;
    while (i != s.size() && flag == 0)
    {
      flag = 1;
      for (string_size x = 0; x < seperator.size(); ++x)
          if (s[i] == seperator[x])
        {
              ++i;
              flag = 0;
              break;
        }
    }

    flag = 0;
    string_size j = i;
    while (j != s.size() && flag == 0)
    {
      for (string_size x = 0; x < seperator.size(); ++x)
          if (s[j] == seperator[x])
        {
              flag = 1;
              break;
        }
      if (flag == 0)
          ++j;
    }
    if (i != j)
    {
      result.push_back(s.substr(i, j - i));
      i = j;
    }
  }
  return result;
}

void read_jpg(char *filename, unsigned char **data, int *width, int *height)
{
    FILE * infile = fopen(filename, "rb");
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);
    *width = cinfo.output_width;
    *height = cinfo.output_height;
    *data = (unsigned char *) malloc(cinfo.output_height * cinfo.output_width * cinfo.output_components);
    unsigned char *line_pointer;
    int i = 0;
    while (cinfo.output_scanline < cinfo.image_height) {
        line_pointer = *data + i * cinfo.output_width * cinfo.output_components;
        jpeg_read_scanlines(&cinfo, &line_pointer, 1); 
        i ++;
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
}

void read_bmp(char *filename, unsigned char **data, int *width, int *height)
{
    int i;
    FILE* f = fopen(filename, "rb");
    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header

    // extract image height and width from header
    int origin_width = *(int*)&info[18];
    int origin_height = *(int*)&info[22];

    int size = 3 * origin_width * origin_height;
    unsigned char* origin_data = new unsigned char[size]; // allocate 3 bytes per pixel
    fread(origin_data, sizeof(unsigned char), size, f); // read the rest of the data at once
    fclose(f);

    for(i = 0; i < size; i += 3)
    {
            unsigned char tmp = origin_data[i];
            origin_data[i] = origin_data[i+2];
            origin_data[i+2] = tmp;
    }
    *width = origin_width;
    *height = origin_height;
    *data= origin_data;
}

