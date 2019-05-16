// Class for the NPD detector
// 
// Shengcai Liao
// National Laboratory of Pattern Recognition
// Institute of Automation, Chinese Academy of Sciences
// scliao@nlpr.ia.ac.cn
//
#pragma once
#include "stdafx.h"
#include "NPDModel.h"
#include <opencv2/objdetect/objdetect.hpp>
#include <vector>

using namespace cv;
using namespace std;

struct CBlock // 检测框类
{
	int scale; // 检测框尺度
	Rect rect; // 检测框参数
	unsigned char *pData; //检测框像素的左上角指针
	//double score; // 检测分数
};


// 基于NPD特征的检测器类
class CNPDDetector
{
public:
	CNPDDetector();
	~CNPDDetector();

	// 初始化NPD检测器
	// filename：训练好的检测模型文件；maxObjSize：最大目标大小；minObjSize：最小目标大小；
	// scaleFactor：多尺度检测的尺度因子；stepFactor：滑动步长因子，即滑动步长相对于检测窗大小的比例。
	bool InitDetector(const char* filename, int height, int width, int minObjSize = 30, double scaleFactor = 1.2, double stepFactor = 0.1);

	// 彩色图像转灰度（注意输入图像是BGR顺序的）
	void BGR2Gray(Mat *I);

	// 检测函数（必须先调用BGR2Gray）
	// ppImg：输入图像，二维数组，即ppImg[i][j]表示第i行第j列像素；height, width：图像尺寸；pResults: 检测结果。
	bool Detect(vector<Rect>& pResults);

protected:
	void InitLookupTable(); // 计算NPD查找表
	bool InitBlocks(); // 初始化检测框参数

protected:
	CNPDModel mNpdModel; // NPD检测模型
	vector< vector< vector<unsigned char> > > mpppGrayTable; //灰度图查找表
	unsigned char mppNPDLookupTable[256][256]; // NPD查找表（需要从浮点数量化为8位整数）
	unsigned char **mppImg; // 灰度图像存储空间
	int mHeight; // 待检测图像高度
	int mWidth; // 待检测图像宽度
	CBlock *mpBlocks; // 检测框
	int mNumBlocks; // 检测框数目
};

