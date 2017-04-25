/*************************************************
* Copyright (c) 2017 Xiaozhe Yao
* xiaozhe.yaoi@gmail.com
**************************************************/

#include "mblbp-detect.h"
#include "mblbp-internal.h"
#include "tinyxml2.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;
using namespace tinyxml2;
#ifdef _OPENMP
omp_lock_t lock;
#endif

FaceRects g_faceRects;
FaceRectsBuf g_faceRectsBuf;

#define MBLBP_CALC_SUM(p0, p1, p2, p3, offset_) \
((p0)[offset_] - (p1)[offset_] - (p2)[offset_] + (p3)[offset_])

Size createSize(int w, int h)
{
    Size sz;
    sz.width = w;
    sz.height = h;
    return sz;
};

inline int is_equal_rect(const FaceRect *r1, const FaceRect *r2)
{
    int delta10x = MIN(r1->width, r2->width) + MIN(r1->height, r2->height);
    return abs(r1->x - r2->x) * 10 <= delta10x &&
           abs(r1->y - r2->y) * 10 <= delta10x &&
           abs(r1->x + r1->width - r2->x - r2->width) * 10 <= delta10x &&
           abs(r1->y + r1->height - r2->y - r2->height) * 10 <= delta10x;
}

inline int intersectionArea(const FaceRectBuf *r1, const FaceRectBuf *r2)
{
    FaceRectBuf r;
    r.x = MAX(r1->x, r2->x);
    r.y = MAX(r1->y, r2->y);
    r.width = MIN(r1->x + r1->width, r2->x + r2->width) - r.x;
    r.height = MIN(r1->y + r1->height, r2->y + r2->height) - r.y;

    if (r.width > 0 && r.height > 0)
        return r.width * r.height;
    else
        return 0;
}

