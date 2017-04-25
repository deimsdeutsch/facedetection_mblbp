/*************************************************
* Copyright (c) 2017 Xiaozhe Yao
* xiaozhe.yao@gmail.com
**************************************************/

#include <iostream>
#include <string>
#include "mblbp-internal.h"
#include "mblbp-detect-mview.h"

using namespace std;

int main(int argc, char *argv[])
{
    cout <<"calling main"<<endl;
    int min_neighbors = 3;
    int min_size = 60;
	int max_size = 100;
    int img_width = 1280;
    int img_height = 960;
    void **pCascade;
    int angles_array[5] = {0,1,2,3,4};
    int *angles;
    angles = angles_array;

    int  step = 2;
    unsigned char *pImg = (unsigned char *)"./assets/face_detection.jpg";
    cout <<"preparing"<<endl;
    pCascade = (void **)LoadMBLBPCascade("./assets/cascade1.xml");
    cout<<"start training"<<endl;
    int * result = MBLBPDetectMultiScale_Multiview(pImg,img_width,img_height,step, pCascade, angles, 2, 1024, min_neighbors, min_size, max_size);
    cout<<"Finished"<<endl;
    cout<<result<<endl;
    return 0;
}

