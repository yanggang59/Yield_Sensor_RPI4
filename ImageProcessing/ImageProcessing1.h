#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;



void ImageProcessing(Mat& thisFrame, Mat& mask, float volume_weight, float CSA,float& sum_weight);
