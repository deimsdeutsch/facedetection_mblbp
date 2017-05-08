#ifndef __MBLBP_DETECT_COMMON__
#define __MBLBP_DETECT_COMMON__

#include <vector>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
using namespace std;

vector<string> split(const std::string &s, const std::string &seperator);

void read_jpg(char *filename, unsigned char **data, int *width, int *height);
void read_bmp(char *filename, unsigned char **data, int *width, int *height);

#endif