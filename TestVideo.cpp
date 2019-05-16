#include <iostream>
#include <stdio.h>
#include <stdlib.h>
//#include <atlimage.h>
#include "LivenessDetector.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <fstream>
#include <algorithm>
#include "AIMG.h"
#include <opencv2/highgui/highgui.hpp>
#include "Timer.h"
#include "imtransform.h"
#include "NPDDetector.h"
#include "model.h"
#include "shapeRegression.h"
#include <thread>   // std::thread
#include <vector>

using namespace cv;

using namespace std;

//void CALLBACK TimerProc(VideoCapture cap)
//{
//	Mat frame;
//	cap >> frame;
//	imshow("Capture", frame);
//}

struct FaceLandMarksOffsets
{
	float    left;
	float	 top;
	float	 right;
	float	 bottom;
	float	 score;
	float	 roll;
	float	 yaw;
	float	 pitch;
	float    landmarks[64];
} gFaceLandMarksOffsets;



void thread_task(LivenessDetector livenessDetector, unsigned char *inner_images2, unsigned char *outer_images2, vector<float> vecScore, vector<float> vecParam) {
	cout << "hello thread" << endl;

	boolean isConfidence = true, isActionDone = true, isOFMallowed = false;
	/*for (int i = 0; i < vecScore.size(); i++) {
		float score = vecScore[i];
		if (score >= 15) {
			isConfidence = false;
			vecScore.clear();
			cout << "score:" << score << endl;
		}	
	}

	for (int i = 0; i < vecParam.size(); i++) {
		float param = vecParam[i];
		if (param >= 21) {
			isActionDone = true;
			cout << "pitch:" << param << endl;
		}

	}*/

	float x = LivenessOFMScore(livenessDetector, inner_images2, outer_images2, 256, 256, 5);

	if (x >= 0.5) {
		isOFMallowed = true;
	}
	cout << "ofm result:" << x << endl;

	if (isConfidence && isActionDone && isOFMallowed) {
		cout << "spoofing result: real" << endl;
		vecScore.clear();
		vecParam.clear();
	}
	else {cout << "spoofing result: attack" << endl;}


	delete[] inner_images2;
	delete[] outer_images2;
}


