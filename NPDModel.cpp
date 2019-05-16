// Class for the NPD detection model
// 
// Shengcai Liao
// National Laboratory of Pattern Recognition
// Institute of Automation, Chinese Academy of Sciences
// scliao@nlpr.ia.ac.cn
//

#include "stdafx.h"
#include "NPDModel.h"
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;


CNPDModel::CNPDModel(void)
{
	mObjSize = 0; // 模板大小
	mNumStages = 0; // 弱分类器或DQT数目
	mNumBranchNodes = 0; // 树分类器分支节点总个数
	mNumLeafNodes = 0; // 树分类器叶子节点总个数
	mpStageThreshold = 0; // 每层的决策阈值（需要从浮点数根据QUAN_LEVELS量化为整数）。mNumStages×1
	mppTreeRoot = 0; // // 每棵树的根节点指针（指向mpBranchNode中的某些节点，所指向的节点即为树的根节点）。mNumTrees×1
	mpBranchNode = 0; // 分支节点列表。mNumBranchNodes×1
	mpFit = 0; // 每棵叶子节点的预测值（需要从浮点数根据QUAN_LEVELS量化为整数）。mNumLeafNodes×1

	mNumScales = 0; // 多尺度检测的尺度个数
	mpScaleFactor = 0; // 多尺度检测的尺度因子（模板放大系数），mNumScales×1
	mpWinSize = 0; // 多尺度检测的检测窗口大小，mNumScales×1
	mpWinStep = 0; // 检测窗滑动步长，mNumScales×1
}


CNPDModel::~CNPDModel(void)
{
	if(mpStageThreshold != 0) delete[] mpStageThreshold;
	if(mppTreeRoot != 0) delete[] mppTreeRoot;
	if(mpFit != 0) delete[] mpFit;
	if(mpScaleFactor != 0) delete[] mpScaleFactor;
	if (mpWinSize != 0) delete[] mpWinSize;
	if (mpWinStep != 0) delete[] mpWinStep;

	if(mpBranchNode != 0)
	{
		for(int i = 0; i < mNumBranchNodes; i++) // 删除多尺度NPD特征参数存储空间
		{
			delete[] mpBranchNode[i].pFea;
		}

		delete[] mpBranchNode;
	}
}


