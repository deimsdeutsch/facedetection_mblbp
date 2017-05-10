#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <cctype>
#include <iostream>
#include <iterator>
#include <stdio.h>

using namespace std;
using namespace cv;

int main(int argc, char* argv[])
{
    if(argc == 1){
        cout << "Usage: " << argv[0] << endl;
        cout << "  -img <img_file_name>" << endl;
        cout << "  -classifier <classifier_file_name>" << endl;
        return 0;
    }
    char *pImg;
    char *classifier_file;
    for(int i=1; i<argc;i++)
    {
        if( !strcmp( argv[i], "-img" ) )
        {
            pImg = argv[++i];
        }
        else if( !strcmp( argv[i], "-classifier" ) )
        {
            classifier_file = argv[++i];
        }
    }
    Mat img = imread(pImg,CV_LOAD_IMAGE_GRAYSCALE);
    if(!img.data){
        printf("Could not load image file: %s\n",pImg);
        exit(0);
    }
    CascadeClassifier faces_cascade;
    faces_cascade.load(classifier_file);
    vector<Rect> faces;
    double t = (double)cvGetTickCount(); 
    faces_cascade.detectMultiScale(img,faces,1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );
    t = (double)cvGetTickCount() - t;
    cout<<"detected in "<<(t/((double)cvGetTickFrequency()*1000.))<<"ms"<<endl;
}