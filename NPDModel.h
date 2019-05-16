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

struct CNPDFeature // NPD特征参数类
{
	short int x1; // 计算NPD特征的第一个参考点的x坐标位置
	short int y1; // 计算NPD特征的第一个参考点的y坐标位置
	short int x2; // 计算NPD特征的第二个参考点的x坐标位置
	short int y2; // 计算NPD特征的第二个参考点的y坐标位置
	int index1; // 计算NPD特征的第一个参考点的一维索引位置
	int index2; // 计算NPD特征的第二个参考点的一维索引位置
};


struct CBranchNode // 树分类器的分支节点类
{
	CNPDFeature *pFea; // 多尺度NPD特征参数
	unsigned char cutpoint[2]; // 分叉的阈值
	void* pLeftChild; // 左子树的指向。其中当isLeftABranch为真时指向分叉节点（CBranchNode类型），否则是指向叶子节点（float类型），即指向回归值。
	void* pRightChild; // 右子树的指向，同pLeftChild
	bool isLeftABranch; // 左子树是否是个分叉节点
	bool isRightABranch; // 右子树是否是个分叉节点
};


class CNPDModel
{
public:
	CNPDModel(void);
	~CNPDModel(void);

	// 申明CNPDDetector为友元类，从而可以访问CNPDModel类的受保护成员。
	friend class CNPDDetector;

	// 读取训练好的检测模型
	// filename：训练好的检测模型文件；maxObjSize：最大目标大小；minObjSize：最小目标大小；
	// scaleFactor：多尺度检测的尺度因子；stepFactor：滑动步长因子，即滑动步长相对于检测窗大小的比例。
	bool ReadModel(const char* filename, int maxObjSize = 300, int minObjSize = 30, double scaleFactor = 1.2, double stepFactor = 0.1);

protected:
	int mObjSize; // 模板大小
	int mNumStages; // 弱分类器或DQT数目
	int mNumBranchNodes; // 树分类器分支节点总个数
	int mNumLeafNodes; // 树分类器叶子节点总个数
	int *mpStageThreshold; // 每层的决策阈值（需要从浮点数根据QUAN_LEVELS量化为整数）。mNumStages×1
	CBranchNode **mppTreeRoot; // 每棵树的根节点指针（指向mpBranchNode中的某些节点，所指向的节点即为树的根节点）。mNumStages×1
	CBranchNode *mpBranchNode; // 分支节点列表。mNumBranchNodes×1
	int *mpFit; // 每棵叶子节点的预测值（需要从浮点数根据QUAN_LEVELS量化为整数）。mNumLeafNodes×1

	int mNumScales; // 多尺度检测的尺度个数
	double *mpScaleFactor; // 多尺度检测的尺度因子（模板放大系数），mNumScales×1
	int *mpWinSize; // 多尺度检测的检测窗口大小，mNumScales×1
	int *mpWinStep; // 检测窗滑动步长，mNumScales×1
};

