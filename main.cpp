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
    int min_neighbors = 3;
    int min_size = 60;
	int max_size = 100;
    int img_width = 1280;
    int img_height = 960;
    void **pCascade;
    int angles_array[5] = {0,1,2,3,4};
    int *angles=angles_array;

    int  step = 2;
    unsigned char *pImg = (unsigned char *)"./assets/face_detection.jpg";
    pCascade = (void **)LoadMBLBPCascade("./assets/mblbpclassifier/cascade1.xml");

    cout <<"Convert from void ** to MBLBP"<<endl;
    MBLBPCascade ** ppCascades;
    ppCascades[0] = (MBLBPCascade**)pCascade;
    cout << ppCascades[0]->count<<endl;
    cout <<"Finished Converting"<<endl;

    cout<<"Start Detection"<<endl;
    int * result = MBLBPDetectMultiScale_Multiview(pImg, img_width, img_height, step, pCascade, angles, 1, 1024, min_neighbors, min_size, max_size);
    cout<<"Finished"<<endl;
    return 0;
}