// pFaces will be modified in the function
void GroupRects(FaceRects *pFaces, FaceRectsBuf *pFacesBuf, int min_neighbors)
{
    if (min_neighbors <= 0)
        return;

    int nLabels[__MAX_FACE_NUM__];
    //init label
    for (int i = 0; i < pFaces->count; i++)
    {
        nLabels[i] = (short)i;
    }

    // group rectangles
    // the computational cost is a little higher,
    // but it can save memory, and avoid to use a list structure.
    for (int i = 0; i < pFaces->count - 1; i++)
    {
        for (int j = i + 1; j < pFaces->count; j++)
        {
            if (is_equal_rect(pFaces->faces + i, pFaces->faces + j))
            {
                short min_label = MIN(nLabels[i], nLabels[j]);
                short max_label = MAX(nLabels[i], nLabels[j]);

                for (int k = 0; k < pFaces->count; k++)
                    if (nLabels[k] == max_label)
                        nLabels[k] = min_label;
            }
        }
    }

    memset(pFacesBuf, 0, sizeof(*pFacesBuf));

    pFacesBuf->count = pFaces->count;
    for (int i = 0; i < pFacesBuf->count; i++)
    {
        int label = nLabels[i];
        pFacesBuf->faces[label].x += pFaces->faces[i].x;
        pFacesBuf->faces[label].y += pFaces->faces[i].y;
        pFacesBuf->faces[label].width += pFaces->faces[i].width;
        pFacesBuf->faces[label].height += pFaces->faces[i].height;
        pFacesBuf->faces[label].neighbors++;
        pFacesBuf->faces[label].angle += pFaces->faces[i].angle;
    }

    memset(pFaces, 0, sizeof(*pFaces));

    int new_label = 0;
    for (int i = 0; i < pFacesBuf->count; i++)
    {
        if (pFacesBuf->faces[i].neighbors > 0)
        {
            int n = pFacesBuf->faces[i].neighbors;
            pFaces->faces[new_label].x = (short)((pFacesBuf->faces[i].x * 2 + n) / (2 * n));
            pFaces->faces[new_label].y = (short)((pFacesBuf->faces[i].y * 2 + n) / (2 * n));
            pFaces->faces[new_label].width = (short)((pFacesBuf->faces[i].width * 2 + n) / (2 * n));
            pFaces->faces[new_label].height = (short)((pFacesBuf->faces[i].height * 2 + n) / (2 * n));
            pFaces->faces[new_label].neighbors = (short)(n);
            pFaces->faces[new_label].angle = (short)((pFacesBuf->faces[i].angle * 2 + n) / (2 * n));

            new_label++;
        }
    }
    pFaces->count = new_label;

    //swap
    pFacesBuf->count = pFaces->count;
    for (int i = 0; i < pFacesBuf->count; i++)
    {
        pFacesBuf->faces[i].x = pFaces->faces[i].x;
        pFacesBuf->faces[i].y = pFaces->faces[i].y;
        pFacesBuf->faces[i].width = pFaces->faces[i].width;
        pFacesBuf->faces[i].height = pFaces->faces[i].height;
        pFacesBuf->faces[i].neighbors = pFaces->faces[i].neighbors;
        pFacesBuf->faces[i].angle = pFaces->faces[i].angle;
    }
    memset(pFaces, 0, sizeof(*pFaces));

    // filter out small face rectangles inside large face rectangles
    for (int i = 0; i < pFacesBuf->count; i++)
    {
        bool frb1_is_good = true;
        FaceRectBuf *frb1 = pFacesBuf->faces + i;
        for (int j = 0; j < pFacesBuf->count; j++)
        {
            FaceRectBuf *frb2 = pFacesBuf->faces + j;

            int area = intersectionArea(frb1, frb2);
            bool overlap = (area * 2 >= frb1->width * frb1->height) || (area * 2 >= frb2->width * frb2->height);

            if ((i != j) && overlap &&
                ((frb1->neighbors < frb2->neighbors) ||
                 ((frb1->neighbors == frb2->neighbors) && (i < j))))
            {
                frb1_is_good = false;
                break;
            }
        }

        if (frb1_is_good && pFacesBuf->faces[i].neighbors >= min_neighbors)
        {
            int c = pFaces->count;
            pFaces->faces[c].x = (short)(pFacesBuf->faces[i].x);
            pFaces->faces[c].y = (short)(pFacesBuf->faces[i].y);
            pFaces->faces[c].width = (short)(pFacesBuf->faces[i].width);
            pFaces->faces[c].height = (short)(pFacesBuf->faces[i].height);
            pFaces->faces[c].neighbors = (short)(pFacesBuf->faces[i].neighbors);
            pFaces->faces[c].angle = (short)(pFacesBuf->faces[i].angle);
            pFaces->count++;
        }
    }
}

