#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;



void ImageProcessing(Mat& thisFrame, Mat& mask, float volume_weight, float CSA,float& this_weight,float& sum_weight,float coeff);
