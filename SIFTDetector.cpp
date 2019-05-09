#include "SIFTDetector.h"


Ptr<Feature2D> detector = xfeatures2d::SIFT::create();


bool DetectorKeyPoint(Mat img_1, Mat img_2,vector<KeyPoint> &keypoints_1, vector<KeyPoint>&  keypoints_2)
{
	if (img_1.empty()) cout << "空影像";
	detector->detect(img_1, keypoints_1);
	detector->detect(img_2, keypoints_2);
	if (!keypoints_1.empty() && !keypoints_2.empty()) return true;
	else return false;
}

bool ComputeDescriptor(Mat img_1, Mat img_2, vector<KeyPoint>& keypoints_1, vector<KeyPoint>& keypoints_2,Mat& descriptors_1, Mat& descriptors_2) {
	detector->compute(img_1, keypoints_1, descriptors_1);
	detector->compute(img_2, keypoints_2, descriptors_2);
	if (!keypoints_1.empty() && !keypoints_2.empty()) return true;
	else return false;
}

int FeatureMatch(vector<vector<DMatch> > &matchePoints, Mat& descriptors_1, Mat& descriptors_2)
{
	FlannBasedMatcher matcher;
	vector<Mat> train_desc(1, descriptors_2);
	matcher.add(train_desc);
	matcher.train();
	matcher.knnMatch(descriptors_1, matchePoints, 2);
	int size = matchePoints.size();
	cout << "匹配的关键点个数为：" << matchePoints.size() << endl;
	return size;
}

/*首先根据最近邻(NN)和第二近邻(SCN)的距离之比(NN / SCN), 选择那些可靠性较高的匹配点对来计算几何约束模型*/
vector<DMatch> NNSCNCheck(vector<vector<DMatch> > Matches,float threshold) {
	vector<DMatch> GoodMatchePoints;
	for (int i = 0; i < Matches.size(); i++)
	{
		if (Matches[i][0].distance < threshold * Matches[i][1].distance)
		{
			GoodMatchePoints.push_back(Matches[i][0]);
		}
	}
	cout << "NN/SCN的阈值为："<< threshold <<"	筛选后的点数为："<<GoodMatchePoints.size() << endl;
	return GoodMatchePoints;
	
}


int RANSACCheck( vector<KeyPoint> keypoints_1, vector<KeyPoint> keypoints_2, Mat descriptors_1, Mat descriptors_2, vector<DMatch> Matches, string model, vector<KeyPoint> &RR_keypoint01, vector<KeyPoint> &RR_keypoint02, vector<DMatch> &RR_matches,double param1,double param2 ){
	if (!(model == string("Fundamental") || model == string("Homography")))
		CV_Error(CV_StsBadArg, "RANSAC模块图像变换类型输入错误！");
	// 分配空间     
	int ptCount = (int)Matches.size();

	if (ptCount < 100)
	{
		CV_Error(CV_StsBadArg, "没有足够的特征点！");
	}
	Mat p1(ptCount, 2, CV_32F);
	Mat p2(ptCount, 2, CV_32F);

	//取出匹配的特征点
	vector<KeyPoint> R_keypoint01, R_keypoint02;
	for (int i = 0; i < Matches.size(); i++)
	{
		R_keypoint01.push_back(keypoints_1[Matches[i].queryIdx]);
		R_keypoint02.push_back(keypoints_2[Matches[i].trainIdx]);
	}
	// 把Keypoint转换为Mat     
	Point2f pt;
	for (int i = 0; i < ptCount; i++)
	{
		pt = keypoints_1[Matches[i].queryIdx].pt;
		p1.at<float>(i, 0) = pt.x;
		p1.at<float>(i, 1) = pt.y;

		pt = keypoints_2[Matches[i].trainIdx].pt;
		p2.at<float>(i, 0) = pt.x;
		p2.at<float>(i, 1) = pt.y;
	}
	// 用RANSAC方法计算
	vector<uchar> m_RANSACStatus;       // 这个变量用于存储RANSAC后每个点的状态
	if (model == string("Fundamental")) {
		Mat m_Fundamental = findFundamentalMat(p1, p2, m_RANSACStatus, FM_RANSAC,param1, param2);
	}
	if (model == string("Homography")) {
		Mat m_Homography = findHomography(p1, p2, m_RANSACStatus, RANSAC, param1);
	}
	int index = 0;
	for (size_t i = 0; i < Matches.size(); i++)
	{
		if (m_RANSACStatus[i] != 0)
		{
			RR_keypoint01.push_back(R_keypoint01[i]);
			RR_keypoint02.push_back(R_keypoint02[i]);
			Matches[i].queryIdx = index;
			Matches[i].trainIdx = index;
			RR_matches.push_back(Matches[i]);
			index++;
		}
		//cout << m_RANSACStatus[i];
	}
	cout << "RANSAC使用的矩阵为：" << model << "	筛选后的点数为：" << RR_matches.size() << endl;
	return RR_matches.size();
}