void *LoadMBLBPCascade(const char *filename)
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile(filename);
    FILE *pFile = fopen(filename, "rb");

    if (pFile == NULL)
    {
        fprintf(stderr, "Can not load detector from file %s\n", filename);
        return NULL;
    }

    MBLBPCascade *pCascade = (MBLBPCascade *)malloc(sizeof(MBLBPCascade));
    memset(pCascade, 0, sizeof(MBLBPCascade));

    cout << "READING FILE" << endl;

    const char *version = doc.FirstChildElement("opencv_storage")->FirstChildElement("cascade")->FirstChildElement("version")->GetText();
    pCascade->win_width = atoi(doc.FirstChildElement("opencv_storage")->FirstChildElement("cascade")->FirstChildElement("width")->GetText());
    pCascade->win_height = atoi(doc.FirstChildElement("opencv_storage")->FirstChildElement("cascade")->FirstChildElement("height")->GetText());
    pCascade->count = atoi(doc.FirstChildElement("opencv_storage")->FirstChildElement("cascade")->FirstChildElement("stageNum")->GetText());
    cout << "Version : " << version << endl;
    cout << "PARAMETERS : " << endl;
    cout << "pCascade Win Width : " << pCascade->win_width << endl;
    cout << "pCascade Win Height : " << pCascade->win_height << endl;
    cout << "pCascade Count : " << pCascade->count << endl;
    pCascade->win_height = 40;
    pCascade->win_width = 80;
    pCascade->count = 2;
    pCascade->stages = (MBLBPStage *)malloc(sizeof(MBLBPStage) * pCascade->count);
    memset(pCascade->stages, 0, sizeof(MBLBPStage) * pCascade->count);
    tinyxml2::XMLElement *root = doc.FirstChildElement("opencv_storage")->FirstChildElement("cascade")->FirstChildElement("stages");
    int tmp_i = 0;
    for (tinyxml2::XMLElement *stage = root->FirstChildElement("_"); stage != NULL; stage = stage->NextSiblingElement("_"))
    {
        pCascade->stages[tmp_i].count = atoi(stage->FirstChildElement("maxWeakCount")->GetText());
        pCascade->stages[tmp_i].threshold = atoi(stage->FirstChildElement("stageThreshold")->GetText());
        pCascade->stages[tmp_i].weak_classifiers = (MBLBPWeak *)malloc(sizeof(MBLBPWeak) * pCascade->stages[tmp_i].count);
        memset(pCascade->stages[tmp_i].weak_classifiers, 0, sizeof(MBLBPWeak) * pCascade->stages[tmp_i].count);
        int tmp_j = 0;
        for (tinyxml2::XMLElement *weak_classifier = stage->FirstChildElement("weakClassifiers")->FirstChildElement("_"); weak_classifier != NULL; weak_classifier = weak_classifier->NextSiblingElement("_"))
        {
            MBLBPWeak *pWeak = pCascade->stages[tmp_i].weak_classifiers + tmp_j;
            string s = weak_classifier->FirstChildElement("rect")->GetText();
            vector<string> v = split(s, " ");
            pWeak->x = atoi(v[0].c_str());
            pWeak->y = atoi(v[1].c_str());
            pWeak->cellwidth = atoi(v[2].c_str());
            pWeak->cellheight = atoi(v[3].c_str());
            pWeak->soft_threshold = atoi(weak_classifier->FirstChildElement("weakThreshold")->GetText());
            string look_up_table_string = weak_classifier->FirstChildElement("lut")->GetText();
            vector<string> lutv = split(look_up_table_string, " ");

            tmp_j++;
        }
        tmp_i++;
    }
    /*
    for (int i = 0; i < pCascade->count; i++) 
    {
        if ( fread( &(pCascade->stages[i].count) , sizeof(int), 1, pFile) != 1) 
            goto EXIT_TAG;

        //float tmp;

        if ( fread( &(pCascade->stages[i].threshold), sizeof(int), 1, pFile) != 1)
            goto EXIT_TAG;

        pCascade->stages[i].weak_classifiers = (MBLBPWeak*)malloc( sizeof(MBLBPWeak) *  pCascade->stages[i].count);
        memset(pCascade->stages[i].weak_classifiers, 0, sizeof(MBLBPWeak) *  pCascade->stages[i].count);


        for(int j = 0; j < pCascade->stages[i].count; j++)
        {
            MBLBPWeak * pWeak = pCascade->stages[i].weak_classifiers + j;

            if (fread(&(pWeak->x), sizeof(int), 1, pFile) != 1) 
                goto EXIT_TAG;
            if (fread(&(pWeak->y), sizeof(int), 1, pFile) != 1) 
                goto EXIT_TAG;
            if (fread(&(pWeak->cellwidth), sizeof(int), 1, pFile) != 1) 
                goto EXIT_TAG;
            if (fread(&(pWeak->cellheight), sizeof(int), 1, pFile) != 1) 
                goto EXIT_TAG;
            if (fread(&(pWeak->soft_threshold), sizeof(int), 1, pFile) != 1) 
                goto EXIT_TAG;

            //for(int k = 0; k < MBLBP_LUTLENGTH; k++) {
            if (fread(pWeak->look_up_table, sizeof(int)*MBLBP_LUTLENGTH, 1, pFile) != 1) 
                    goto EXIT_TAG;
            //}
        }
    }
    */
    fclose(pFile);
    return pCascade;

