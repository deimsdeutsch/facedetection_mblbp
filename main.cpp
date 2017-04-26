/*************************************************
* Copyright (c) 2017 Xiaozhe Yao
* xiaozhe.yaoi@gmail.com
**************************************************/

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "mblbp-internal.h"
#include "mblbp-detect-mview.h"

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
    int min_neighbors = 2;
    int min_size = 200;
	int max_size = 400;
    int img_width = 1280;
    int img_height = 960;
    void **pCascades;
    void *pCascade_1;
    int angles_array[5] = {0};
    int *angles=angles_array;
    int  step = 2;
    string pImg = "./assets/face_detection.jpg";
    const char * classifier_file = "./assets/mblbpclassifier/cascade4.xml";
    for(int i=1; i<argc;i++)
    {
        bool set = false;
        if( !strcmp( argv[i], "-img" ) )
        {
            pImg = argv[++i];
            cout<<pImg<<endl;
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
        else if( !strcmp( argv[i], "-max_size" ) )
        {
            max_size = atoi(argv[++i]);
        }
        else if( !strcmp( argv[i], "-img_width" ) )
        {
            img_width = atoi(argv[++i]);
        }
        else if( !strcmp( argv[i], "-img_height" ) )
        {
            img_height = atoi(argv[++i]);
        }
    }

    pCascade_1 = LoadMBLBPCascade(classifier_file);
    void * cascades_array[1]={pCascade_1};
    pCascades = cascades_array;
    MBLBPCascade ** ppCascades = (MBLBPCascade**)pCascades;
    int * result = MBLBPDetectMultiScale_Multiview((unsigned char*)pImg.c_str(), img_width, img_height, step, pCascades, angles, 1, 1126, min_neighbors, min_size, max_size);
    cout<<*result<<endl;
    return 0;
}