int DistributedCheck(Mat img_2, vector<KeyPoint> keypoints_1, vector<KeyPoint> keypoints_2, vector<DMatch> Matches,vector<KeyPoint>& RR_keypoint01, vector<KeyPoint>& RR_keypoint02, vector<DMatch>& RR_matches,int distance)
{
	int Y = img_2.rows;
	int X = img_2.cols;
	int index = 0;
	for (int i = 0; i < Y+ distance; i += distance)
	{
		for (int j = 0; j < X+ distance; j += distance)
		{

			for (int k =0; k < Matches.size(); k++)
			{
				//cout << keypoints_2[Matches[k].trainIdx].pt.x << endl;
				if (keypoints_2[Matches[k].trainIdx].pt.x >= j && keypoints_2[Matches[k].trainIdx].pt.x < j + distance && 
					keypoints_2[Matches[k].trainIdx].pt.y >= i && keypoints_2[Matches[k].trainIdx].pt.y < i + distance) 
				{
					RR_keypoint01.push_back(keypoints_1[k]);
					RR_keypoint02.push_back(keypoints_2[k]);
					Matches[k].queryIdx = index;
					Matches[k].trainIdx = index;
					RR_matches.push_back(Matches[k]);
					index++;
					break;
				}
			}
		}
	}
	//int index = 0;
	//for (int i = 0; i < Matches.size(); i+=distance)
	//{
	//	int j;
	//	for ( j = i + 1; j < Matches.size(); j++)
	//	{
	//		float dist_x = abs((keypoints_2[Matches[i].trainIdx].pt.x - keypoints_2[Matches[j].trainIdx].pt.x));
	//		float dist_y = abs((keypoints_2[Matches[i].trainIdx].pt.y - keypoints_2[Matches[j].trainIdx].pt.y));
	//		if (dist_x < distance || dist_y < distance) break;
	//	}
	//	j += 1;
	//	if (j == Matches.size())
	//	{
	//		RR_keypoint01.push_back(keypoints_1[Matches[i].queryIdx]);
	//		RR_keypoint02.push_back(keypoints_2[Matches[i].trainIdx]);
	//		Matches[i].queryIdx = index;
	//		Matches[i].trainIdx = index;
	//		RR_matches.push_back(Matches[i]);
	//		index++;
	//	}
	//}
	cout << "在" << distance <<"*" << distance <<"的中只取一个点" << "	筛选后的点数为：" << RR_matches.size() << endl;
	return RR_matches.size();
}

Mat WarpImage(Mat img_1, Mat img_2, vector<KeyPoint> keypoints_1, vector<KeyPoint> keypoints_2, vector<DMatch> Matches)
{
	int ptCount = (int)Matches.size();
	Mat p1(ptCount, 2, CV_32F);
	Mat p2(ptCount, 2, CV_32F);
	//取出匹配的特征点
	vector<KeyPoint> R_keypoint01, R_keypoint02;
	for (int i = 0; i < Matches.size(); i++)
	{
		R_keypoint01.push_back(keypoints_1[Matches[i].queryIdx]);
		R_keypoint02.push_back(keypoints_2[Matches[i].trainIdx]);
	}
	// 把Keypoint转换为Mat     
	Point2f pt;
	for (int i = 0; i < ptCount; i++)
	{
		pt = keypoints_1[Matches[i].queryIdx].pt;
		p1.at<float>(i, 0) = pt.x;
		p1.at<float>(i, 1) = pt.y;

		pt = keypoints_2[Matches[i].trainIdx].pt;
		p2.at<float>(i, 0) = pt.x;
		p2.at<float>(i, 1) = pt.y;
	}


	Mat Homography = findHomography(p1, p2, CV_RANSAC);
	////也可以使用getPerspectiveTransform方法获得透视变换矩阵，不过要求只能有4个点，效果稍差
	//Mat	homo=getPerspectiveTransform(imagePoints1,imagePoints2);   
	//cout << "变换矩阵为：\n" << homo << endl << endl; //输出映射矩阵
//图像配准
	Mat imageTransform;
	warpPerspective(img_2, imageTransform, Homography, Size(img_1.cols, img_1.rows));
	return imageTransform;
}