EXIT_TAG:

    fclose(pFile);
    ReleaseMBLBPCascade((void **)(&pCascade));
    return NULL;
}

void ReleaseMBLBPCascade(void **ppCascade)
{
    if (!ppCascade)
        return;
    MBLBPCascade **pp = (MBLBPCascade **)ppCascade;
    MBLBPCascade *pCascade = *pp;
    if (!pCascade)
        return;

    for (int i = 0; i < pCascade->count && pCascade->stages; i++)
    {
        for (int j = 0; j < pCascade->stages[i].count; j++)
        {
            free(pCascade->stages[i].weak_classifiers);
            pCascade->stages[i].weak_classifiers = NULL;
        }
    }
    free(pCascade->stages);
    pCascade->stages = NULL;
    free(pCascade);
    pCascade = NULL;
}

//////////////////////////////////////////////
// pSrc: image data
// width: image width
// height: image height
// step: the number of bytes per row of pSrc
// pSum: integral image, the same size with pSrc
//////////////////////////////////////////////
void myIntegral(unsigned char *pSrc, int width, int height, int step, int *pSum, int sum_width)
{
    int x, y;
    int s;
    unsigned char *psrc = pSrc;
    int *psum = pSum;
    int src_step = step;
    int sum_step = sum_width;

    if (pSrc == NULL || pSum == NULL)
    {
        fprintf(stderr, "%s: NULL pointer.\n", __FUNCTION__);
        return;
    }
    if (width <= 0 || height <= 0 || step <= 0)
    {
        fprintf(stderr, "%s: Invalid image size.\n", __FUNCTION__);
        return;
    }

    //the first row
    for (x = 0; x < width + 1; x++)
        psum[x] = 0;
    //the first column
    for (y = 1; y < height + 1; y++)
        psum[y * sum_step] = 0;

    for (y = 1, psum += sum_step;
         y < height + 1;
         y++, psrc += src_step, psum += sum_step)
    {
        for (x = 1, s = 0; x < width + 1; x++)
        {
            s += (psrc[x - 1]);
            psum[x] = psum[x - sum_step] + s;
        }
    }

    return;
}

// pCascade: the classifier
// pSum: the integral image
// width: width of pSum
// height: height of pSum
void UpdateCascade(MBLBPCascade *pCascade, int *pSum, int sum_width)
{
    if (pSum == NULL)
    {
        fprintf(stderr, "%s: Null integral image pointer", __FUNCTION__);
        return;
    }

    if (!pCascade)
    {
        fprintf(stderr, "%s: Invalid cascade classifier", __FUNCTION__);
        return;
    }

    for (int i = 0; i < pCascade->count; i++)
    {
        for (int j = 0; j < pCascade->stages[i].count; j++)
        {
            MBLBPWeak *pw = pCascade->stages[i].weak_classifiers + j;
            int x = pw->x;
            int y = pw->y;
            int w = pw->cellwidth;
            int h = pw->cellheight;

            pw->p[0] = pSum + y * sum_width + (x);
            pw->p[1] = pSum + y * sum_width + (x + w);
            pw->p[2] = pSum + y * sum_width + (x + w * 2);
            pw->p[3] = pSum + y * sum_width + (x + w * 3);

            pw->p[4] = pSum + (y + h) * sum_width + (x);
            pw->p[5] = pSum + (y + h) * sum_width + (x + w);
            pw->p[6] = pSum + (y + h) * sum_width + (x + w * 2);
            pw->p[7] = pSum + (y + h) * sum_width + (x + w * 3);

            pw->p[8] = pSum + (y + h * 2) * sum_width + (x);
            pw->p[9] = pSum + (y + h * 2) * sum_width + (x + w);
            pw->p[10] = pSum + (y + h * 2) * sum_width + (x + w * 2);
            pw->p[11] = pSum + (y + h * 2) * sum_width + (x + w * 3);

            pw->p[12] = pSum + (y + h * 3) * sum_width + (x);
            pw->p[13] = pSum + (y + h * 3) * sum_width + (x + w);
            pw->p[14] = pSum + (y + h * 3) * sum_width + (x + w * 2);
            pw->p[15] = pSum + (y + h * 3) * sum_width + (x + w * 3);
        }
    }

    return;
}

