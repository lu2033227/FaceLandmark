////#include <iostream>
////#include "LivenessDetector.h"
////#include <opencv2/opencv.hpp>
////#include <string>
////#include <fstream>
////#include "AIMG.h"
////
////using namespace std;
////
////
////int main()
////{
////	string modelFoldePath = "model";
////
////	CoInitializeEx(NULL, COINIT_MULTITHREADED);
////
////	// initialize face engine
////	char wPath[MAX_PATH];
////	char apiPath[MAX_PATH];
////	GetWindowsDirectory(wPath, MAX_PATH);
////	sprintf_s(apiPath, MAX_PATH, "%s\\Help\\%s\\jet9.chi", wPath, AIM_VERSION);
////	HMODULE app = LoadAPI(apiPath);
////
////	// face detector
////
////	HAIM hAIM = IAimCreate();
////	cout << "hAIM:" << hAIM << endl;
////	// set params
////	// min eye, max eye, roll, yaw, pitch, conf, quality, sort type
////	TFaceParams params = { 30.0f, 10000.0f, 0, 90, 90, 0.2f, 100, 1, COMPACT_FEATURE };
////	IAimSetParams(hAIM, &params);
////
////
////	LivenessDetector livenessDetector = CreateLivenessDetector(modelFoldePath.c_str(), 0.0, 0.0, 0.0, 0.0);
////
////	for (int i = 0; i < 2; i++)
////	{
////
////		cv::Mat img = cv::imread("E:\\Training_Samples\\Training_vdieos\\picture\\1\\ground_new.bmp");
////
////		//cv::VideoCapture cap("E:\\Training_Samples\\Training_vdieos\\real\\1\\1_ground1.avi");
////		cv::VideoCapture cap("E:\\Training_Samples\\Training_vdieos\\hu\\20151012143517.avi");
////		//cv::VideoCapture cap("E:\\Training_Samples\\视频数据\\负样本\\20151010105230.avi");
////		int start = 3;
////		int stride = 6;
////		int n_pics = 4;
////
////		//cv::Mat *images = new cv::Mat[n_pics + 1];
////
////		unsigned char **images = new unsigned char*[n_pics + 1];
////		unsigned char **inner_images = new unsigned char*[n_pics + 1];
////
////		cv::Mat frame;
////
////		cap.set(CV_CAP_PROP_POS_FRAMES, start);
////
////		long totalFrameNumber = cap.get(CV_CAP_PROP_FRAME_COUNT);
////		cout << "整个视频共" << totalFrameNumber << "帧" << endl;
////
////
////		cap >> frame;
////
////		//cv::Mat dst1;
////		//cv::resize(frame, dst1, cv::Size(256, 256));
////		//frame = cv::imread("E:\\Training_Samples\\Training_vdieos\\train_slt_data\\train_face_sherlet\\1_ground.avi_1.real.jpg");
////		//frame = cv::imread("E:\\Training_Samples\\Training_vdieos\\picture\\1\\");
////		//frame = cv::imread("0sub.bmp");
////		//int height1 = dst1.rows;
////		//int pitch1 = dst1.step.p[0];
////		//inner_images[0] = new unsigned char[height1 * pitch1];
////		//memcpy(inner_images[0], dst1.data, height1 * pitch1);
////
////
////		for (int i = 0; i < n_pics + 1; i++)
////		{
////			for (int k = 0; k < stride; k++)
////			{
////				cap >> frame;
////			}
////
////			cv::Mat dst0;
////			cv::resize(frame, dst0, cv::Size(256, 256));
////			int height0 = dst0.rows;
////			int pitch0 = dst0.step.p[0];
////
////			images[i] = new unsigned char[height0 * pitch0];
////			memcpy(images[i], dst0.data, height0 * pitch0);
////
////			TFaceDetectionResult templates2[1];
////			int count2 = IAimDetect(hAIM, (BYTE*)frame.data,
////				frame.cols, frame.rows, frame.step.p[0],
////				24, 1, templates2);
////
////			cout << "faceCount:" << count2 << endl;
////
////			int left = templates2[0].rtFace.left;
////			int top = templates2[0].rtFace.top;
////			int right = templates2[0].rtFace.right;
////			int bottom = templates2[0].rtFace.bottom;
////
////			int w = right - bottom;
////			int h = bottom - top;
////
////			right = ((right + w / 10) < frame.cols) ? right + w / 10 : frame.cols;
////			left = ((left - w / 10) > 0) ? left - w / 10 : 0;
////			top = ((top - h / 10) > 0) ? top - h / 10 : 0;
////			bottom = ((bottom + h / 10) < frame.rows) ? bottom + h / 10 : frame.rows;
////
////			cv::Mat face = frame(cv::Range(top, bottom), cv::Range(left, right));
////			cv::Mat dst;
////			cv::resize(face, dst, cv::Size(256, 256));
////
////			if (i == 0)
////			{
////				dst = cv::imread("1111.jpg");
////				//cv::imwrite("1111.jpg", dst);
////			}
////
////			height0 = dst.rows;
////			pitch0 = dst.step.p[0];
////
////			inner_images[i] = new unsigned char[height0 * pitch0];
////			memcpy(inner_images[i], dst.data, height0 * pitch0);
////			//images[i + 1] = frame.clone();
////		}
////
////		float score1 = 0;
////		float score2 = 0;
////		float score3 = 0;
////		float score4 = 0;
////		float T1 = 0;
////		float T2 = 0;
////		float T3 = 0;
////		float T4 = 0.9;
////
////		float x = LivenessScore(livenessDetector, inner_images, images, 256, 256, 5,
////			score1, score2, score3, score4);
////		//bool x = isLive(livenessDetector, images, images, width, height, width, height, n_pics + 1);
////		cout << score1 << endl;
////	}
////	getchar();
////	return 0;
////}
////
////
////#include <iostream>
////#include <stdio.h>
////#include <stdlib.h>
////#include <atlimage.h>
////#include "LivenessDetector.h"
////#include <opencv2/opencv.hpp>
////#include <string>
////#include <fstream>
////#include <algorithm>
////#include "AIMG.h"
////#include <opencv2/highgui/highgui.hpp>
////
////using namespace cv;
////
////using namespace std;
////
////
////int main()
////{
////	string modelFoldePath = "D:\\AuthenMetric\\anti_spoofing3.0\\Test_MLP\\Release";
////	int top = 0, left = 0, bottom = 0, right = 0;
////
////	CoInitializeEx(NULL, COINIT_MULTITHREADED);
////
////	 initialize face engine
////	char wPath[MAX_PATH];
////	char apiPath[MAX_PATH];
////	GetWindowsDirectory(wPath, MAX_PATH);
////	sprintf_s(apiPath, MAX_PATH, "%s\\Help\\%s\\jet9.chi", wPath, AIM_VERSION);
////	HMODULE app = LoadAPI(apiPath);
////
////	 face detector
////
////	HAIM hAIM = IAimCreate();
////	cout << "hAIM:" << hAIM << endl;
////	 set params
////	 min eye, max eye, roll, yaw, pitch, conf, quality, sort type
////	TFaceParams params = { 30.0f, 10000.0f, 0, 90, 90, 0.2f, 100, 1, COMPACT_FEATURE };
////	IAimSetParams(hAIM, &params);
////
////
////	LivenessDetector livenessDetector = CreateLivenessDetector(modelFoldePath.c_str(), 0.0, 0.0, 0.0, 0.0);
////
////
////	cv::VideoCapture cap("E:\\Training_Samples\\Training_vdieos\\real\\1\\1_ground1.avi");
////	cv::VideoCapture cap(0);
////	cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
////	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
////	确认是否成功打开摄像头
////	if (!cap.isOpened()){
////		cout << "打开摄像头失败，退出";
////		exit(-1);
////	}
////	namedWindow("Capture", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
////
////	cv::VideoCapture cap("E:\\Training_Samples\\视频数据\\负样本\\20151010105230.avi");
////	int start = 1;
////	int stride = 6;
////	int n_pics = 4;
////
////	cv::Mat *images = new cv::Mat[n_pics + 1];
////
////	unsigned char **images = new unsigned char*[n_pics + 1];
////	unsigned char **inner_images = new unsigned char*[n_pics + 1];
////
////	unsigned char* hscImage = new unsigned char[128 * 128 * 3];
////	unsigned char *inner_images2 = new unsigned char[(n_pics + 1) * 256 * 256 * 3];
////	unsigned char *outer_images2 = new unsigned char[(n_pics + 1) * 256 * 256 * 3];
////	cv::Mat* img = new cv::Mat[1];
////	unsigned char *inner_images2 = new unsigned char[(n_pics + 1) * 256 * 256 * 3];
////
////	unsigned char* ptr1 = inner_images2;
////	unsigned char* ptr2 = outer_images2;
////
////	cv::Mat frame;
////
////	cap.set(CV_CAP_PROP_POS_FRAMES, start);
////
////	long totalFrameNumber = cap.get(CV_CAP_PROP_FRAME_COUNT);
////	cout << "整个视频共" << totalFrameNumber << "帧" << endl;
////
////	
////	cv::Mat dst1;
////	cv::resize(frame, dst1, cv::Size(256, 256));
////	frame = cv::imread("E:\\Training_Samples\\Training_vdieos\\train_slt_data\\train_face_sherlet\\1_ground.avi_1.real.jpg");
////	frame = cv::imread("E:\\Training_Samples\\Training_vdieos\\picture\\1\\");
////	frame = cv::imread("0sub.bmp");
////	int height1 = dst1.rows;
////	int pitch1 = dst1.step.p[0];
////	inner_images[0] = new unsigned char[height1 * pitch1];
////	memcpy(inner_images[0], dst1.data, height1 * pitch1);
////
////	while (1){
////		cap >> frame;
////		imshow("Capture", frame);
////
////		for (int i = 0; i < n_pics + 1; i++)
////		{
////			for (int k = 0; k < stride; k++)
////			{
////				cap >> frame;
////				
////			}
////
////			if (i == 0){
////
////				img[0] = frame.clone();
////
////				TFaceDetectionResult templates2[1];
////				int count = IAimDetect(hAIM, (BYTE*)img[0].data,
////					img[0].cols, img[0].rows, img[0].step.p[0],
////					24, 1, templates2);
////				int eyeleft_x = templates2[0].ptLeftEye.x;
////				int eyeleft_y = templates2[0].ptLeftEye.y;
////				int eyeright_x = templates2[0].ptRightEye.x;
////				int eyeright_y = templates2[0].ptRightEye.y;
////				int eyeWidth = eyeright_x - eyeleft_x;
////
////				int canshu = 128;
////
////				float scale = 72.0 / (float)eyeWidth;
////				cout << scale << endl;
////
////				float cols = img[0].cols * scale;
////				float rows = img[0].rows * scale;
////
////				cv::Mat scaledSource;
////				cv::resize(img[0].clone(), scaledSource, cv::Size(cols, rows));
////
////				left = ((eyeleft_x + eyeright_x) * scale / 2 - canshu / 2) > 0 ? ((eyeleft_x + eyeright_x) * scale / 2 - canshu / 2) : 0;
////				right = ((eyeleft_x + eyeright_x) * scale / 2 + canshu / 2) < cols ? ((eyeleft_x + eyeright_x) * scale / 2 + canshu / 2) : cols;
////				top = ((eyeleft_y + eyeright_y) * scale / 2 - canshu / 4) > 0 ? ((eyeleft_y + eyeright_y) * scale / 2 - canshu / 4) : 0;
////				bottom = ((eyeleft_y + eyeright_y) * scale / 2 + canshu * 3 / 4) < rows ? ((eyeleft_y + eyeright_y) * scale / 2 + canshu * 3 / 4) : rows;
////
////				cv::Mat faceHSCFrame = scaledSource.clone();
////				cv::Mat faceHSC = faceHSCFrame(cv::Range(top, bottom), cv::Range(left, right));
////				cv::Mat dstHSC;
////				cv::resize(faceHSC, dstHSC, cv::Size(128, 128));
////
////				cv::imwrite("face_hsc.bmp", dstHSC.clone());
////
////				memcpy(hscImage, dstHSC.data, dstHSC.rows * dstHSC.cols * 3);
////
////
////
////				ofm face kuang
////				count = IAimDetect(hAIM, (BYTE*)frame.data,
////					frame.cols, frame.rows, frame.step.p[0],
////					24, 1, templates2);
////
////
////				cout << "faceCount:" << count << endl;
////
////				if (count <= 0) {
////					cout << "no face found:" << endl;
////					break;
////				}
////
////				left = templates2[0].rtFace.left;
////				top = templates2[0].rtFace.top;
////				right = templates2[0].rtFace.right;
////				bottom = templates2[0].rtFace.bottom;
////
////				int w = right - left;
////				int h = bottom - top;
////
////				right = ((right + w / 4) < frame.cols) ? right + w / 4 : frame.cols;
////				left = ((left - w / 4) > 0) ? left - w / 4 : 0;
////				top = ((top - h / 4) > 0) ? top - h / 4 : 0;
////				bottom = ((bottom + h / 4) < frame.rows) ? bottom + h / 4 : frame.rows;
////
////				cout << "*********************************" << endl;
////				cout << right << "  " << left << "  " << top << "  " << bottom << endl;
////				cout << frame.rows << "   " << frame.cols << endl;
////				cout << "*********************************" << endl;
////			}
////
////			cv::Mat faceFrame = frame.clone();
////
////			cv::Mat face = faceFrame(cv::Range(top, bottom), cv::Range(left, right));
////			cv::Mat dst;
////			cv::resize(face, dst, cv::Size(256, 256));
////			int height1 = dst.rows;
////			int pitch1 = dst.step.p[0];
////
////			cv::Mat dst0;
////			cv::resize(frame, dst0, cv::Size(256, 256));
////			int height0 = dst0.rows;
////			int pitch0 = dst0.step.p[0];
////
////
////			memcpy(ptr2, dst.data, 256 * 256 * 3);
////			ptr2 += 256 * 256 * 3;
////			memcpy(ptr1, dst0.data, 256 * 256 * 3);
////			ptr1 += 256 * 256 * 3;
////			images[i + 1] = frame.clone();
////		}
////
////		float score1 = 0;
////		float score2 = 0;
////		float score3 = 0;
////		float score4 = 0;
////		float T1 = 0;
////		float T2 = 0;
////		float T3 = 0;
////		float T4 = 0.9;
////
////
////
////		float x = LivenessScore(livenessDetector, hscImage, inner_images2, outer_images2, 128, 128, 256, 256, 5,
////			score1, score2, score3, score4);
////		bool x = isLive(livenessDetector, images, images, width, height, width, height, n_pics + 1);
////		cout << "score1:" << score1 << endl;
////		cout << "score3:" << score3 << endl;
////		cout << "score4:" << score4 << endl;
////
////		delete[] hscImage;
////		delete[] img;
////		delete[] inner_images2;
////		delete[] outer_images2;
////	}
////	
////
////
////	destroyWindow("Capture");
////	getchar();
////	return 0;
////}
////
