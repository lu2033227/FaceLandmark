// Class for the NPD detection model
// 
// Shengcai Liao
// National Laboratory of Pattern Recognition
// Institute of Automation, Chinese Academy of Sciences
// scliao@nlpr.ia.ac.cn
//
#pragma once
#include "stdafx.h"

#define QUAN_LEVELS 32760

struct CNPDFeature // NPD����������
{
	short int x1; // ����NPD�����ĵ�һ���ο����x����λ��
	short int y1; // ����NPD�����ĵ�һ���ο����y����λ��
	short int x2; // ����NPD�����ĵڶ����ο����x����λ��
	short int y2; // ����NPD�����ĵڶ����ο����y����λ��
	int index1; // ����NPD�����ĵ�һ���ο����һά����λ��
	int index2; // ����NPD�����ĵڶ����ο����һά����λ��
};


struct CBranchNode // ���������ķ�֧�ڵ���
{
	CNPDFeature *pFea; // ��߶�NPD��������
	unsigned char cutpoint[2]; // �ֲ����ֵ
	void* pLeftChild; // ��������ָ�����е�isLeftABranchΪ��ʱָ��ֲ�ڵ㣨CBranchNode���ͣ���������ָ��Ҷ�ӽڵ㣨float���ͣ�����ָ��ع�ֵ��
	void* pRightChild; // ��������ָ��ͬpLeftChild
	bool isLeftABranch; // �������Ƿ��Ǹ��ֲ�ڵ�
	bool isRightABranch; // �������Ƿ��Ǹ��ֲ�ڵ�
};


class CNPDModel
{
public:
	CNPDModel(void);
	~CNPDModel(void);

	// ����CNPDDetectorΪ��Ԫ�࣬�Ӷ����Է���CNPDModel����ܱ�����Ա��
	friend class CNPDDetector;

	// ��ȡѵ���õļ��ģ��
	// filename��ѵ���õļ��ģ���ļ���maxObjSize�����Ŀ���С��minObjSize����СĿ���С��
	// scaleFactor����߶ȼ��ĳ߶����ӣ�stepFactor�������������ӣ���������������ڼ�ⴰ��С�ı�����
	bool ReadModel(const char* filename, int maxObjSize = 300, int minObjSize = 30, double scaleFactor = 1.2, double stepFactor = 0.1);

protected:
	int mObjSize; // ģ���С
	int mNumStages; // ����������DQT��Ŀ
	int mNumBranchNodes; // ����������֧�ڵ��ܸ���
	int mNumLeafNodes; // ��������Ҷ�ӽڵ��ܸ���
	int *mpStageThreshold; // ÿ��ľ�����ֵ����Ҫ�Ӹ���������QUAN_LEVELS����Ϊ��������mNumStages��1
	CBranchNode **mppTreeRoot; // ÿ�����ĸ��ڵ�ָ�루ָ��mpBranchNode�е�ĳЩ�ڵ㣬��ָ��Ľڵ㼴Ϊ���ĸ��ڵ㣩��mNumStages��1
	CBranchNode *mpBranchNode; // ��֧�ڵ��б�mNumBranchNodes��1
	int *mpFit; // ÿ��Ҷ�ӽڵ��Ԥ��ֵ����Ҫ�Ӹ���������QUAN_LEVELS����Ϊ��������mNumLeafNodes��1

	int mNumScales; // ��߶ȼ��ĳ߶ȸ���
	double *mpScaleFactor; // ��߶ȼ��ĳ߶����ӣ�ģ��Ŵ�ϵ������mNumScales��1
	int *mpWinSize; // ��߶ȼ��ļ�ⴰ�ڴ�С��mNumScales��1
	int *mpWinStep; // ��ⴰ����������mNumScales��1
};