inline int DetectAt(MBLBPCascade *pCascade, int offset)
{
    if (!pCascade)
        return 0;
    int confidence = 0;

    for (int i = 0; i < pCascade->count; i++)
    {
        int stage_sum = 0;
        int code = 0;

        MBLBPWeak *pw = pCascade->stages[i].weak_classifiers;

        for (int j = 0; j < pCascade->stages[i].count; j++)
        {
            int **p = pw->p;

            int cval = MBLBP_CALC_SUM(p[5], p[6], p[9], p[10], offset);

            code = ((MBLBP_CALC_SUM(p[0], p[1], p[4], p[5], offset) >= cval) << 7) |
                   ((MBLBP_CALC_SUM(p[1], p[2], p[5], p[6], offset) >= cval) << 6) |
                   ((MBLBP_CALC_SUM(p[2], p[3], p[6], p[7], offset) >= cval) << 5) |
                   ((MBLBP_CALC_SUM(p[6], p[7], p[10], p[11], offset) >= cval) << 4) |
                   ((MBLBP_CALC_SUM(p[10], p[11], p[14], p[15], offset) >= cval) << 3) |
                   ((MBLBP_CALC_SUM(p[9], p[10], p[13], p[14], offset) >= cval) << 2) |
                   ((MBLBP_CALC_SUM(p[8], p[9], p[12], p[13], offset) >= cval) << 1) |
                   ((MBLBP_CALC_SUM(p[4], p[5], p[8], p[9], offset) >= cval));

            stage_sum += pw->look_up_table[LBPMAP[code]];

            if (stage_sum < pw->soft_threshold)
                return -i;

            pw++;
        }

        if (stage_sum < pCascade->stages[i].threshold)
            return -i;
        else
            confidence = stage_sum - pCascade->stages[i].threshold + 1; //when stage_sum==threshold, confidence should be > 0; to avoid confusing with return -i(i==0)
    }

    return confidence;
}

