/*************************************************
* Copyright (c) 2017 Xiaozhe Yao
* xiaozhe.yao@gmail.com
**************************************************/

#ifndef __MBLBP_DETECT__
#define __MBLBP_DETECT__


void * LoadMBLBPCascade(const char * filename );
void ReleaseMBLBPCascade(void ** ppCascade);

int * MBLBPDetectMultiScale( unsigned char * pImg, int width, int height, int step, //输入图像
                               void * cascade, //分类器
                               int scale_factor1024x, //扫描窗口缩放系数，是浮点数的1024倍，如果是1.1，此处应为1024*1.1=1126
                               int min_neighbors, //聚类最小近邻数
                               int min_size, //最小扫描窗口大小（即窗口宽度）
							   int max_size=0); //最大扫描窗口大小（即窗口宽度）
#endif
