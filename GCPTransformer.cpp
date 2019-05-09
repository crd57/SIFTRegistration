#include "GCPTransformer.h"

int CreateGCPsList(vector<KeyPoint> keypoints_1, vector<KeyPoint> keypoints_2, vector<DMatch> Matches, ImageInfo info,GDAL_GCP* gcplist)
{
	vector<vector<float>> GCPPixel;
	vector<vector<float>> GCPXY;
	for (int i = 0; i < Matches.size(); i++)
	{
		cout << i << endl;
		Point2f pt_1;
		Point2f pt_2;
		float x, y;
		pt_1 = keypoints_1[Matches[i].queryIdx].pt;
		ImageRowCol2Projection(info, pt_1.x, pt_1.y, x, y);
		pt_2 = keypoints_2[Matches[i].trainIdx].pt;
		char id[10];
		sprintf(id, "%d", i);
		gcplist[i].pszId = id;
		gcplist[i].pszInfo = (char*) "create by sift";
		gcplist[i].dfGCPLine = (double)pt_2.y;
		gcplist[i].dfGCPPixel = (double)pt_2.x;
		gcplist[i].dfGCPX = (double)x;
		gcplist[i].dfGCPY = (double)y;
		gcplist[i].dfGCPZ = 0;
	}
	return Matches.size();
}

int ImageWarpByGCP(const char * pszSrcFile,
	const char * pszDstFile,
	int nGCPCount,
	const GDAL_GCP *pasGCPList,
	const char * pszDstWKT,
	int iOrder,
	double dResX,
	double dResY,
	GDALResampleAlg eResampleMethod,
	const char * pszFormat)
{

	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	// ��ԭʼͼ��
	GDALDatasetH hSrcDS = GDALOpen(pszSrcFile, GA_ReadOnly);
	if (NULL == hSrcDS)
	{
		return 0;
	}

	GDALDataType eDataType = GDALGetRasterDataType(GDALGetRasterBand(hSrcDS, 1));
	int nBandCount = GDALGetRasterCount(hSrcDS);

	// �������ζ���ʽ����ת����ϵ
	void *hTransform = GDALCreateGCPTransformer(nGCPCount, pasGCPList, iOrder, FALSE);
	if (NULL == hTransform)
	{
		GDALClose(hSrcDS);
		return 0;
	}

	// �������ͼ��������Χ����С������任����������Ϣ
	double adfGeoTransform[6] = { 0 };
	double adfExtent[4] = { 0 };
	int    nPixels = 0, nLines = 0;

	if (GDALSuggestedWarpOutput2(hSrcDS, GDALGCPTransform, hTransform,
		adfGeoTransform, &nPixels, &nLines, adfExtent, 0) != CE_None)
	{
		GDALClose(hSrcDS);
		return 0;
	}

	// ���濪ʼ�����û�ָ���ķֱ������������ͼ��Ĵ�С������������Ϣ
	double dResXSize = dResX;
	double dResYSize = dResY;

	//���Ϊ0����Ĭ��ΪԭʼӰ��ķֱ���
	if (dResXSize == 0.0 && dResYSize == 0.0)
	{
		double dbGeoTran[6] = { 0 };
		GDALGCPsToGeoTransform(nGCPCount, pasGCPList, dbGeoTran, 0);
		dResXSize = fabs(dbGeoTran[1]);
		cout << dResXSize << endl;
		dResYSize = fabs(dbGeoTran[5]);
	}

	// ����û�ָ�������ͼ��ķֱ���
	else if (dResXSize != 0.0 || dResYSize != 0.0)
	{
		if (dResXSize == 0.0) dResXSize = adfGeoTransform[1];
		if (dResYSize == 0.0) dResYSize = adfGeoTransform[5];
	}

	if (dResXSize < 0.0) dResXSize = -dResXSize;
	if (dResYSize > 0.0) dResYSize = -dResYSize;

	// �������ͼ��ķ�Χ
	double minX = adfGeoTransform[0];
	double maxX = adfGeoTransform[0] + adfGeoTransform[1] * nPixels;
	double maxY = adfGeoTransform[3];
	double minY = adfGeoTransform[3] + adfGeoTransform[5] * nLines;

	nPixels = ceil((maxX - minX) / dResXSize);
	nLines = ceil((minY - maxY) / dResYSize);
	adfGeoTransform[0] = minX;
	adfGeoTransform[3] = maxY;
	adfGeoTransform[1] = dResXSize;
	adfGeoTransform[5] = dResYSize;

	// �������ͼ��
	GDALDriverH hDriver = GDALGetDriverByName(pszFormat);
	if (NULL == hDriver)
	{
		return 0;
	}
	GDALDatasetH hDstDS = GDALCreate(hDriver, pszDstFile, nPixels, nLines, nBandCount, eDataType, NULL);
	if (NULL == hDstDS)
	{
		return 0;
	}
	GDALSetProjection(hDstDS, pszDstWKT);
	GDALSetGeoTransform(hDstDS, adfGeoTransform);

	//���ԭʼͼ�������������
	int nXsize = GDALGetRasterXSize(hSrcDS);
	int nYsize = GDALGetRasterYSize(hSrcDS);

	//Ȼ����ͼ���ز���
	int nFlag = 0;
	float dfValue = 0;
	CPLErr err = CE_Failure;
	//���ڽ�����
	for (int nBandIndex = 0; nBandIndex < nBandCount; nBandIndex++)
	{
		GDALRasterBandH hSrcBand = GDALGetRasterBand(hSrcDS, nBandIndex + 1);
		GDALRasterBandH hDstBand = GDALGetRasterBand(hDstDS, nBandIndex + 1);
		for (int nRow = 0; nRow < nLines; nRow++)
		{
			for (int nCol = 0; nCol < nPixels; nCol++)
			{
				double dbX = adfGeoTransform[0] + nCol * adfGeoTransform[1]
					+ nRow * adfGeoTransform[2];
				double dbY = adfGeoTransform[3] + nCol * adfGeoTransform[4]
					+ nRow * adfGeoTransform[5];

				//�������ͼ���������ϵ�任��ԭʼ����������ϵ
				GDALGCPTransform(hTransform, TRUE, 1, &dbX, &dbY, NULL, &nFlag);
				int nXCol = (int)(dbX + 0.5);
				int nYRow = (int)(dbY + 0.5);

				//������Χ����0���
				if (nXCol < 0 || nXCol >= nXsize || nYRow < 0 || nYRow >= nYsize)
				{
					dfValue = 0;
				}

				else
				{
					err = GDALRasterIO(hSrcBand, GF_Read, nXCol, nYRow, 1, 1, &dfValue, 1, 1, eDataType, 0, 0);

				}
				err = GDALRasterIO(hDstBand, GF_Write, nCol, nRow, 1, 1, &dfValue, 1, 1, eDataType, 0, 0);

			}
		}

	}



	if (hTransform != NULL)
	{
		GDALDestroyGCPTransformer(hTransform);
		hTransform = NULL;
	}

	GDALClose(hSrcDS);
	GDALClose(hDstDS);


	return 1;
}