void MBLBPDetectSingleScale(unsigned char *pimg, int width, int height, int step,
                            MBLBPCascade *pCascade,
                            Size winStride,
                            int factor1024x,
                            int angle,
                            bool isflip)
{
    int *sum = NULL;
    int sum_width, sum_height;
    int ystep, xstep, ymax, xmax;

    if (!pimg)
    {
        fprintf(stderr, "%s: Null image pointer\n", __FUNCTION__);
        return;
    }

    if (!pCascade)
    {
        fprintf(stderr, "%s: Invalid classifier cascade\n", __FUNCTION__);
        return;
    }

    if (pCascade->win_width > width ||
        pCascade->win_height > height)
        return;

    sum_width = width + 1;
    sum_height = height + 1;
    sum = (int *)malloc(sum_width * sum_height * sizeof(int));
    if (sum == NULL)
    {
        fprintf(stderr, "%s: can not alloc memory.\n", __FUNCTION__);
        return;
    }

    myIntegral(pimg, width, height, step, sum, sum_width);
    UpdateCascade(pCascade, sum, sum_width);

    ystep = winStride.height;
    xstep = winStride.width;
    // '-1' is to avoid that
    // the face rect is out of the image range caused by the round error
    ymax = height - pCascade->win_height - 1;
    xmax = width - pCascade->win_width - 1;

#ifdef _OPENMP
    omp_init_lock(&lock);
#pragma omp parallel for
#endif
    for (int iy = 0; iy < ymax; iy += ystep)
    {
        for (int ix = 0; ix < xmax; ix += xstep)
        {
            int w_offset = iy * sum_width + ix;
            int result = DetectAt(pCascade, w_offset);
            if (result > 0)
            {
                FaceRect fr;

                fr.x = ix;
                fr.y = iy;
                if (isflip)
                    fr.x = width - 1 - fr.x - pCascade->win_width;

                fr.x = (short)((fr.x * factor1024x + 512) >> 10);
                fr.y = (short)((fr.y * factor1024x + 512) >> 10);
                fr.width = (short)((pCascade->win_width * factor1024x + 512) >> 10);
                fr.height = (short)((pCascade->win_height * factor1024x + 512) >> 10);
                fr.neighbors = 1; //result/256;
                fr.angle = angle;
#ifdef _OPENMP
                omp_set_lock(&lock);
#endif
                if (g_faceRects.count < __MAX_FACE_NUM__)
                {
                    g_faceRects.faces[g_faceRects.count] = fr;
                    g_faceRects.count++;
                }
#ifdef _OPENMP
                omp_unset_lock(&lock);
#endif
            }
            if (result == 0)
            {
                ix += xstep;
            }
        }
    }
#ifdef _OPENMP
    omp_destroy_lock(&lock);
#endif

    free(sum);
    return;
}

int *MBLBPDetectMultiScale(unsigned char *pImg, int width, int height, int step,
                           void *cascade,
                           int scale_factor1024x,
                           int min_neighbors,
                           int min_size,
                           int max_size)
{
    MBLBPCascade *pCascade = (MBLBPCascade *)cascade;
    int factor1024x;
    int factor1024x_max;

    if (!pImg)
    {
        fprintf(stderr, "%s: null image pointer", __FUNCTION__);
        return NULL;
    }
    if (!pCascade)
    {
        fprintf(stderr, "%s: Invalid classifier cascade", __FUNCTION__);
        return NULL;
    }

    min_size = MAX(pCascade->win_width, min_size);
    if (max_size <= 0)
        max_size = MIN(width, height);
    if (max_size < min_size)
        return NULL;

    //clear memory
    memset(&g_faceRects, 0, sizeof(g_faceRects));

    factor1024x = ((min_size << 10) + (pCascade->win_width / 2)) / pCascade->win_width;
    factor1024x_max = (max_size << 10) / pCascade->win_width; //do not round it, to avoid the scan window be out of range

    for (; factor1024x <= factor1024x_max;
         factor1024x = ((factor1024x * scale_factor1024x + 512) >> 10))
    {
        int dwidth = ((width << 10) + factor1024x / 2) / factor1024x;
        int dheight = ((height << 10) + factor1024x / 2) / factor1024x;
        int dstep = (((dwidth * 8 + 7) / 8) + 4 - 1) & (~(4 - 1));
        unsigned char *psmall = (unsigned char *)malloc(dstep * dheight);
        if (psmall == NULL)
        {
            fprintf(stderr, "can not alloc memory.\n");
            return NULL;
        }
        myResize(pImg, width, height, step,
                 psmall, dwidth, dheight, dstep);

        Size winStride = createSize((factor1024x <= 2048) + 1, (factor1024x <= 2048) + 1);

        MBLBPDetectSingleScale(psmall, dwidth, dheight, dstep, pCascade, winStride, factor1024x);

        free(psmall);
    }

    //angle estimation is not needed for single view face detection.
    for (int i = 0; i < g_faceRects.count; i++)
    {
        g_faceRects.faces[i].angle = 0;
    }

    GroupRects(&g_faceRects, &g_faceRectsBuf, min_neighbors);

    return (int *)(&g_faceRects);
}
