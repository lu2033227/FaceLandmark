#include "stdafx.h"
#include "model.h"
#include "shapeRegression.h"
#include "FaceDet.h"

#if DETECTMode

int main( int argc, char** argv )
{
	Model model("model_small_14.dat");
	shapeXY tShapeXY;

	Mat im=imread("K:\\newProject\\FaceDetection_DLL\\Experiments_CVPR2014\\testimages\\2751381965.jpg",CV_LOAD_IMAGE_COLOR);
	Mat frame;
	TFaceParams tFaceParams;
	tFaceParams.thresh = 0.0;
	tFaceParams.interval = 5;
	tFaceParams.padx = 0;
	tFaceParams.pady = 0;
	strcpy(tFaceParams.modelfile1,"Face_YAW_0_15_ROLL_0_30_PITCH_0_30_11_5.dat");
	strcpy(tFaceParams.modelfile2,"pcamatrix_new.dat");

	HFaceDetector tHFaceDetector = FD_Create(tFaceParams);
	TFaces tFaces;

	_mtimes mtimes_1;
	QueryPerformanceCounter(&mtimes_1.start);	
	tFaces = FD_Process(tHFaceDetector,im.data, im.cols, im.rows,im.step.p[0],24,1.0f);
	QueryPerformanceCounter(&mtimes_1.end); 
	cout<<"time of detection: "<<mtimes_1.calcTime() <<"ms"<<endl;
	
	vector<shapeXY> shapeXYs;
	for (int i=0;i<tFaces._count;i++)
	{
		_bbox bbox;
		bbox.rootcoord[0] = tFaces._pos[i].left;
		bbox.rootcoord[1] = tFaces._pos[i].top;
		bbox.rootcoord[2] = tFaces._pos[i].right;
		bbox.rootcoord[3] = tFaces._pos[i].bottom;

		shapeXY tShapeXY;
		shapeRegression(model, im,bbox,tShapeXY);
		shapeXYs.push_back(tShapeXY);
	}

	for (int i=0;i<tFaces._count;i++)
	{
		rectangle(im, Point(tFaces._pos[i].left,tFaces._pos[i].top), Point(tFaces._pos[i].right,tFaces._pos[i].bottom),cvScalar(0,0,255),2,8,0);
		
		for (int j=0;j<_pointNum;j++)
		{
			circle(im,Point(shapeXYs[i].shapeX[j],shapeXYs[i].shapeY[j]),2,Scalar(0,0,255),2,6);
		}
	}
	cout<<tFaces._count<<endl;
	
	Mat im2;
	resize(im,im2,Size(),0.5,0.5,INTER_LINEAR);
	im = im2;
	namedWindow("detected_image", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("detected_image",50,50);
	imshow("detected_image",im);
	waitKey();
	FD_Free(tHFaceDetector);
	return 0;
}
#endif