/*************************************************
* Copyright (c) 2017 Xiaozhe Yao
* xiaozhe.yaoi@gmail.com
**************************************************/

#include <iostream>
#include <string>
#include "mblbp-internal.h"
#include "mblbp-detect-mview.h"

using namespace std;

int main(int argc, char *argv[])
{
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
    unsigned char *pImg = (unsigned char *)"./assets/face_detection.jpg";
    pCascade_1 = LoadMBLBPCascade("./assets/mblbpclassifier/cascade4.xml");
    void * cascades_array[1]={pCascade_1};
    pCascades = cascades_array;
    MBLBPCascade ** ppCascades = (MBLBPCascade**)pCascades;
    int * result = MBLBPDetectMultiScale_Multiview(pImg, img_width, img_height, step, pCascades, angles, 1, 1126, min_neighbors, min_size, max_size);
    cout<<*result<<endl;
    return 0;
}
