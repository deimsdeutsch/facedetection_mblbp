/*************************************************
* Copyright (c) 2017 Xiaozhe Yao
* xiaozhe.yao@gmail.com
**************************************************/

#include "mblbp-detect-mview.h"
#include "mblbp-internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;

/*************************************************
 * flip gray image
 * psrc should be the same size with pdst
 *************************************************/
void myFlip(unsigned char * psrc, int width, int height, int step, 
            unsigned char * pdst)
{
    unsigned char * s;
    unsigned char * d;

    for(int r = 0; r < height; r++)
    {
        s = psrc + r * step;
        d = pdst + r * step;
#ifdef _OPENMP
    #pragma omp parallel for
#endif
        for(int c = 0; c < width; c++)
        {
            d[c] = s[width-1-c];
        }
    }
}

int * MBLBPDetectMultiScale_Multiview( unsigned char * pImg, int width, int height, int step,
                                       void ** cascades, 
                                       int * angles,
                                       int classifier_num, 
                                       int scale_factor1024x,
                                       int min_neighbors, 
                                       int min_size,
                                       int max_size,
                                       bool * flips)
{
    MBLBPCascade ** ppCascades = (MBLBPCascade**)cascades;
    int factor1024x;
    int factor1024x_max;
    int classifier_win_size;

    if( ! pImg) 
    {
        fprintf( stderr, "%s: null image pointer", __FUNCTION__ );
        return NULL;
    }
    //check the classifiers pointer
    if( ! ppCascades) 
    {
        fprintf( stderr, "%s: Invalid classifiers", __FUNCTION__ );
        return NULL;
    }
    //check the number of classifiers
    if( classifier_num < 1 )
    {
        fprintf( stderr, "%s: classifier number must be a positive number", __FUNCTION__ );
        return NULL;
    }
    //check the classifiers
    for(int i=0; i < classifier_num; i++)
    {
        if( ! ppCascades[i]) 
        {
            fprintf( stderr, "%s: The %d classifier is invalid", __FUNCTION__, i);
            return NULL;
        }
    }
    //check window size in the classifiers
    classifier_win_size = ppCascades[0]->win_width;
    for(int i=1; i < classifier_num; i++)
    {
        if( ppCascades[i]->win_width != classifier_win_size)
        {
            fprintf( stderr, "%s: The window size in the classifiers must be the same", __FUNCTION__);
            return NULL;
        }
    }

    min_size  = MAX(classifier_win_size,  min_size);
	if(max_size <=0 )
		max_size = MIN(width, height);
	if(max_size < min_size)
		return NULL;

    //clear memory
    memset(&g_faceRects, 0 , sizeof(g_faceRects));

    factor1024x = ((min_size<<10)+(classifier_win_size/2)) / classifier_win_size;
	factor1024x_max = (max_size<<10) / classifier_win_size; //do not round it, to avoid the scan window be out of range

    for( ; factor1024x <= factor1024x_max;
         factor1024x = ((factor1024x*scale_factor1024x+512)>>10) )
    {
        int dwidth = ((width<<10)+factor1024x/2)/factor1024x;
        int dheight = ((height<<10)+factor1024x/2)/factor1024x;
        int dstep = (((dwidth * 8 + 7)/8)+ 4 - 1) & (~(4 - 1));
        unsigned char * psmall = (unsigned char *)malloc( dstep * dheight);

        unsigned char * psmallflip = NULL;
        psmallflip = (unsigned char *)malloc( dstep * dheight);
        if(psmall==NULL || psmallflip==NULL)
        {
            fprintf(stderr, "can not alloc memory.\n");
            return NULL;
        }
        
        myResize(pImg, width, height, step, 
                 psmall, dwidth, dheight, dstep);
        myFlip(psmall, dwidth, dheight, dstep, psmallflip); 

        Size winStride = createSize( (factor1024x<=2048)+1,  (factor1024x<=2048)+1 );

        for(int i = 0; i < classifier_num; i++)
        {
            MBLBPDetectSingleScale( psmall,     dwidth, dheight, dstep, ppCascades[i], winStride, factor1024x, angles[i], false);
            if( flips[i] )
                MBLBPDetectSingleScale( psmallflip, dwidth, dheight, dstep, ppCascades[i], winStride, factor1024x, -angles[i], true);
        }

        free(psmall);
        free(psmallflip);
    }
    
    GroupRects(&g_faceRects, &g_faceRectsBuf, min_neighbors);

    return (int*)(&g_faceRects);
}