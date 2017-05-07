﻿#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <ctime>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "common.h"
#include "mblbp-internal.h"
#include "mblbp-detect-mview.h"

using namespace cv;
using namespace std;

int main(int argc, char *argv[])
{
    if(argc == 1){
        cout << "Usage: " << argv[0] << endl;
        cout << "  -img <img_file_name>" << endl;
        cout << "  -classifier <classifier_file_name>" << endl;
        cout << "  -minNeighbors <minimum neighbors>" << endl;
        cout << "  -minSize <minimum size>" << endl;
        cout << "  -maxSize <maximum size>" << endl;
        cout << "  -imgWidth <image width>" << endl;
        cout << "  -imgHeight <image height>" << endl;
        return 0;
    }
    int min_neighbors = 5;
    int min_size = 20;
	int max_size = 200;
    int img_width = 1280;
    int img_height = 960;
    void **pCascades;
    void *pCascade_1;
    int angles_array[24] = {0,1,2,3,4,5,6,7,8,9,10,11,12,14,15,16,17,18,19,20,21,22,23};
    int *angles=angles_array;
    int  step = 10;
    char * pImg;
    
    const char * classifier_file = "./assets/cascade10.xml";
    for(int i=1; i<argc;i++)
    {
        bool set = false;
        if( !strcmp( argv[i], "-img" ) )
        {
            pImg = argv[++i];
        }
        else if( !strcmp( argv[i], "-classifier" ) )
        {
            classifier_file = argv[++i];
        }
        else if( !strcmp( argv[i], "-minNeighbors" ) )
        {
            min_neighbors = atoi(argv[++i]);
        }
        else if( !strcmp( argv[i], "-minSize" ) )
        {
            min_size = atoi(argv[++i]);
        }
        else if( !strcmp( argv[i], "-maxSize" ) )
        {
            max_size = atoi(argv[++i]);
        }
    }
    unsigned char * image_pointer;
    // Read Image
    //read_bmp(pImg,&image_pointer,&img_width,&img_height);
    Mat src;
    src = imread(pImg,CV_LOAD_IMAGE_GRAYSCALE);
    if(!src.data){
        printf("Could not load image file: %s\n",pImg);
        exit(0);
    }
    img_width = src.size().width;
    img_height = src.size().height;
    image_pointer = (unsigned char *)src.data;

    for(int i=0;i<100;i++){
        cout<<*(image_pointer+i)<<" ";
    }

    // Print Parameters
    cout << "Parameters:"  << endl;
    cout << "  -img: " <<pImg<< endl;
    cout << "  -classifier: "<<classifier_file << endl;
    cout << "  -minNeighbors: " <<min_neighbors<< endl;
    cout << "  -minSize: "<<min_size<< endl;
    cout << "  -maxSize: " << max_size<<endl;
    cout << "  -imgWidth: "<<img_width<< endl;
    cout << "  -imgHeight: " << img_height<<endl;

    pCascade_1 = LoadMBLBPCascade(classifier_file);
    void * cascades_array[1]={pCascade_1};
    bool flips[1]= {true};
    bool * flips_ptr=flips;
    pCascades = cascades_array;
    time_t startTime = 0,endTime = 0;
    time(&startTime);
    int * result = MBLBPDetectMultiScale_Multiview(image_pointer, img_width, img_height, img_width, pCascades, angles, 1,1126, min_neighbors, min_size, max_size,flips_ptr);
    time(&endTime);
    cout<<*result<<endl;
    return 0;
}
