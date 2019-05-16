#include "stdafx.h"
#include "model.h"

sType round1(sType r) 
{
	return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}

void CalcGradientTable(uchar* inputMat, int numSection)//输入必须是511*511大小的一个向量，分成多少份
{
	float eachOffset = 360/numSection;
	float angleoffset = (eachOffset / 180) * 3.1415926;
	float radio = 0;
	int index = 0;
	uchar best_o;
	bool symbol_dy;
	bool symbol_dx;
	for (int i=255; i>=-255; i--)//y
	{
		symbol_dy = i>0? 1:0;
		for (int j=-255; j<=255; ++j)//x
		{
			if (j==0)
			{
				if (i>0)
				{
					best_o = 4;
				}
				else if (i==0)
				{
					best_o = 0;
				}
				else
				{
					best_o = 4 + ceil((float)numSection/2);
				}
				inputMat[index] = best_o;
				index++;
				continue;
			}
			symbol_dx = j>0? 1:0;

			float radio = (float)i/j;
			float angle = atan(fabs(radio));
			float chushu = angle/angleoffset;
			float yushu = chushu-(int)chushu;
			if (radio>=0)
			{
				best_o = (int)chushu;
				if (yushu> 0.5)   
				{
					best_o += 1;
				}
				if(symbol_dy==false && symbol_dx==false)
				{
					best_o += ceil((float)numSection/2);
				}
			}
			else
			{
				if (chushu<1 && yushu< 0.5)
				{
					best_o = 0;
					if (symbol_dy==true && symbol_dx==false)
					{
						best_o = ceil((float)numSection/2);
					}
				}
				else
				{
					best_o = numSection-(int)chushu;
					if (yushu > 0.5)
					{
						best_o -= 1;
					}
					if (symbol_dy==true && symbol_dx==false)
					{
						best_o -= ceil((float)numSection/2);
					}
				}
				
			}

			inputMat[index] = best_o;
			index++;
		}
	}

	/*
	cv::Mat showimg(510,510, CV_8UC3);
	uchar *pImg = showimg.data;
	int* color = new int[numSection*3];
	srand( (unsigned)time( NULL ) );
	for (int i=0; i<numSection*3; ++i)
	{
		color[i] = rand()%255;
	}
	for (int i=0; i<510; ++i)
	{
		for (int j=0; j<510*3; j+=3, pImg+=3)
		{
			pImg[0] = color[inputMat[index]*3];
			pImg[1] = color[inputMat[index]*3+1];
			pImg[2] = color[inputMat[index]*3+2];
			index++;
		}
	}
	imshow("img", showimg);
	cvWaitKey(0);
	destroyAllWindows();
	delete []color;*/
}


