# SIFT Registration

## 基于OpenCV 和 GDAL 的遥感影像配准方法

1. GDAL 读取多波段遥感影像转换成OpenCV可以识别的三波段和单波段影像。
2. Opencv计算SIFT特征点和描述子。
3. 通过NN/SNN 初步筛选特征点。
4. *RANSAC*算法对初始关键点对进行精确匹配，剔除错误的匹配点。
5. 通过特定大小的滑动窗口选择特征点，使特征点均匀分布。
6. 将互相匹配的特征点转换成GCPList。
7. 使用GCPWarp对待配准遥感影像进行Warp，并输出遥感影像。