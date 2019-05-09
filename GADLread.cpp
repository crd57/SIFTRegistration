#include "GDALread.h"
using namespace std;
void GetImageInfo(GDALDataset *poDataset, ImageInfo & St){
	// 获取投影参数
	St.Xsize = poDataset->GetRasterXSize();
	St.Ysize = poDataset->GetRasterYSize();
	St.nbands = poDataset->GetRasterCount();
	St.proj = poDataset->GetProjectionRef();//投影
	poDataset->GetGeoTransform(St.adfGeoTransform);//六参数
	St.iDataType = poDataset->GetRasterBand(1)->GetRasterDataType(); //数据类型


}

GDALDataset* GDALRead(const char* path,ImageInfo & St) {
	GDALAllRegister();
	GDALDataset* poDataset = (GDALDataset*)GDALOpen(path, GA_ReadOnly);
	//检查
	if (poDataset == NULL)
	{
		printf("File: %s 不能打开", path);
	}
	GetImageInfo(poDataset, St);
	std::cout << St.iDataType << std::endl;
	std::cout << "影像大小：" << St.Xsize << "×" << St.Ysize << "×" << St.nbands << std::endl;
	return poDataset;
}

cv::Mat GDAL2Mat(GDALDataset* poDataset, ImageInfo St,int bands[3]) {
	auto MdataType = NULL;
	auto MdataTypes = NULL;
	// 只读取1个波段，或者3个波段
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
	// 转为opencv的数据类型，只写了一部分够用了………
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
	
	//转换
	std::vector<cv::Mat> imgMat;// 每个波段
	float size = St.Xsize * St.Ysize;
	float* pafScan = new float[size];   // 存储数据
	for (int i = 0; i < band_number; i++){
		int band = bands[i];
		GDALRasterBand* pBand = poDataset->GetRasterBand(band);
		pBand->RasterIO(GF_Read, 0, 0, St.Xsize, St.Ysize, pafScan,
			St.Xsize, St.Ysize, St.iDataType, 0, 0);
		cv::Mat tmpMat = cv::Mat(St.Ysize, St.Xsize, MdataType, pafScan);
		imgMat.push_back(tmpMat.clone());
	}
	//释放内存
	delete[]pafScan;
	pafScan = NULL;

	cv::Mat img;
	img.create(St.Ysize, St.Xsize, MdataTypes); 
	//展示
	cv::merge(imgMat, img);
	cv::Mat img_;
	//释放内存
	GDALClose((GDALDatasetH)poDataset);
	if(band_number == 1)
		cv::normalize(img, img_, 0, 255, cv::NORM_MINMAX, CV_8UC1);
	if (band_number == 3)
		cv::normalize(img, img_, 0, 255, cv::NORM_MINMAX, CV_8UC3);
	return img_;
}

bool ImageRowCol2Projection(ImageInfo info, float iCol, float iRow, float& dProjX, float& dProjY)
{
	//adfGeoTransform[6]  数组adfGeoTransform保存的是仿射变换中的一些参数，分别含义见下
	//adfGeoTransform[0]  左上角x坐标 
	//adfGeoTransform[1]  东西方向分辨率
	//adfGeoTransform[2]  旋转角度, 0表示图像 "北方朝上"
	//adfGeoTransform[3]  左上角y坐标 
	//adfGeoTransform[4]  旋转角度, 0表示图像 "北方朝上"
	//adfGeoTransform[5]  南北方向分辨率

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
	//加载图像，这样加载没有投影信息
	Mat image = imread(path, IMREAD_LOAD_GDAL | IMREAD_COLOR | IMREAD_ANYDEPTH);

	//判断是否加载成功
	if (!image.data) //或者image.empty()
	{
		std::cout << path << "cannot open!" << std::endl;
		return;
	}
	// 输出图像大小信息 C x W x H
	std::cout << image.channels() << " x " << image.rows << " x " << image.cols << std::endl;
}