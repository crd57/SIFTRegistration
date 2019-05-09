#include "SIFTDetector.h"
#include "GDALread.h"
#include "GCPTransformer.h"


int main() {
	cout << "�������׼Ӱ��·��" << endl;
	string srcpath;
	cin >> srcpath;
	cout << "���������׼Ӱ��·��" << endl;
	string pzpath;
	cin >> pzpath;
	int b;
	int srcband[3];
	int i = 0;
	cout << "�������׼Ӱ���"<<("%d",i+1)<<"�Ĳ���"<< endl;
	while (cin >> b && i <=2) {
		srcband[i] = b;
		i++;
		if (i == 3)
			break;
		cout << "�������׼Ӱ���" << ("%d", i + 1) << "�Ĳ��Σ����������ظ���һ��ֵ��" << endl;
	}
	int pzband[3];
	i = 0;
	cout << "��������׼Ӱ���" << ("%d", i + 1) << "�Ĳ���" << endl;
	while (cin >> b && i <=2) {
		pzband[i] = b;
		i++;
		if (i == 3)
			break;
		cout << "��������׼Ӱ���" << ("%d", i + 1) << "�Ĳ��Σ����������ظ���һ��ֵ��" << endl;
	}
	cout << "���ڶ�ȡӰ�����Եȣ�" << endl;
	const char* bzFile = srcpath.c_str();
	const char* pzFile = pzpath.c_str();
	ImageInfo bzinfo;
	GDALDataset *bzpoDataset = GDALRead(bzFile, bzinfo);
	cout <<"bz proj::::" <<bzinfo.proj << endl;
	cv::Mat img_1 = GDAL2Mat(bzpoDataset,bzinfo,srcband);
	ImageInfo pzinfo;
	GDALDataset* pzpoDataset = GDALRead(pzFile, pzinfo);
	cout << "pz proj::::" << pzinfo.proj << endl;
	cv::Mat img_2 = GDAL2Mat(pzpoDataset, pzinfo, pzband);
	imshow("��һ��1", img_1);
	imshow("��һ��2", img_2);
	cout << "�ؼ�����ҿ�ʼ" << endl;
	vector<KeyPoint> keypoints_1,keypoints_2;
	DetectorKeyPoint(img_1, img_2, keypoints_1, keypoints_2);
	cout << "�����Ӽ��㿪ʼ" << endl;
	Mat descriptors_1, descriptors_2;
	ComputeDescriptor(img_1, img_2, keypoints_1, keypoints_2, descriptors_1, descriptors_2);
	cout << "ƥ����㿪ʼ" << endl;
	vector<vector<DMatch> > matchePoints;
	int size_ = FeatureMatch(matchePoints, descriptors_1, descriptors_2);
	vector<DMatch>_matches;
	//��ʼ
	for (int i = 0; i < matchePoints.size(); i++)
	{
		_matches.push_back(matchePoints[i][0]);
	}
	Mat img_matches_ini;
	drawMatches(img_1, keypoints_1, img_2, keypoints_2, _matches, img_matches_ini);
	//���NN/SCN
	vector<DMatch> matches_;
	matches_ = NNSCNCheck(matchePoints);
	Mat img_matches_;
	drawMatches( img_1, keypoints_1, img_2, keypoints_2, matches_, img_matches_);
	// RANSAC
	vector<KeyPoint>R_keypoint01;
	vector<KeyPoint>R_keypoint02; 
	vector<DMatch>R_matches;
	int size_2 = RANSACCheck(keypoints_1, keypoints_2, descriptors_1, descriptors_2, matches_, string("Homography"), R_keypoint01, R_keypoint02, R_matches);
	Mat img_matches;
	drawMatches(img_1, R_keypoint01, img_2, R_keypoint02, R_matches, img_matches);
	//������
	vector<KeyPoint>RR_keypoint01, RR_keypoint02;
	vector<DMatch> RR_matches;
	DistributedCheck(img_2, R_keypoint01, R_keypoint02, R_matches,RR_keypoint01, RR_keypoint02, RR_matches,20);
	Mat img_matches__;
	drawMatches(img_1, RR_keypoint01, img_2, RR_keypoint02, RR_matches, img_matches__);
	//imshow("��ʼmatchͼ", img_matches_ini);
	//imshow("NN/SCN", img_matches_);
	//imshow("RANSAC", img_matches);
	imshow("ƥ����", img_matches__);
	//imshow("����ͼ1", img_1);
	//imshow("����ͼ2", img_2);
	int count = RR_matches.size();
	GDAL_GCP gcplist[2000];
	CreateGCPsList(RR_keypoint01, RR_keypoint02, RR_matches, bzinfo, gcplist);
	const char * pszDstFile = "D:/test/OUT3.TIF";
	ImageWarpByGCP(pzFile,pszDstFile, RR_matches.size(),gcplist,(char*)bzinfo.proj,3,0,0,GRA_NearestNeighbour, "GTiff");
		waitKey(0);
}