#include "stdafx.h"
//#include "model.h"
//#include "shapeRegression.h"
////#include "FaceDet.h"
//
//
//
//
//int main2( int argc, char** argv )
//{
////	Model model("landmark.dat");
//	Model model("D:\\Code\\FaceLankmark\\bin\\model_NormMeanShape_Full_32pts.dat");
//	shapeXY tShapeXY;
//	shapeXY tShapeXY2;
//
//	for(int i=0; i<475; i++)
//	{
//		cout<<i<<endl;
//		string image_name = "E:\\Code\\Images\\face_alignment\\output\\";
//		image_name = image_name + to_string(i+1) + ".jpg";
//		//Mat_<uchar> temp0 = imread(image_name,0);
//		Mat temp0 = imread(image_name,CV_LOAD_IMAGE_COLOR);
//
//		_bbox bbox;
//		if(i==0)
//		{
//			ifstream box_in;
//			//string temp1 = imgname.erase(imgname.size()-4, imgname.size()-1)+".rect";
//			string temp1 = image_name+".rect";
//			cout<<temp1<<endl;
//			box_in.open(temp1.data());
//			if(box_in.fail())
//				continue;
//			int x, y, w, h;
//			box_in>>x>>y>>w>>h;
//			bbox.rootcoord[0] = x;
//			bbox.rootcoord[1] = y+10;
//			bbox.rootcoord[2] = x+w;
//			bbox.rootcoord[3] = y+h;
//			DWORD start = GetTickCount();
//			float score = 0.0;
//			float roll, yaw, pitch;
//			shapeRegression(model, temp0, bbox,tShapeXY2,score, roll, yaw, pitch);
//			printf("landmark location time: %dms\n", GetTickCount() - start);
//			cout<<score<<endl;
//		}
//		else
//		{
//			int min_x = 9999.0;
//			int min_y = 9999.0;
//			int max_x = -9999.0;
//			int max_y = -9999.0;
//			for(int j=0; j<_pointNum; j++)
//			{
//				if(tShapeXY.shapeX[j]<min_x)
//					min_x = tShapeXY.shapeX[j];
//				if(tShapeXY.shapeX[j]>max_x)
//					max_x = tShapeXY.shapeX[j];
//				if(tShapeXY.shapeY[j]<min_y)
//					min_y = tShapeXY.shapeY[j];
//				if(tShapeXY.shapeY[j]>max_y)
//					max_y = tShapeXY.shapeY[j];
//			}
//			cout<<min_x<<" "<<min_y<<" "<<max_x<<" "<<max_y<<endl;
//			int center_x = (min_x + max_x)/2;
//			int height = max_y - min_y;
//			bbox.rootcoord[0] = center_x - height/2;
//			bbox.rootcoord[1] = min_y;
//			bbox.rootcoord[2] = center_x + height/2;
//			bbox.rootcoord[3] = max_y;
//			DWORD start = GetTickCount();
//			float score = 0.0;
//			float roll, yaw, pitch;
//			shapeRegression2(model, temp0, bbox,tShapeXY,tShapeXY2, score, roll, yaw, pitch);
//			printf("landmark location time: %dms\n", GetTickCount() - start);
//			cout<<score<<endl;
//		}
//
//		
//
//		for (int j=0;j<_pointNum;j++)
//		{
//			circle(temp0, Point(tShapeXY2.shapeX[j],tShapeXY2.shapeY[j]),1,Scalar(0,255,0), -1, 8);
//		}
//		tShapeXY = tShapeXY2;
//
//	//	rectangle(temp0,cv::Rect(x, y, w, h), cv::Scalar(255,0,0));  
//     //   imshow("result",temp0);
//	//	cvWaitKey(0); 
//
//		imwrite(image_name,temp0);
//
//	}
//
//	return 0;
//}