// 读取训练好的检测模型
// filename：训练好的检测模型文件；maxObjSize：最大目标大小；minObjSize：最小目标大小；
// scaleFactor：多尺度检测的尺度因子；stepFactor：滑动步长因子，即滑动步长相对于检测窗大小的比例。
bool CNPDModel::ReadModel(const char* filename, int maxObjSize, int minObjSize, double scaleFactor, double stepFactor)
{
	if(scaleFactor <= 1.0f || minObjSize <= 0 || maxObjSize < minObjSize)
	{
		cout << "Error input parameters: scaleFactor <= 1.0f || minObjSize <= 0 || maxObjSize < minObjSize." << endl;
		return false;
	}

	ifstream fin(filename, ios::binary);
	if(fin.fail())
	{
		cout << "Cannot open model file: " << filename << endl;
		return false;
	}

	cout << "Load NPD detection model from " << filename << endl;

	fin.read((char*) &mObjSize, sizeof(int)); // 模板大小
	fin.read((char*) &mNumStages, sizeof(int)); // 弱分类器或DQT数目
	if(fin.fail())
	{
		cout << "Error reading the model file for mObjSize and mNumStages: " << filename << endl;
		fin.close();
		return false;
	}

	if(mObjSize <= 0 || mNumStages <= 0)
	{
		cout << "Error model parameters: mObjSize <= 0 || mNumStages <= 0." << endl;
		fin.close();
		return false;
	}

	fin.read((char*) &mNumBranchNodes, sizeof(int)); // 树分类器分支节点总个数
	fin.read((char*) &mNumLeafNodes, sizeof(int)); // 树分类器叶子节点总个数
	if(fin.fail())
	{
		cout << "Error reading the model file for mNumBranchNodes, and mNumLeafNodes: " << filename << endl;
		fin.close();
		return false;
	}

	if(mNumBranchNodes <= 0 || mNumLeafNodes <= 0)
	{
		cout << "Error model parameters: mNumBranchNodes <= 0 || mNumLeafNodes <= 0." << endl;
		fin.close();
		return false;
	}

	mpStageThreshold = new int [mNumStages]; // 每层的决策阈值（需要从浮点数根据QUAN_LEVELS量化为整数）。mNumStages×1
	if(mpStageThreshold == 0)
	{
		cout << "Out of memory: mpStageThreshold." << endl;
		fin.close();
		return false;
	}
	

	for ( int i = 0; i < mNumStages; i++)
	{
		float threshold;
		fin.read((char*) &threshold, sizeof(float));
		if(fin.fail())
		{
			cout << "Error reading the model file for mpStageThreshold: " << filename << endl;
			fin.close();
			return false;
		}
		
		// 变换阈值以提高比对速度
		//threshold = float ( log( threshold / (1.0 - (double)threshold) ) / 2.0 );
		// 每层的决策阈值（需要从浮点数根据QUAN_LEVELS量化为整数）
		mpStageThreshold[i] = (int) floor(double(threshold) * QUAN_LEVELS);
	}

	mppTreeRoot = new CBranchNode* [mNumStages]; // 每棵树的根节点指针（指向mpBranchNode中的某些节点，所指向的节点即为树的根节点）。mNumStages×1
	mpBranchNode = new CBranchNode[mNumBranchNodes]; // 分支节点列表。mNumBranchNodes×1
	mpFit = new int[mNumLeafNodes]; // 每棵叶子节点的预测值。mNumLeafNodes×1
	if (mppTreeRoot == 0 || mpBranchNode == 0 || mpFit == 0)
	{
		cout << "Out of memory: mppTreeRoot, mpBranchNode, mpFit." << endl;
		fin.close();
		return false;
	}

	for (int i = 0; i < mNumStages; i++)
	{
		int rootIndex;
		fin.read((char*)&rootIndex, sizeof(int));
		if (fin.fail())
		{
			cout << "Error reading the model file for tree root index: " << filename << endl;
			fin.close();
			return false;
		}

		mppTreeRoot[i] = mpBranchNode + rootIndex;
	}


	minObjSize = max(mObjSize, minObjSize); //最小检测窗口大小必须大于模板窗口大小
	mNumScales = (int) floor((log(maxObjSize) - log(minObjSize)) / log(scaleFactor)) + 1; // 确定多尺度检测的尺度个数

	for(int i = 0; i < mNumBranchNodes; i++) // 开辟多尺度NPD特征参数存储空间
	{
		mpBranchNode[i].pFea = new CNPDFeature[mNumScales];
		if(mpBranchNode[i].pFea == 0)
		{
			cout << "Out of memory: mpBranchNode[i].pFea." << endl;
			fin.close();
			return false;
		}
	}

	for(int i = 0; i < mNumBranchNodes; i++) // 读pixel1
	{
		short int pixel;
		fin.read((char*)&pixel, sizeof(short int));
		if(fin.fail())
		{
			cout << "Error reading the model file for pixel1: " << filename << endl;
			fin.close();
			return false;
		}

		mpBranchNode[i].pFea[0].x1 = pixel / mObjSize; //自动floor取整
		mpBranchNode[i].pFea[0].y1 = pixel % mObjSize;
	}

	for (int i = 0; i < mNumBranchNodes; i++) // 读pixel2
	{
		short int pixel;
		fin.read((char*)&pixel, sizeof(short int));
		if (fin.fail())
		{
			cout << "Error reading the model file for pixel2: " << filename << endl;
			fin.close();
			return false;
		}
		
		mpBranchNode[i].pFea[0].x2 = pixel / mObjSize; //自动floor取整
		mpBranchNode[i].pFea[0].y2 = pixel % mObjSize;
	}

	for (int i = 0; i < mNumBranchNodes; i++) // 读分叉节点的分叉阈值
	{
		fin.read((char*)&mpBranchNode[i].cutpoint, 2 * sizeof(unsigned char));
		if (fin.fail())
		{
			cout << "Error reading the model file for mpBranchNode[i].cutpoint: " << filename << endl;
			fin.close();
			return false;
		}
	}

	int child;

	for (int i = 0; i < mNumBranchNodes; i++) // 读左子树的指向
	{
		fin.read((char*)&child, sizeof(int));
		if (fin.fail())
		{
			cout << "Error reading the model file for left child: " << filename << endl;
			fin.close();
			return false;
		}

		if (child >= 0) // 指向分叉节点
		{
			mpBranchNode[i].isLeftABranch = true;
			mpBranchNode[i].pLeftChild = mpBranchNode + child;
		}
		else // 指向叶子节点
		{
			mpBranchNode[i].isLeftABranch = false;
			mpBranchNode[i].pLeftChild = mpFit - child - 1;
		}
	}

	for (int i = 0; i < mNumBranchNodes; i++) // 读右子树的指向
	{
		fin.read((char*)&child, sizeof(int));
		if (fin.fail())
		{
			cout << "Error reading the model file for right child: " << filename << endl;
			fin.close();
			return false;
		}

		if (child >= 0) // 指向分叉节点
		{
			mpBranchNode[i].isRightABranch = true;
			mpBranchNode[i].pRightChild = mpBranchNode + child;
		}
		else // 指向叶子节点
		{
			mpBranchNode[i].isRightABranch = false;
			mpBranchNode[i].pRightChild = mpFit - child - 1;
		}
	}

	for( int i = 0; i < mNumLeafNodes; i++ )
	{
		float fit = 0;
		fin.read((char*)&fit, sizeof(float)); // 读叶子节点的预测值
		if (fin.fail())
		{
			cout << "Error reading the model file for mpFit: " << filename << endl;
			fin.close();
			return false;
		}
		
		// 每棵叶子节点的预测值（需要从浮点数根据QUAN_LEVELS量化为整数）
		mpFit[i] = (int) floor(double(fit) * QUAN_LEVELS);;
	}

	fin.close();
	
	cout << "Succefully loaded the NPD detection model." << endl << endl
		<< "Model details: " << endl
		<< "objSize: " << mObjSize << endl
		<< "#stages: " << mNumStages << endl
		<< "#branches: " << mNumBranchNodes << endl
		<< "#leaves: " << mNumLeafNodes << endl << endl
		<< "Detection parameters: " << endl
		<< "maxObjSize = " << maxObjSize << endl
		<< "minObjSize = " << minObjSize << endl
		<< "scaleFactor = " << scaleFactor << endl
		<< "stepFactor = " << stepFactor << endl << endl;


	mpScaleFactor = new double[mNumScales]; // 多尺度检测的尺度因子（模板放大系数），mNumScales×1
	mpWinSize = new int[mNumScales]; // 多尺度检测的检测窗口大小，mNumScales×1
	mpWinStep = new int[mNumScales]; // 检测窗滑动步长，mNumScales×1
	if (mpScaleFactor == 0 || mpWinSize == 0 || mpWinStep == 0)
	{
		cout << "Out of memory: mpScaleFactor, mpWinSize, mpWinStep." << endl;
		fin.close();
		return false;
	}

	for (int s = 0; s < mNumScales; s++)
	{
		mpWinSize[s] = (int) floor(minObjSize * pow(scaleFactor, s) + 0.5); // 不用迭代乘scaleFactor是为了避免取整误差被放大
		mpScaleFactor[s] = double(mpWinSize[s]) / double(mObjSize);

		// 确定检测窗滑动步长
		mpWinStep[s] = (int)floor(mpWinSize[s] * stepFactor);
		if (mpWinStep[s] < 1) mpWinStep[s] = 1;
	}

	// 扩展检测器为多尺度检测器
	for (int i = 0; i < mNumBranchNodes; i++)
	{
		for (int s = 1; s < mNumScales; s++)
		{
			mpBranchNode[i].pFea[s].x1 = (short) floor(mpBranchNode[i].pFea[0].x1 * mpScaleFactor[s] + 0.5);
			mpBranchNode[i].pFea[s].y1 = (short) floor(mpBranchNode[i].pFea[0].y1 * mpScaleFactor[s] + 0.5);
			mpBranchNode[i].pFea[s].x2 = (short) floor(mpBranchNode[i].pFea[0].x2 * mpScaleFactor[s] + 0.5);
			mpBranchNode[i].pFea[s].y2 = (short) floor(mpBranchNode[i].pFea[0].y2 * mpScaleFactor[s] + 0.5);
		}
	}

	return true;
}
