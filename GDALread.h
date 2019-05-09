#pragma once
#include <gdal_priv.h>
#include <gdal.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <vector>


/*! byte */
typedef unsigned char   byte;
/*! 8U */
typedef unsigned char   DT_8U;
/*! 16U */
typedef unsigned short  DT_16U;
/*! 16S */
typedef short           DT_16S;
/*! 32U */
typedef unsigned int    DT_32U;
/*! 32S */
typedef int             DT_32S;
/*! 32F */
typedef float           DT_32F;
/*! 64F */
typedef double          DT_64F;


//创建一个存放影像信息的structure
typedef struct
{
	//GDALDataset *poDataset = NULL;
	int Xsize = 0;
	int Ysize = 0;
	int nbands = 0;
	double* adfGeoTransform = new double[6]; //存储地理6参数
	const char* proj = NULL;//存储投影
	GDALDataType iDataType = GDT_Byte;
}ImageInfo;


//影像信息
void GetImageInfo(GDALDataset* poDataset, ImageInfo & St);
//使用GDAL打开影像
GDALDataset* GDALRead(const char* path,ImageInfo & St);
//GDAL转Mat
cv::Mat GDAL2Mat(GDALDataset *poDataset, ImageInfo St,int bands[3]);
//获取像素的地理坐标
bool ImageRowCol2Projection(ImageInfo info, float iCol, float iRow, float& dProjX, float& dProjY);
//opencv打开没有坐标
void OpencvRead(const char* path);
