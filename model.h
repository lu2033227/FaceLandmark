#ifndef MODEL_H
#define MODEL_H
#include "stdafx.h"
#include "featpyramid.h"
#include "config.h"

class _nmatrix
{
public:
	sType *value;
	int size[2];
	_nmatrix(){value=NULL;}
};

class _partBias
{
	// partBias_x_min = [1-binSizePart*2 -binSizePart 1-binSizePart*2 -binSizePart];
	// partBias_x_max = partBias_x_min + binSizePart*3 - 1;
	// partBias_y_min = [1-binSizePart*2 1-binSizePart*2 -binSizePart -binSizePart];
	// partBias_y_max = partBias_y_min + binSizePart*3 - 1;
public:
	vector<int> partBias_x_min;
	vector<int> partBias_y_min;
	vector<int> partBias_x_max;
	vector<int> partBias_y_max;
};

class _matrixlist
{
public:
	vector<_nmatrix> Mats;
	int MatNum;
	void init(ifstream& filein, int _MatNum, int height, int width)
	{
		if (sizeof(sType) == sizeof(float))
		{
			MatNum = _MatNum;
			Mats.resize(MatNum);
			for (int i=0;i<MatNum;i++)
			{
				Mats[i].size[0] = height;
				Mats[i].size[1] = width;
				if (Mats[i].value == NULL)
				{
					Mats[i].value =new sType[height*width];
					filein.read((char*)Mats[i].value,sizeof(sType)*height*width);
				}
			}
		}
		else
		{
			MatNum = _MatNum;
			Mats.resize(MatNum);
			for (int i=0;i<MatNum;i++)
			{
				Mats[i].size[0] = height;
				Mats[i].size[1] = width;
				if (Mats[i].value == NULL)
				{
					Mats[i].value =new sType[height*width];
					double * tempData = new double[height*width];
					filein.read((char*)tempData,sizeof(double)*height*width);
					for (int j=0;j<height*width;j++)
					{
						Mats[i].value[j] = static_cast<sType>(tempData[j]);
					}
					delete []tempData;
				}
			}
		}
	}
	~_matrixlist()
	{
		for(int i=0;i<MatNum;i++)
		{
			if (Mats[i].value!= NULL)
			{
				delete [] Mats[i].value;
			    Mats[i].value = NULL;
			}
		}
	}
};

struct shapeXY
{
	vector<sType> shapeX;
	vector<sType> shapeY;
};

struct _predictbb
{
	sType tpredictbb[(partnum+1)*3];
};

class Model 
{
public:
	int sbin;
	int iterNum;
	_partBias partBias;
	sType meanDE;
#if FINEGRAINMODE
	int componentNum;
	vector<_predictbb> predictbb;
#else
	sType predictbb[6];
#endif
	shapeXY meanShape;
	_matrixlist regressionmatrix;
	uchar HOGGradient[511*511];
	_matrix featMat;
	_feat tfeat;
	float *score_w;
	int dx;
	int dy;
	int* best_o_t;
	sType* v_t;
	Model() {};
	Model(const string& n1) 
	{
		initmodel(n1); 
	};
	~Model();
	void initmodel(const string&);

    _vxvy_r vxvy_r;
	_vxvy_p vxvy_p;
};
#endif