#include <vector>
#include <string>
#include <iostream>
#include "common.h"
#include "lib/jpeg/jpeglib.h"

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

void read_img(char *filename, unsigned char **data, int *width, int *height){
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
