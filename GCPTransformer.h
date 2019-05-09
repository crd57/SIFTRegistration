#pragma once
#include <gdal_priv.h>
#include <gdal.h>
#include <opencv2/opencv.hpp>  
#include "GDALread.h"
#include <gdal_alg.h>
#include <gdalwarper.h>
#include <gdal_priv.h>
using namespace cv;
using namespace std;


struct TransformChain {
	/*GDAL×ø±ê×ª»»Ö¸Õë*/
	GDALTransformerFunc GDALTransformer;
	void* GDALTransformerArg;
	double adfGeotransform[6];
	double adfInvGeotransform[6];
};
int CreateGCPsList(vector<KeyPoint> keypoints_1, vector<KeyPoint> keypoints_2, vector<DMatch> Matches, ImageInfo info,GDAL_GCP* gcplist);
int ImageWarpByGCP(const char * pszSrcFile,
	const char * pszDstFile,
	int nGCPCount,
	const GDAL_GCP *pasGCPList,
	const char * pszDstWKT,
	int iOrder,
	double dResX,
	double dResY,
	GDALResampleAlg eResampleMethod,
	const char * pszFormat);

