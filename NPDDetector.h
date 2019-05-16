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

struct CBlock // ������
{
	int scale; // ����߶�
	Rect rect; // �������
	unsigned char *pData; //�������ص����Ͻ�ָ��
	//double score; // ������
};


// ����NPD�����ļ������
class CNPDDetector
{
public:
	CNPDDetector();
	~CNPDDetector();

	// ��ʼ��NPD�����
	// filename��ѵ���õļ��ģ���ļ���maxObjSize�����Ŀ���С��minObjSize����СĿ���С��
	// scaleFactor����߶ȼ��ĳ߶����ӣ�stepFactor�������������ӣ���������������ڼ�ⴰ��С�ı�����
	bool InitDetector(const char* filename, int height, int width, int minObjSize = 30, double scaleFactor = 1.2, double stepFactor = 0.1);

	// ��ɫͼ��ת�Ҷȣ�ע������ͼ����BGR˳��ģ�
	void BGR2Gray(Mat *I);

	// ��⺯���������ȵ���BGR2Gray��
	// ppImg������ͼ�񣬶�ά���飬��ppImg[i][j]��ʾ��i�е�j�����أ�height, width��ͼ��ߴ磻pResults: �������
	bool Detect(vector<Rect>& pResults);

protected:
	void InitLookupTable(); // ����NPD���ұ�
	bool InitBlocks(); // ��ʼ���������

protected:
	CNPDModel mNpdModel; // NPD���ģ��
	vector< vector< vector<unsigned char> > > mpppGrayTable; //�Ҷ�ͼ���ұ�
	unsigned char mppNPDLookupTable[256][256]; // NPD���ұ���Ҫ�Ӹ���������Ϊ8λ������
	unsigned char **mppImg; // �Ҷ�ͼ��洢�ռ�
	int mHeight; // �����ͼ��߶�
	int mWidth; // �����ͼ����
	CBlock *mpBlocks; // ����
	int mNumBlocks; // ������Ŀ
};

