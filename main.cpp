#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <ctime>
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
    string pImg = "./assets/face_detection.jpg";
    
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
        else if( !strcmp( argv[i], "-imgWidth" ) )
        {
            img_width = atoi(argv[++i]);
        }
        else if( !strcmp( argv[i], "-imgHeight" ) )
        {
            img_height = atoi(argv[++i]);
        }
    }
    // Print Parameters
    cout << "Parameters:"  << endl;
    cout << "  -img: " <<pImg<< endl;
    cout << "  -classifier: "<<classifier_file << endl;
    cout << "  -minNeighbors: " <<min_neighbors<< endl;
    cout << "  -minSize: "<<min_size<< endl;
    cout << "  -maxSize: " << max_size<<endl;
    cout << "  -imgWidth: "<<img_width << endl;
    cout << "  -imgHeight: "<<img_height << endl;

    // Read image

    pCascade_1 = LoadMBLBPCascade(classifier_file);
    void * cascades_array[1]={pCascade_1};
    bool flips[1]= {true};
    bool * flips_ptr=flips;
    pCascades = cascades_array;
    time_t startTime = 0,endTime = 0;
    time(&startTime);
    cout<<"PIMG"<<pImg.c_str()<<endl;
    int * result = MBLBPDetectMultiScale_Multiview((unsigned char*)pImg.c_str(), img_width, img_height, 150, pCascades, angles, 1,1126, min_neighbors, min_size, max_size,flips_ptr);
    time(&endTime);
    cout<<*result<<endl;
    return 0;
}
