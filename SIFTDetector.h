#ifndef SIFTDETECTOR
#define SIFTDETECTOR

#include <opencv2/opencv.hpp>  
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/stitching.hpp>
using namespace cv;
using namespace std;

extern Ptr<Feature2D> detector;

bool DetectorKeyPoint(Mat img_1, Mat img_2, vector<KeyPoint>& keypoints_1, vector<KeyPoint>& keypoints_2);
bool ComputeDescriptor(Mat img_1, Mat img_2, vector<KeyPoint>& keypoints_1, vector<KeyPoint>& keypoints_2, Mat &descriptors_1, Mat &descriptors_2);
int FeatureMatch(vector<vector<DMatch> >& matchePoints, Mat& descriptors_1, Mat& descriptors_2);
vector<DMatch> NNSCNCheck(vector<vector<DMatch> > matchePoints,float threshold = 0.7);
int RANSACCheck( vector<KeyPoint> keypoints_1, vector<KeyPoint> keypoints_2, Mat descriptors_1, Mat descriptors_2, vector<DMatch> Matches, string model, vector<KeyPoint>& RR_keypoint01, vector<KeyPoint>& RR_keypoint02, vector<DMatch>& RR_matches, double param1 = 3., double param2 = 0.99);

int DistributedCheck(Mat img_2, vector<KeyPoint> keypoints_1, vector<KeyPoint> keypoints_2, vector<DMatch> Matches,vector<KeyPoint>& RR_keypoint01, vector<KeyPoint>& RR_keypoint02, vector<DMatch>& RR_matches, int distance = 5);

Mat WarpImage(Mat img_1, Mat img_2, vector<KeyPoint> keypoints_1, vector<KeyPoint> keypoints_2, vector<DMatch> Matches);
#endif