int main()
{

	namedWindow("Capture", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::VideoCapture cap(0);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	//确认是否成功打开摄像头
	if (!cap.isOpened()) {
		cout << "打开摄像头失败，退出";
		exit(-1);
	}

	string modelFoldePath = "C:\\Users\\lulu\\Desktop\\PCAntispoofing\\Release";
	int top = 0, left = 0, bottom = 0, right = 0;
	float leftX = 0, leftY = 0, rightX = 0, rightY = 0;

	Model model("C:\\Users\\lulu\\Desktop\\PCAntispoofing\\Release\\model_32pts_score_fast.dat");
	string modelFile = "C:\\Users\\lulu\\Desktop\\PCAntispoofing\\Release\\npd_model_fro.dat";

	int numImgs = 1;
	int minObjSize = 80;
	double scaleFactor = 1.2;
	double stepFactor = 0.04;
	bool b_detect = true;
	_bbox bbox;
	shapeXY tShapeXY;
	shapeXY tShapeXY2;

	_bbox bbox1;
	shapeXY tShapeXY1;
	shapeXY tShapeXY3;


	vector<float> vecScore;
	vector<float> vecParam;


	//CoInitializeEx(NULL, COINIT_MULTITHREADED);

	//// initialize face engine
	//char wPath[MAX_PATH];
	//char apiPath[MAX_PATH];
	//GetWindowsDirectory(wPath, MAX_PATH);
	//sprintf_s(apiPath, MAX_PATH, "%s\\Help\\%s\\jet9.chi", wPath, AIM_VERSION);
	//HMODULE app = LoadAPI(apiPath);

	//// face detector

	//HAIM hAIM = IAimCreate();
	//cout << "hAIM:" << hAIM << endl;
	//// set params
	//// min eye, max eye, roll, yaw, pitch, conf, quality, sort type
	//TFaceParams params = { 30.0f, 10000.0f, 0, 90, 90, 0.2f, 100, 1, COMPACT_FEATURE };
	//IAimSetParams(hAIM, &params);

	LivenessDetector livenessDetector = CreateLivenessDetector(modelFoldePath.c_str(), 0.0, 0.0, 0.0, 0.0);
	int start = 1;
	int stride = 6;
	int n_pics = 4;



	/*cv::VideoCapture cap(0);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);*/
	//确认是否成功打开摄像头
	if (!cap.isOpened()) {
		cout << "打开摄像头失败，退出";
		exit(-1);
	}

	Mat img;
	cap >> img;
	int height = img.rows;
	int width = img.cols;
	cout << height << " " << width << endl;
	CNPDDetector detector;
	if (!detector.InitDetector(modelFile.data(), height, width, minObjSize, scaleFactor, stepFactor))
	{
		cout << "Failed to initiate the NPD detector: " << modelFile << endl;
		return 0;
	}


	while (1) {



		//Mat frame;
		//cap >> frame;

		//imshow("Capture", frame);

		//unsigned char* hscImage = new unsigned char[128 * 128 * 3];
		unsigned char *inner_images2 = new unsigned char[(n_pics + 1) * 256 * 256 * 3];
		unsigned char *outer_images2 = new unsigned char[(n_pics + 1) * 256 * 256 * 3];
		cv::Mat* img = new cv::Mat[1];
		//unsigned char *inner_images2 = new unsigned char[(n_pics + 1) * 256 * 256 * 3];

		unsigned char* ptr1 = inner_images2;
		unsigned char* ptr2 = outer_images2;
		Mat frame;
		cap >> frame;



		for (int i = 0; i < n_pics + 1; i++)
		{
			for (int k = 0; k < stride; k++)
			{
				Mat frame0;
				cap >> frame;
				frame0 = frame.clone();
				//cv::waitKey(30);


				clock_t begin, finish, procTime = 0;

				begin = clock();
				finish = clock();

				float score = 0.0;
				float roll, yaw, pitch;
				if (b_detect)
				{
					detector.BGR2Gray(&frame0);
					vector<Rect> pResults1;
					for (int i = 0; i < numImgs; i++)
					{
						pResults1.clear();
						detector.Detect(pResults1);
					}
					if (pResults1.size() == 0)
						continue;

					bbox1.rootcoord[0] = pResults1[0].x;
					bbox1.rootcoord[1] = pResults1[0].y;
					bbox1.rootcoord[2] = pResults1[0].x + pResults1[0].width;
					bbox1.rootcoord[3] = pResults1[0].y + pResults1[0].height*1.0;
					b_detect = false;
					DWORD start = GetTickCount();
					shapeRegression(model, frame0, bbox1, tShapeXY1, score, roll, yaw, pitch);
					/*if (abs(score) > 15)
					{
						b_detect = true;
					}*/
					//printf("landmark location time: %dms\n", GetTickCount() - start);
					//cout << score << endl;
					//cout << "roll: " << roll << " yaw: " << yaw << " pitch: "  << pitch << endl;
				}
				else
				{
					int min_x = 9999.0;
					int min_y = 9999.0;
					int max_x = -9999.0;
					int max_y = -9999.0;
					for (int j = 0; j < _pointNum; j++)
					{
						if (tShapeXY1.shapeX[j] < min_x)
							min_x = tShapeXY1.shapeX[j];
						if (tShapeXY1.shapeX[j] > max_x)
							max_x = tShapeXY1.shapeX[j];
						if (tShapeXY1.shapeY[j] < min_y)
							min_y = tShapeXY1.shapeY[j];
						if (tShapeXY1.shapeY[j] > max_y)
							max_y = tShapeXY1.shapeY[j];
					}
					int center_x = (min_x + max_x) / 2;
					int height = max_y - min_y;
					bbox1.rootcoord[0] = center_x - height / 2;
					bbox1.rootcoord[1] = min_y;
					bbox1.rootcoord[2] = center_x + height / 2;
					bbox1.rootcoord[3] = max_y;
					DWORD start = GetTickCount();
					shapeRegression2(model, frame0, bbox1, tShapeXY3, tShapeXY1, score, roll, yaw, pitch);
					if (abs(score) > 19 || abs(score) < 3)
					{
						b_detect = true;
					}
					//printf("landmark location time: %dms\n", GetTickCount() - start);
					//cout << score << endl;
					//cout << "roll: " << roll << " yaw: " << yaw << " pitch: " << pitch << endl;
				}

				tShapeXY3 = tShapeXY1;

				vecScore.push_back(score);
				vecParam.push_back(pitch);

				rectangle(frame0, Point(bbox1.rootcoord[0], bbox1.rootcoord[1]), Point(bbox1.rootcoord[2], bbox1.rootcoord[3]), cvScalar(0, 0, 255), 2, 8, 0);
				for (int j = 0; j < _pointNum; j++)
				{
					circle(frame0, Point(tShapeXY1.shapeX[j], tShapeXY1.shapeY[j]), 1, Scalar(0, 255, 0), -1, 8);
				}
				imshow("Capture", frame0);
				cv::waitKey(30);
			}

			if (i == 0) {

				img[0] = frame.clone();

				detector.BGR2Gray(&img[0]);
				vector<Rect> pResults;
				for (int i = 0; i < numImgs; i++)
				{
					pResults.clear();
					detector.Detect(pResults);
				}
				if (pResults.size() == 0) {
					cout << "no face found:" << endl;
					break;
				}

				bbox.rootcoord[0] = pResults[0].x;
				bbox.rootcoord[1] = pResults[0].y;
				bbox.rootcoord[2] = pResults[0].x + pResults[0].width;
				bbox.rootcoord[3] = pResults[0].y + pResults[0].height*1.0;
				DWORD start = GetTickCount();
				float score = 0.0;
				float roll, yaw, pitch;
				shapeRegression(model, img[0], bbox, tShapeXY2, score, roll, yaw, pitch);

				leftX = (tShapeXY2.shapeX[7] + tShapeXY2.shapeX[8] + tShapeXY2.shapeX[9] + tShapeXY2.shapeX[10]) / 4;
				leftY = (tShapeXY2.shapeY[7] + tShapeXY2.shapeY[8] + tShapeXY2.shapeY[9] + tShapeXY2.shapeY[10]) / 4;
				rightX = (tShapeXY2.shapeX[11] + tShapeXY2.shapeX[12] + tShapeXY2.shapeX[13] + tShapeXY2.shapeX[14]) / 4;
				rightY = (tShapeXY2.shapeY[11] + tShapeXY2.shapeY[12] + tShapeXY2.shapeY[13] + tShapeXY2.shapeY[14]) / 4;
			}

			cv::Mat img_transform;
			img_transform = sim_transform_image_3channels(frame,
				leftX,
				leftY,
				rightX,
				rightY);

			cv::Mat dst = img_transform.clone();
			int height1 = dst.rows;
			int pitch1 = dst.step.p[0];

			cv::Mat dst0;
			cv::resize(frame, dst0, cv::Size(256, 256));
			int height0 = dst0.rows;
			int pitch0 = dst0.step.p[0];


			memcpy(ptr2, dst.data, 256 * 256 * 3);
			ptr2 += 256 * 256 * 3;
			memcpy(ptr1, dst0.data, 256 * 256 * 3);
			ptr1 += 256 * 256 * 3;
			//images[i + 1] = frame.clone();
		}

		std::thread t(thread_task, livenessDetector, inner_images2, outer_images2, vecScore, vecParam);
		t.join();

		delete[] img;

	}

	//destroyWindow("Capture");
	getchar();
	return 0;
}
