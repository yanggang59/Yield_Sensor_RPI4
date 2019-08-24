#include "ImageProcessing.h"

/**
*?????
*     Zc = 32*fx/(u-cx)
*     Xc = (u-cx)*Zc/fx
*     Yc = (v-cy)*Zc/fy = 32*fx*(v-cy)/(fy*(u-cx))
*     ???y??????? ??????????????????
*/


/**??锟斤拷????
*/
void ImageProcessing(Mat& thisFrame, Mat& mask, float volume_weight, float CSA,float& sum_weight)
{
	float fx = 1671.39;
	float fy = 1718.28;
	float cx = 783.30;
	float cy = 610.64;

	bitwise_and(thisFrame, mask, thisFrame);

	threshold(thisFrame, thisFrame, 160, 255, CV_THRESH_BINARY);


	/**????????????????????
	* A:(296,936)(474,1025)
	* B:(474,1025)(600,1168)
	* 
	*/
	bool inAreaA = false,findPeakInA = false; //A??????????????
	bool inAreaB = false,findPeakInB = false; //B??????????????

	Point peak;  //??????????
	Point3f real_peak; //????????????????????????

	Point bottom;//???????
	Point3f real_bottom; //????????????????????????

	bool isVoid = false;//????????锟斤拷

	//thisFrame???A????
	Mat AreaA(thisFrame, Rect(936, 296, 89, 278));

	//???A????????????
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(AreaA, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point());
	if (contours.size()  > 2)
		inAreaA = true;
	/*??A????????锟斤拷?
	*/
	if (inAreaA)
	{
		for (int i = 296; i < 474; i++) //i?????锟斤拷??????V
		{
			if (findPeakInA) break;
			for (int j = 936; j < 1025; j++)//j?????锟斤拷??????U
			{
				if (thisFrame.at<uchar>(i, j)>0)
				{
					findPeakInA = true;
					cout << "In A area;" << endl;
					peak.x = j;
					peak.y = i;
					cout <<"Peak: "<< j << " , " << i << endl;
					break;
				}
			}
		}
	}




	/*???????A??????????B????
	*/
	if (!findPeakInA)
	{
		//thisFrame???B????
		Mat AreaB(thisFrame, Rect(1025, 474, 126, 143));
		findContours(AreaB, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point());

		if (contours.size() > 2)
			inAreaB = true;
		for (int i = 474; i < 600; i++)
		{
			if (findPeakInB) break;
			for (int j = 1025; j < 1168; j++)
			{
				if (thisFrame.at<uchar>(i, j)>0)
				{
					findPeakInB = true;
					cout << "In B area;" << endl;
					peak.x = j;
					peak.y = i;
					cout << j << " , " << i << endl;
					break;
				}
			}
		}
	}
	
	

	/*???????A?????B????
	*/
	if (!findPeakInA && !findPeakInB)
	{
		isVoid = true;

		cout << "Void Scrapper" << endl;
	}
	else
	{

		real_peak.z = 32 * fx / (peak.x - cx);
		real_peak.x = (peak.x - cx)*real_peak.z / fx;
		real_peak.y = 32 * fx*(peak.y - cy) / (fy*(peak.x - cx));
	}

	//?????锟絀???,??????????
	bool findBottom = false;
	if (!isVoid)
	{
		for (int i = 880; i >= 640; i--)
		{
			if (findBottom) break;
			for (int j = 1168; j > 1130; j--)
			{
				if (thisFrame.at<uchar>(i, j)>0)
				{
					findBottom = true;
					cout << "bottom find !!!" << endl;
					bottom.x = j;
					bottom.y = i;
					cout << j << " , " << i << endl;
					break;
				}
			}
		}
	}
	//??锟斤拷???锟絀???????????????????????
	if (!isVoid && findBottom)
	{
		real_bottom.z = 32 * fx / (bottom.x - cx);
		//real_bottom.x = (bottom.x - cx)*real_bottom.z / fx;
		real_bottom.y = 32 * fx*(bottom.y - cy) / (fy*(bottom.x - cx));
		//???y??????????????????????????,??锟斤拷??mm
		float distance = -(real_peak.y - real_bottom.y);
		if (distance > 10) distance -= 10.0;
		else distance = 0.0;
		float this_weight = CSA * distance / 10 * volume_weight;
		sum_weight += this_weight;
		cout << "weight is " << sum_weight / 1000 << "  kg" << endl;
	}

}
