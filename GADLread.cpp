#include "GDALread.h"
using namespace std;
void GetImageInfo(GDALDataset *poDataset, ImageInfo & St){
	// ��ȡͶӰ����
	St.Xsize = poDataset->GetRasterXSize();
	St.Ysize = poDataset->GetRasterYSize();
	St.nbands = poDataset->GetRasterCount();
	St.proj = poDataset->GetProjectionRef();//ͶӰ
	poDataset->GetGeoTransform(St.adfGeoTransform);//������
	St.iDataType = poDataset->GetRasterBand(1)->GetRasterDataType(); //��������


}

GDALDataset* GDALRead(const char* path,ImageInfo & St) {
	GDALAllRegister();
	GDALDataset* poDataset = (GDALDataset*)GDALOpen(path, GA_ReadOnly);
	//���
	if (poDataset == NULL)
	{
		printf("File: %s ���ܴ�", path);
	}
	GetImageInfo(poDataset, St);
	std::cout << St.iDataType << std::endl;
	std::cout << "Ӱ���С��" << St.Xsize << "��" << St.Ysize << "��" << St.nbands << std::endl;
	return poDataset;
}

cv::Mat GDAL2Mat(GDALDataset* poDataset, ImageInfo St,int bands[3]) {
	auto MdataType = NULL;
	auto MdataTypes = NULL;
	// ֻ��ȡ1�����Σ�����3������
	int band_number;
	if (St.nbands < 3) {
		 band_number= 1;
	}
	else if(St.nbands >= 3 && bands[0] !=bands[1])
	{
		band_number = 3;
	}
	else if (St.nbands >= 3 && bands[0] == bands[1])
	{
		band_number = 1;
	}
	std::cout << band_number;
	// תΪopencv���������ͣ�ֻд��һ���ֹ����ˡ�����
	if (St.iDataType == GDT_Byte)
	{
		MdataType = CV_MAKETYPE(CV_8U, 1);
		MdataTypes = CV_MAKETYPE(CV_8U, band_number);
	}
	if (St.iDataType == GDT_UInt16)
	{
		MdataType = CV_MAKETYPE(CV_16U, 1);
		MdataTypes = CV_MAKETYPE(CV_16U, band_number);
	}
	if (St.iDataType == GDT_Int16)
	{
		MdataType = CV_MAKETYPE(CV_16U, 1);
		MdataTypes = CV_MAKETYPE(CV_16U, band_number);
	}
	
	//ת��
	std::vector<cv::Mat> imgMat;// ÿ������
	float size = St.Xsize * St.Ysize;
	float* pafScan = new float[size];   // �洢����
	for (int i = 0; i < band_number; i++){
		int band = bands[i];
		GDALRasterBand* pBand = poDataset->GetRasterBand(band);
		pBand->RasterIO(GF_Read, 0, 0, St.Xsize, St.Ysize, pafScan,
			St.Xsize, St.Ysize, St.iDataType, 0, 0);
		cv::Mat tmpMat = cv::Mat(St.Ysize, St.Xsize, MdataType, pafScan);
		imgMat.push_back(tmpMat.clone());
	}
	//�ͷ��ڴ�
	delete[]pafScan;
	pafScan = NULL;

	cv::Mat img;
	img.create(St.Ysize, St.Xsize, MdataTypes); 
	//չʾ
	cv::merge(imgMat, img);
	cv::Mat img_;
	//�ͷ��ڴ�
	GDALClose((GDALDatasetH)poDataset);
	if(band_number == 1)
		cv::normalize(img, img_, 0, 255, cv::NORM_MINMAX, CV_8UC1);
	if (band_number == 3)
		cv::normalize(img, img_, 0, 255, cv::NORM_MINMAX, CV_8UC3);
	return img_;
}

bool ImageRowCol2Projection(ImageInfo info, float iCol, float iRow, float& dProjX, float& dProjY)
{
	//adfGeoTransform[6]  ����adfGeoTransform������Ƿ���任�е�һЩ�������ֱ������
	//adfGeoTransform[0]  ���Ͻ�x���� 
	//adfGeoTransform[1]  ��������ֱ���
	//adfGeoTransform[2]  ��ת�Ƕ�, 0��ʾͼ�� "��������"
	//adfGeoTransform[3]  ���Ͻ�y���� 
	//adfGeoTransform[4]  ��ת�Ƕ�, 0��ʾͼ�� "��������"
	//adfGeoTransform[5]  �ϱ�����ֱ���

	try
	{
		dProjX = info.adfGeoTransform[0] + info.adfGeoTransform[1] * iCol + info.adfGeoTransform[2] * iRow;
		dProjY = info.adfGeoTransform[3] + info.adfGeoTransform[4] * iCol + info.adfGeoTransform[5] * iRow;
		return true;
	}
	catch (...)
	{
		return false;
	}
}

void OpencvRead(const char* path) {
	using namespace cv;
	//����ͼ����������û��ͶӰ��Ϣ
	Mat image = imread(path, IMREAD_LOAD_GDAL | IMREAD_COLOR | IMREAD_ANYDEPTH);

	//�ж��Ƿ���سɹ�
	if (!image.data) //����image.empty()
	{
		std::cout << path << "cannot open!" << std::endl;
		return;
	}
	// ���ͼ���С��Ϣ C x W x H
	std::cout << image.channels() << " x " << image.rows << " x " << image.cols << std::endl;
}