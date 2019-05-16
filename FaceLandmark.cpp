// FaceLandmark.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "model.h"
#include "shapeRegression.h"

#if 0
int _tmain(int argc, _TCHAR* argv[])
{
	string imgname = "K:\\newProject\\RegressionFaceLandmark\\Landmark_DLL\\FaceLandmark\\FaceLandmark\\testImg\\Tony_Blair_0002.jpg";
	//string imgname = "K:\\newProject\\RegressionFaceLandmark\\Landmark_DLL\\FaceLandmark\\FaceLandmark\\testImg\\crop1.jpg";

	Mat img = imread(imgname,CV_LOAD_IMAGE_COLOR);
	Model model("model_small_14.dat");
	shapeXY tShapeXY;

	_bbox bbox;
	bbox.rootcoord[0] = 79;
	bbox.rootcoord[1] = 85;
	bbox.rootcoord[2] = 174;
	bbox.rootcoord[3] = 186;

// 	_bbox bbox;
// 	bbox.rootcoord[0] = 4;
// 	bbox.rootcoord[1] = 10;
// 	bbox.rootcoord[2] = 100;
// 	bbox.rootcoord[3] = 116;

#if MEASURETIME
	clock_t start,finish;
	double duration;
	start = clock();
#endif
	for (int i=0;i<_loopNum;i++)
	{
        shapeRegression(model, img,bbox,tShapeXY);
	}
#if MEASURETIME
	finish = clock();
	duration = (double)(finish-start)/CLOCKS_PER_SEC;
	printf("total time %f seconds\n",duration);
#endif
	namedWindow("landmark Localization", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("landmark Localization",100,100);
	for (int i=0;i<_pointNum;i++)
	{
		circle( img,Point(tShapeXY.shapeX[i],tShapeXY.shapeY[i]),2,Scalar(0,0,255),2,6);
	}
	imshow("landmark Localization",img);
	waitKey();
	return 0;
}
#endif