void Model::initmodel(const string& modelfile)
{
	if (sizeof(sType) == sizeof(float))
	{
		ifstream filein(modelfile,ios::in | ios::binary);
		sType tIterNum;

		filein.read((char*)&tIterNum,sizeof(sType));
		iterNum = round1(tIterNum);

		filein.read((char*)&meanDE,sizeof(sType));

		filein.read((char*)predictbb,6*sizeof(sType));

		meanShape.shapeX.resize(_pointNum);
		meanShape.shapeY.resize(_pointNum);

		for(int i=0;i<_pointNum;i++)
			filein.read((char*)&meanShape.shapeX[i],sizeof(sType));
		for(int i=0;i<_pointNum;i++)
			filein.read((char*)&meanShape.shapeY[i],sizeof(sType));

		regressionmatrix.init(filein,iterNum,_featDim1,_featDim2);

		score_w = new float[10541];
		filein.read((char *)score_w, 10541*sizeof(float));
		filein.close();

		partBias.partBias_x_min.push_back(1-_binSizePart*2);
		partBias.partBias_x_min.push_back(-_binSizePart);
		partBias.partBias_x_min.push_back(1-_binSizePart*2);
		partBias.partBias_x_min.push_back(-_binSizePart);

		partBias.partBias_x_max.push_back(partBias.partBias_x_min[0] + _binSizePart*3 - 1);
		partBias.partBias_x_max.push_back(partBias.partBias_x_min[1] + _binSizePart*3 - 1);
		partBias.partBias_x_max.push_back(partBias.partBias_x_min[2] + _binSizePart*3 - 1);
		partBias.partBias_x_max.push_back(partBias.partBias_x_min[3] + _binSizePart*3 - 1);

		partBias.partBias_y_min.push_back(1-_binSizePart*2);
		partBias.partBias_y_min.push_back(1-_binSizePart*2);
		partBias.partBias_y_min.push_back(-_binSizePart);
		partBias.partBias_y_min.push_back(-_binSizePart);

		partBias.partBias_y_max.push_back(partBias.partBias_y_min[0] + _binSizePart*3 - 1);
		partBias.partBias_y_max.push_back(partBias.partBias_y_min[1] + _binSizePart*3 - 1);
		partBias.partBias_y_max.push_back(partBias.partBias_y_min[2] + _binSizePart*3 - 1);
		partBias.partBias_y_max.push_back(partBias.partBias_y_min[3] + _binSizePart*3 - 1);

		CalcGradientTable(HOGGradient, 18);

		if (featMat.value == NULL)
		{
			featMat.value = new sType[sizeof(sType)*_featDim2];
			featMat.size[0] = _featDim2;
			featMat.size[1] = 1;
			featMat.value[_featDim2-1] = 1;
		}

		tfeat.size[0] = 1;
		tfeat.size[1] = 1;
		tfeat.size[2] = 32;
		tfeat.ft=new sType[32];

		dx = 0;
		dy = 0;

		best_o_t = NULL;
		v_t = NULL;
	}

	vxvy_r.patchwidth = _binSizeRoot;
	for (int x = 0;x<_binSizeRoot;x++)
	{
		for (int y = 0; y<_binSizeRoot;y++)
		{
			sType xp = ((sType)x+0.5)/(sType)_binSizeRoot - 0.5;
			sType yp = ((sType)y+0.5)/(sType)_binSizeRoot - 0.5;
			int ixp = (int)floor(xp);
			int iyp = (int)floor(yp);
			sType vx0 = xp-ixp;
			sType vy0 = yp-iyp;
			sType vx1 = 1.0-vx0;
			sType vy1 = 1.0-vy0;
			vxvy_r.vx0[y][x] = vx0;
			vxvy_r.vy0[y][x] = vy0;
			vxvy_r.vx1[y][x] = vx1;
			vxvy_r.vy1[y][x] = vy1;
			vxvy_r.vx0vy0[y][x] = vx0*vy0;
			vxvy_r.vx0vy1[y][x] = vx0*vy1;
			vxvy_r.vx1vy0[y][x] = vx1*vy0;
			vxvy_r.vx1vy1[y][x] = vx1*vy1;
		}
	}

	for (int i=0;i<_binSizeRoot;i++)
	{
		sType xp = ((sType)i+0.5)/(sType)_binSizeRoot - 0.5;
		int ixp = (int)floor(xp);
		vxvy_r.ixp[i] = ixp;
		vxvy_r.iyp[i] = ixp;
	}
	vxvy_p.patchwidth = _binSizePart;
	for (int x = 0;x<_binSizePart;x++)
	{
		for (int y = 0; y<_binSizePart;y++)
		{
			sType xp = ((sType)x+0.5)/(sType)_binSizePart - 0.5;
			sType yp = ((sType)y+0.5)/(sType)_binSizePart - 0.5;
			int ixp = (int)floor(xp);
			int iyp = (int)floor(yp);
			sType vx0 = xp-ixp;
			sType vy0 = yp-iyp;
			sType vx1 = 1.0-vx0;
			sType vy1 = 1.0-vy0;
			vxvy_p.vx0[y][x] = vx0;
			vxvy_p.vy0[y][x] = vy0;
			vxvy_p.vx1[y][x] = vx1;
			vxvy_p.vy1[y][x] = vy1;
			vxvy_p.vx0vy0[y][x] = vx0*vy0;
			vxvy_p.vx0vy1[y][x] = vx0*vy1;
			vxvy_p.vx1vy0[y][x] = vx1*vy0;
			vxvy_p.vx1vy1[y][x] = vx1*vy1;
		}
	}

	for (int i=0;i<_binSizePart;i++)
	{
		sType xp = ((sType)i+0.5)/(sType)_binSizePart - 0.5;
		int ixp = (int)floor(xp);
		vxvy_p.ixp[i] = ixp;
		vxvy_p.iyp[i] = ixp;
	}

}

Model::~Model() 
{
	if (featMat.value!=NULL)
	{
		delete [] featMat.value;
		featMat.value = NULL;
	}

	if (tfeat.ft!=NULL)
	{
		delete [] tfeat.ft;
		tfeat.ft = NULL;
	}
	if (v_t != NULL)
	{
		delete []v_t;
		v_t == NULL;
	}
	if(best_o_t != NULL)
	{
		delete []best_o_t;
		best_o_t = NULL;
	}
	if(score_w != NULL)
	{
		delete []score_w;
		score_w = NULL;
	}
}