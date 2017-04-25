/*************************************************
* Copyright (c) 2015 Shiqi Yu
* shiqi.yu@gmail.com
* The software is licensed to Hubei MicroPattern Corporation.
**************************************************/

#include <stdlib.h>
#include <string.h>

#define ICV_WARP_SHIFT          10
#define ICV_WARP_SHIFT2         15
#define ICV_SHIFT_DIFF          (ICV_WARP_SHIFT2-ICV_WARP_SHIFT)
#define ICV_WARP_MASK           ((1 << ICV_WARP_SHIFT) - 1)

#define  CV_DESCALE(x,n)     (((x) + (1 << ((n)-1))) >> (n))

#define ICV_WARP_MUL_ONE_8U(x)  ((x) << ICV_WARP_SHIFT)
#define ICV_WARP_DESCALE_8U(x)  CV_DESCALE((x), ICV_WARP_SHIFT*2)
#define CV_SWAP(a,b,t) ((t) = (a), (a) = (b), (b) = (t))


 /****************************************************************************************\
*                                         Resize                                         *
\****************************************************************************************/



typedef struct CvResizeAlpha
{
    int idx;
    int ialpha;
}CvResizeAlpha;


static int 
icvResize_Bilinear_8u_C1( const unsigned char* src, int srcstep, int swidth, int sheight,
                                   unsigned char* dst, int dststep, int dwidth, int dheight,
                                   int xmax,
                                   const CvResizeAlpha* xofs,
                                   const CvResizeAlpha* yofs,
                                   int* buf0, int* buf1 )
{
    int prev_sy0 = -1, prev_sy1 = -1;                                           
    int k, dx, dy;                                                              
                                                                                
    srcstep /= sizeof(src[0]);                                                  
    dststep /= sizeof(dst[0]);                                                  
                                                                                
    for( dy = 0; dy < dheight; dy++, dst += dststep )                      
    {                                                                           
        int fy = yofs[dy].ialpha, *swap_t;                            
        int sy0 = yofs[dy].idx, sy1 = sy0 + (fy > 0 && sy0 < sheight-1);   
                                                                                
        if( sy0 == prev_sy0 && sy1 == prev_sy1 )                                
            k = 2;                                                              
        else if( sy0 == prev_sy1 )                                              
        {                                                                       
            CV_SWAP( buf0, buf1, swap_t );                                      
            k = 1;                                                              
        }                                                                       
        else                                                                    
            k = 0;                                                              
                                                                                
        for( ; k < 2; k++ )                                                     
        {                                                                       
            int* _buf = k == 0 ? buf0 : buf1;                              
            const unsigned char* _src;                                                
            int sy = k == 0 ? sy0 : sy1;                                        
            if( k == 1 && sy1 == sy0 )                                          
            {                                                                   
                memcpy( buf1, buf0, dwidth*sizeof(buf0[0]) );              
                continue;                                                       
            }                                                                   
                                                                                
            _src = src + sy*srcstep;                                            
            for( dx = 0; dx < xmax; dx++ )                                      
            {                                                                   
                int sx = xofs[dx].idx;                                          
                int fx = xofs[dx].ialpha;                             
                int t = _src[sx];                                          
                _buf[dx] = ICV_WARP_MUL_ONE_8U(t) + fx*(_src[sx+1] - t);             
            }                                                                   
                                                                                
            for( ; dx < dwidth; dx++ )                                     
                _buf[dx] = ICV_WARP_MUL_ONE_8U(_src[xofs[dx].idx]);                   
        }                                                                       
                                                                                
        prev_sy0 = sy0;                                                         
        prev_sy1 = sy1;                                                         
                                                                                
        if( sy0 == sy1 )                                                        
            for( dx = 0; dx < dwidth; dx++ )                               
                dst[dx] = (unsigned char)ICV_WARP_DESCALE_8U( ICV_WARP_MUL_ONE_8U(buf0[dx]));     
        else                                                                    
            for( dx = 0; dx < dwidth; dx++ )                               
                dst[dx] = (unsigned char)ICV_WARP_DESCALE_8U( ICV_WARP_MUL_ONE_8U(buf0[dx]) +     
                                                  fy*(buf1[dx] - buf0[dx]));    
    }                                                                           
                                                                                
    return 1;
}


//////////////////////////////////////////////////////////////////////////////////////////

void
myResize(unsigned char * psrc, int swidth, int sheight, int sstep,
         unsigned char * pdst, int dwidth, int dheight, int dstep)
{
    void* temp_buf = 0;
    
    int scale_x, scale_y;
    int sx, sy, dx, dy;
    int xmax = dwidth, buf_size;
    int *buf0, *buf1;
    CvResizeAlpha *xofs, *yofs;
    int fx_1024x, fy_1024x;
    
    scale_x = ((swidth<<ICV_WARP_SHIFT2)+dwidth/2)/dwidth;
    scale_y = ((sheight<<ICV_WARP_SHIFT2)+dheight/2)/dheight;


    buf_size = dwidth*2*sizeof(int) + (dwidth + dheight)*sizeof(CvResizeAlpha);
    temp_buf = buf0 = (int*)malloc(buf_size);
    buf1 = buf0 + dwidth;
    xofs = (CvResizeAlpha*)(buf1 + dwidth);
    yofs = xofs + dwidth;
    
    for( dx = 0; dx < dwidth; dx++ )
    {
        fx_1024x = ((dx*2+1)*scale_x-(1<<ICV_WARP_SHIFT2))/2;
        sx = (fx_1024x>>ICV_WARP_SHIFT2);
        fx_1024x = ((fx_1024x-(sx<<ICV_WARP_SHIFT2))>>ICV_SHIFT_DIFF);

        if( sx < 0 )
            sx = 0, fx_1024x = 0;
        
        if( sx >= swidth-1 )
        {
            fx_1024x = 0, sx = swidth-1;
            if( xmax >= dwidth )
                xmax = dx;
        }
        
        xofs[dx].idx = sx;
        xofs[dx].ialpha = fx_1024x;
    }
    
    for( dy = 0; dy < dheight; dy++ )
    {
        fy_1024x = ((dy*2+1)*scale_y-(1<<ICV_WARP_SHIFT2))/2;
        sy = (fy_1024x>>ICV_WARP_SHIFT2);
        fy_1024x = ((fy_1024x-(sy<<ICV_WARP_SHIFT2))>>ICV_SHIFT_DIFF);

        if( sy < 0 )
            sy = 0, fy_1024x = 0;
        
        yofs[dy].idx = sy;
        yofs[dy].ialpha = fy_1024x;
    }

    icvResize_Bilinear_8u_C1( psrc, sstep, swidth, sheight, pdst,
                               dstep, dwidth, dheight, xmax, xofs, yofs, buf0, buf1 );
    
    free( temp_buf );
}

