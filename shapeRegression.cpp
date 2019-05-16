#include "stdafx.h"
#include "shapeRegression.h"
#include "pose.h"

#include <cblas.h>

sType round2(sType r) 
{
	return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}

struct TPointF {
	float x;
	float y;
};

static BOOL solve(float* A, int m, float* b)
{
    float* L = A;
    int i, j, k;
    float s;
    
    for( i = 0; i < m; i++ ) {
        for( j = 0; j < i; j++ ) {
            s = A[i * m + j];
            for( k = 0; k < j; k++ )
                s -= L[i * m + k] * L[j * m + k];
            L[i * m + j] = s * L[j * m + j];
        }
        s = A[i * m + i];
        for( k = 0; k < j; k++ ) {
            float t = L[i * m + k];
            s -= t * t;
        }
        if( s < FLT_MIN )
            return FALSE;
        L[i * m + i] = 1.0f / sqrtf(s);
    }
    
    if( !b )
        return TRUE;
    
    // LLt x = b
    // 1: L y = b
    // 2. Lt x = y
    
    /*
     [ L00             ]  y0   b0
     [ L10 L11         ]  y1 = b1
     [ L20 L21 L22     ]  y2   b2
     [ L30 L31 L32 L33 ]  y3   b3
     
     [ L00 L10 L20 L30 ]  x0   y0
     [     L11 L21 L31 ]  x1 = y1
     [         L22 L32 ]  x2   y2
     [             L33 ]  x3   y3
    */
    
    for( i = 0; i < m; i++ ) {
        s = b[i];
        for( k = 0; k < i; k++ )
            s -= L[i * m + k] * b[k];
        b[i] = s * L[i * m + i];
    }
    
    for( i = m - 1; i >= 0; i-- ) {
        s = b[i];
        for( k = m - 1; k > i; k-- )
            s -= L[k * m + i] * b[k];
        b[i] = s * L[i * m + i];
    }
    
    return TRUE;
}

/* solve linear equation system of 4th order, Ah=b, h=A^(-1)b */
static void linsolve4(float* A, float* b, float* h)
{
	
	float det;
	float x, y, w, z;
	
	x =  A[0]; y =  A[1]; w =  A[2]; z =  A[3];	
	det = w*z - x*x - y*y; 
	
	/* 
	                 |-x -y  w  0| 
	              1  | y -x  0  w|
       A^(-1) = -----| z  0 -x  y|
	             det | 0  z -y -x|
	  	
	*/	
		
	h[0] = -x * b[0] - y * b[1] + w * b[2];
	h[1] =  y * b[0] - x * b[1] + w * b[3];
	h[2] =  z * b[0] - x * b[2] + y * b[3];
	h[3] =  z * b[1] - y * b[2] - x * b[3];
	
	h[0] /= det;
	h[1] /= det;
	h[2] /= det;
	h[3] /= det;	
}

// compute scale, rotation, and translation parameters
static void sim_params_from_points(const TPointF dstKeyPoints[],  
									 const TPointF srcKeyPoints[], int count,
									 float* a, float* b, float* tx, float* ty)
{
	int i;
	float X1, Y1, X2, Y2, Z, C1, C2;
	float A[4], c[4], h[4];
	
	X1 = 0.f; Y1 = 0.f;
	X2 = 0.f; Y2 = 0.f;
	Z = 0.f; 
	C1 = 0.f; C2 = 0.f;
	for(i = 0; i < count; i++) {
		float x1, y1, x2, y2;

		x1 = dstKeyPoints[i].x; 
		y1 = dstKeyPoints[i].y;
		x2 = srcKeyPoints[i].x; 
		y2 = srcKeyPoints[i].y;				
		
		X1 += x1;
		Y1 += y1;
		X2 += x2;
		Y2 += y2;	
		Z  += (x2 * x2 + y2 * y2);
		C1 += (x1 * x2 + y1 * y2);
		C2 += (y1 * x2 - x1 * y2);		
	}

	/* solve Ah = c 
	   A = 	|X2, -Y2,   w,   0|		b=|X1|
	        |Y2,  X2,   0,   w|       |Y1|
	        | Z,   0,  X2,  Y2|       |C1|
	        | 0,   Z, -Y2,  X2|       |C2|;	
	*/
	A[0] = X2; A[1] = Y2; A[2] = (float)count;  A[3] =  Z;
	c[0] = X1; c[1] = Y1; c[2] = C1; c[3] = C2;
	linsolve4(A, c, h);
	
	/* rotation, scaling, and translation parameters */
	*a = h[0];
	*b = h[1];
    *tx = h[2];
	*ty = h[3];
}

static void sim_transform_landmark(const TPointF* landmark, TPointF* dst, 
	int count, float a, float b, float tx, float ty)
{
	int i;
	float x, y;

	// transform last shape to current shape
	for(i = 0; i < count; i++) {
		x = landmark[i].x;
		y = landmark[i].y;

		dst[i].x = a * x - b * y + tx;
		dst[i].y = a * y + b * x + ty;
	}
} 

static void affine_params_from_points(const TPointF dstKeyPoints[],  
								   const float srcKeyPoints[][3], int count,
								   float affine[])
{
	int i, j, k;
	float A[16], A1[64];
	
	for (i = 0; i < 16; i++) {
		A[i] = 0.0f;
	}
	for (i = 0; i < 8; i++) {
		affine[i] = 0.0f;
	}
	for(i = 0; i < count; i++) {
		A[0] += srcKeyPoints[i][0] * srcKeyPoints[i][0];
		A[1] += srcKeyPoints[i][0] * srcKeyPoints[i][1];
		A[2] += srcKeyPoints[i][0] * srcKeyPoints[i][2];
		A[3] += srcKeyPoints[i][0];

		// A[4] = A[1]
		A[5] += srcKeyPoints[i][1] * srcKeyPoints[i][1];
		A[6] += srcKeyPoints[i][1] * srcKeyPoints[i][2];
		A[7] += srcKeyPoints[i][1];

		// A[8] = A[2]
		// A[9] = A[6]
		A[10] += srcKeyPoints[i][2] * srcKeyPoints[i][2];
		A[11] += srcKeyPoints[i][2];

		// A[12] = A[3]
		// A[13] = A[7]
		// A[14] = A[11]
		A[15] += 1.0f;

		affine[0] += srcKeyPoints[i][0] * dstKeyPoints[i].x;
		affine[1] += srcKeyPoints[i][1] * dstKeyPoints[i].x;
		affine[2] += srcKeyPoints[i][2] * dstKeyPoints[i].x;
		affine[3] += dstKeyPoints[i].x;
		affine[4] += srcKeyPoints[i][0] * dstKeyPoints[i].y;
		affine[5] += srcKeyPoints[i][1] * dstKeyPoints[i].y;
		affine[6] += srcKeyPoints[i][2] * dstKeyPoints[i].y;
		affine[7] += dstKeyPoints[i].y;
	}
	A[4] = A[1];
	A[8] = A[2];
	A[9] = A[6];
	A[12] = A[3];
	A[13] = A[7];
	A[14] = A[11];
	for (i = 0; i < 64; i++) {
		A1[i] = 0.0f;
	}

	k = 0;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			A1[i * 8 + j] = A[k];
			A1[(i + 4) * 8 + j + 4] = A[k];
			k++;
		}
	}

	solve(A1, 8, affine);
}

static void affine_transform_landmark(const float facialPoints[][3], int count, const float affine[], TPointF* dst)
{
	int i;
	float x, y, z;

	// transform last shape to current shape
	for(i = 0; i < count; i++) {
		x = facialPoints[i][0];
		y = facialPoints[i][1];
		z = facialPoints[i][2];

		dst[i].x = affine[0] * x + affine[1] * y + affine[2] * z + affine[3];
		dst[i].y = affine[4] * x + affine[5] * y + affine[6] * z + affine[7];
	}
}


#if FINEGRAINMODE
void calcp1(vector<sType>& p1,sType* predictbb,_bbox& bbox)
{
	sType regFeat[partnum+1];
	
	regFeat[0] = (bbox.rootcoord[0]+bbox.rootcoord[2])/2;

	for (int i=0;i<partnum;i++)
	{
		regFeat[i+1] = (bbox.parts[i].left+bbox.parts[i].right)/2;
	}

	sType tempType = 0;
	for (int i=0;i<partnum+1;i++)
	{
		tempType+= predictbb[i]*regFeat[i];
	}
	p1.push_back(tempType);

	tempType = 0;
	for (int i=0;i<partnum+1;i++)
	{
		tempType+= predictbb[i+partnum+1]*regFeat[i];
	}
	p1.push_back(tempType);

	tempType = 0;
	for (int i=0;i<partnum+1;i++)
	{
		tempType+= predictbb[i+2*partnum+2]*regFeat[i];
	}
	p1.push_back(tempType);
}
#else
void calcp1(vector<sType>& p1,sType* predictbb,_bbox& bbox)
{
	sType bx = bbox.rootcoord[2] - bbox.rootcoord[0] + 1;
	sType by = bbox.rootcoord[3] - bbox.rootcoord[1] + 1;

	p1.push_back(predictbb[0]*bx + predictbb[3]*by);
	p1.push_back(predictbb[1]*bx + predictbb[4]*by);
	p1.push_back(predictbb[2]*bx + predictbb[5]*by);
}
#endif

void initpts(shapeXY& shape, shapeXY& meanshape, vector<sType>& p1, sType resizescale,_bbox& bbox)
{
	shape.shapeX = meanshape.shapeX;
	shape.shapeY = meanshape.shapeY;
	for (int i=0;i<_pointNum;i++)
	{
		shape.shapeX[i] = (shape.shapeX[i]*p1[2]+p1[0]+bbox.rootcoord[0]-1)*resizescale;
		shape.shapeY[i] = (shape.shapeY[i]*p1[2]+p1[1]+bbox.rootcoord[1]-1)*resizescale;
	}
	for (int i=0;i<4;i++)
	{
		bbox.rootcoord[i] = bbox.rootcoord[i] *resizescale;
	}
}

void initpts2(shapeXY& shape, shapeXY& meanshape, shapeXY &dstShape, vector<sType>& p1, sType resizescale,_bbox& bbox)
{
	shape.shapeX = meanshape.shapeX;
	shape.shapeY = meanshape.shapeY;

	// int norm_bin[21] = {1, 5, 9, 13, 17, 18, 22, 23, 27,28, 31, 32, 36, 37, 40, 43, 46, 49, 52, 55, 58};

	float a, b, tx, ty;
	float a1, b1, tx1, ty1;
	TPointF landmark[_pointNum];
	TPointF dstPoints[_pointNum];
	TPointF srcPoints[_pointNum];
/*	for(int i=0; i<21; i++)
	{
		dstPoints[i].x = dstShape.shapeX[norm_bin[i]-1];
		dstPoints[i].y = dstShape.shapeY[norm_bin[i]-1];
		srcPoints[i].x = meanshape.shapeX[norm_bin[i]-1];
		srcPoints[i].y = meanshape.shapeY[norm_bin[i]-1];
	}*/

	for(int i=0; i<_pointNum; i++)
	{
		dstPoints[i].x = dstShape.shapeX[i];
		dstPoints[i].y = dstShape.shapeY[i];
		srcPoints[i].x = meanshape.shapeX[i];
		srcPoints[i].y = meanshape.shapeY[i];
	}
	sim_params_from_points(dstPoints, srcPoints, _pointNum, &a, &b, &tx, &ty);

	for(int i=0; i<_pointNum; i++)
	{
		srcPoints[i].x = meanshape.shapeX[i];
		srcPoints[i].y = meanshape.shapeY[i];
	}

	sim_transform_landmark(srcPoints, landmark, _pointNum, a, b, tx, ty);

/*	TPointF landmark[_pointNum];
	TPointF dstPoints[_pointNum];
	float srcPoints[_pointNum][3];

	for(int i=0; i<_pointNum; i++)
	{
		dstPoints[i].x = dstShape.shapeX[i];
		dstPoints[i].y = dstShape.shapeY[i];
		srcPoints[i][0] = meanshape.shapeX[i];
		srcPoints[i][1] = meanshape.shapeY[i];
		srcPoints[i][2] = 1;
	}

	float affine[8];
	affine_params_from_points(dstPoints, srcPoints, _pointNum, affine);
	affine_transform_landmark(srcPoints, _pointNum, affine, landmark);
	*/


	for (int i=0;i<_pointNum;i++)
	{
		shape.shapeX[i] = (landmark[i].x)*resizescale;
		shape.shapeY[i] = (landmark[i].y)*resizescale;
	}
	for (int i=0;i<4;i++)
	{
		bbox.rootcoord[i] = bbox.rootcoord[i] *resizescale;
	}
}

void updateShape(Model& model, _matrix& featMat,shapeXY& shape, int iternum)
{
	sType deltaShape[_pointNum*2];
	for (int i=0;i<_pointNum*2;i++)
	{
		deltaShape[i] = 0.0;
	}
	int m = 1;
	int k = _featDim2;
	int n = _featDim1;

#if USEDOUBLE
	cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, 1.0, featMat.value, k, model.regressionmatrix.Mats[iternum].value,n,0.0,deltaShape,n);
#else
	cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, 1.0, featMat.value, k, model.regressionmatrix.Mats[iternum].value,n,0.0,deltaShape,n);
#endif
	for (int i=0;i<_pointNum;i++)
	{
		shape.shapeX[i] += deltaShape[i];
		shape.shapeY[i] += deltaShape[i+_pointNum];
	}
}

bool pointOut(Model& model,Mat& img,shapeXY shape)
{
	int min_x = INFINITY;
	int min_y = INFINITY;
	int max_x = -INFINITY;
	int max_y = -INFINITY;

	for (int i=0;i<_pointNum;i++)
	{
		min_x = (shape.shapeX[i]<min_x) ? shape.shapeX[i]:min_x;
		min_y = (shape.shapeY[i]<min_y) ? shape.shapeY[i]:min_y;
		max_x = (shape.shapeX[i]>max_x) ? shape.shapeX[i]:max_x;
		max_y = (shape.shapeY[i]>max_y) ? shape.shapeY[i]:max_y;
	}
	int height = img.rows;
	int width = img.cols;

	if (min_x-_binSizeRoot<0||min_y-_binSizeRoot<0||max_x+_binSizeRoot-1>width||max_y+_binSizeRoot-1>height)
	{
		return true;
	}
	return false;
}

bool extractFeature(Model& model,Mat& img,shapeXY shape)
{
	if (pointOut(model,img,shape))
	{
		return false;
	}

	int x_min,y_min,x_max,y_max;
	for (int i=0;i<_pointNum;i++)
	{
		x_min = static_cast<int>(shape.shapeX[i]) - _biasNum;
		y_min = static_cast<int>(shape.shapeY[i]) - _biasNum;
		x_max = static_cast<int>(shape.shapeX[i]) + _biasNum;
		y_max = static_cast<int>(shape.shapeY[i]) + _biasNum;

#if fastMode
		//features_lookup(model.tfeat,_binSizeRoot, model.best_o_t, model.v_t, x_min,y_min,img.cols);
		features_lookup(model.tfeat,_binSizeRoot, model.best_o_t, model.v_t, x_min,y_min,img.cols,model.vxvy_r);
#else
		features_lookup(img(Range(y_min,y_max+1), Range(x_min,x_max+1)),model.tfeat,_binSizeRoot, model.HOGGradient);
#endif
		memcpy(model.featMat.value+i*_featPoint,model.tfeat.ft,31*sizeof(sType));

		for (int j=0;j<4;j++)
		{
			x_min = static_cast<int>(shape.shapeX[i]) + model.partBias.partBias_x_min[j];
			y_min = static_cast<int>(shape.shapeY[i]) + model.partBias.partBias_y_min[j];
			x_max = static_cast<int>(shape.shapeX[i]) + model.partBias.partBias_x_max[j];
			y_max = static_cast<int>(shape.shapeY[i]) + model.partBias.partBias_y_max[j];
#if fastMode
			//features_lookup(model.tfeat,_binSizePart, model.best_o_t, model.v_t, x_min,y_min,img.cols);
			features_lookup(model.tfeat,_binSizePart, model.best_o_t, model.v_t, x_min,y_min,img.cols,model.vxvy_p);
#else
			features_lookup(img(Range(y_min,y_max+1), Range(x_min,x_max+1)),model.tfeat,_binSizePart, model.HOGGradient);
#endif
			memcpy(model.featMat.value+i*_featPoint+(j+1)*31,model.tfeat.ft,31*sizeof(sType));
		}
	}
	return true;
}


Mat combineMat_x(Mat& A,Mat& B)
{	
	int height = A.rows;
	int datanuma = 3*A.cols;
	int datanumb = 3*B.cols;
	
	int widtha = A.step.buf[0];
	int widthb = B.step.buf[0];

	Mat C(height,A.cols+B.cols, CV_8UC3, Scalar(0,0,255));

	int widthc = C.step.buf[0];
    int datanumc = 3*C.cols;

	uchar* sa = A.data;
	uchar* sb = B.data;
	uchar* sc = C.data;

	int tempInt1 = 0;
	int tempInt2 = datanuma;
	int tempInt3 = 0;
	int tempInt4 = 0;

	for (int i=0;i<height;i++)
	{
		for (int j=0;j<datanuma;j++)
		{
			*(sc+tempInt1+j) = sa[tempInt3+j];
		}
		for (int j=0;j<datanumb;j++)
		{
			*(sc+j+tempInt2) = sb[tempInt4+j];
		}
		tempInt1 += widthc;
		tempInt2 += widthc;
		tempInt3 += widtha;
		tempInt4 += widthb;
	}
	return C;
}

Mat combineMat_y(Mat& A, Mat& B)
{
	uchar* sa = A.data;
	uchar* sb = B.data;
	
	int widtha = A.step.buf[0];
	int widthb = B.step.buf[0];
	
	int datanuma = 3*A.cols;

	int heighta = A.rows;
	int heightb = B.rows;

	Mat C(heighta+heightb,A.cols, CV_8UC3, Scalar(0,0,255));

    uchar* sc = C.data;
	int heightc = C.rows;
	int widthc = C.step.buf[0];

	int tempInt1 = 0;
	int tempInt2 = 0;
	int tempInt3 = 0;

	for (int i=0;i<heighta;i++)
	{
		for (int j=0;j<datanuma;j++)
		{
			*(sc+j+tempInt3) = sa[j+tempInt1];
		}
		tempInt3 += widthc;
		tempInt1 += widtha;
	}
	for (int i=0;i<heightb;i++)
	{
		for (int j=0;j<datanuma;j++)
		{
			*(sc+j+tempInt3) = sb[j+tempInt2];
		}
		tempInt3 += widthc;
		tempInt2 += widthb;
	}
	return C;
}


void basisImgShape(Model& model,Mat& img,shapeXY& shape,_bbox & bbox)
{
	model.dx = 0;
	model.dy = 0;

	vector<int>shapeX;
	vector<int>shapeY;
	for (int i=0;i<_pointNum;i++)
	{
		shapeX.push_back(static_cast<int>(round2(shape.shapeX[i])));
		shapeY.push_back(static_cast<int>(round2(shape.shapeY[i])));
	}

	int min_x = INFINITY;
	int min_y = INFINITY;
	int max_x = -INFINITY;
	int max_y = -INFINITY;

	for (int i=0;i<_pointNum;i++)
	{
		min_x = (shapeX[i]<min_x) ? shapeX[i]:min_x;
		min_y = (shapeY[i]<min_y) ? shapeY[i]:min_y;
		max_x = (shapeX[i]>max_x) ? shapeX[i]:max_x;
		max_y = (shapeY[i]>max_y) ? shapeY[i]:max_y;
	}

	min_x = min_x - 2*_biasNum;
	min_y = min_y - 2*_biasNum;
	max_x = max_x + 2*_biasNum;
	max_y = max_y + 2*_biasNum;

	min_x = (min_x<bbox.rootcoord[0])?min_x:bbox.rootcoord[0];
	min_y = (min_y<bbox.rootcoord[1])?min_y:bbox.rootcoord[1];
	max_x = (max_x>bbox.rootcoord[2])?max_x:bbox.rootcoord[2];
	max_y = (max_y>bbox.rootcoord[3])?max_y:bbox.rootcoord[3];

	int height = img.rows;
	int width = img.cols;

	if (min_x<0)
	{
		int dx = - min_x;
		model.dx = dx;
		for (int i=0;i<_pointNum;i++)
		{
			shapeX[i] = shapeX[i] + dx;
		}
		img = combineMat_x(img.colRange(1,dx+1), img);
	}

	if (min_y<0)
	{
		int dy = - min_y;
		model.dy = dy;

		for (int i=0;i<_pointNum;i++)
		{
			shapeY[i] = shapeY[i] + dy;
		}
		img = combineMat_y(img.rowRange(1,dy+1), img);
	}

	if (max_x>width)
	{
		int dx = max_x - width;
		img = combineMat_x(img,img.colRange(width-dx+1,width+1));
	}

	if (max_y>height)
	{
		int dy = max_y - height;
		img = combineMat_y(img, img.rowRange(height-dy+1,height+1));
	}

	for (int i=0;i<_pointNum;i++)
	{
		shape.shapeX[i] = static_cast<sType>(shapeX[i]);
		shape.shapeY[i] = static_cast<sType>(shapeY[i]);
	}
}

void basisImgShape2(Model& model,Mat& img,shapeXY& shape,_bbox & bbox)
{
	model.dx = 0;
	model.dy = 0;

	vector<int>shapeX;
	vector<int>shapeY;
	for (int i=0;i<_pointNum;i++)
	{
		shapeX.push_back(static_cast<int>(round2(shape.shapeX[i])));
		shapeY.push_back(static_cast<int>(round2(shape.shapeY[i])));
	}

	int min_x = INFINITY;
	int min_y = INFINITY;
	int max_x = -INFINITY;
	int max_y = -INFINITY;

	for (int i=0;i<_pointNum;i++)
	{
		min_x = (shapeX[i]<min_x) ? shapeX[i]:min_x;
		min_y = (shapeY[i]<min_y) ? shapeY[i]:min_y;
		max_x = (shapeX[i]>max_x) ? shapeX[i]:max_x;
		max_y = (shapeY[i]>max_y) ? shapeY[i]:max_y;
	}

	min_x = min_x - addbiasnum*_biasNum;
	min_y = min_y - addbiasnum*_biasNum;
	max_x = max_x + addbiasnum*_biasNum;
	max_y = max_y + addbiasnum*_biasNum;

	int n_width = max_x - min_x + 1;
	int n_hegith = max_y - min_y +1;

	min_x = (min_x<bbox.rootcoord[0])?min_x:bbox.rootcoord[0];
	min_y = (min_y<bbox.rootcoord[1])?min_y:bbox.rootcoord[1];
	max_x = (max_x>bbox.rootcoord[2])?max_x:bbox.rootcoord[2];
	max_y = (max_y>bbox.rootcoord[3])?max_y:bbox.rootcoord[3];

	int height = img.rows;
	int width = img.cols;

	int dx = - min_x;
	model.dx = dx;

	int dy = - min_y;
	model.dy = dy;

	for (int i=0;i<_pointNum;i++)
	{
		shapeX[i] = shapeX[i] + dx;
		shapeY[i] = shapeY[i] + dy;
	}
	
    if (min_x<0)
	{	
		img = combineMat_x(img.colRange(1,dx+1), img);
	}

	if (min_y<0)
	{
		img = combineMat_y(img.rowRange(1,dy+1), img);
	}

	if (max_x>width)
	{
		int dx = max_x - width;
		img = combineMat_x(img,img.colRange(width-dx-1,width-1));
	}

	if (max_y>height)
	{
		int dy = max_y - height;
		img = combineMat_y(img, img.rowRange(height-dy-1,height-1));
	}

	int crop_x1 = min_x>0?min_x:0;
	int crop_y1 = min_y>0?min_y:0;

	img = img(Range(crop_y1,(crop_y1+n_hegith-1)),Range(crop_x1,(crop_x1+n_width-1)));

	for (int i=0;i<_pointNum;i++)
	{
		shape.shapeX[i] = static_cast<sType>(shapeX[i]);
		shape.shapeY[i] = static_cast<sType>(shapeY[i]);
	}
}

void feature_cache(Model& model, Mat & img)
{
	int *p_o = model.best_o_t;
	sType *p_v = model.v_t;

	uchar *  locaMat = model.HOGGradient;

	int width = img.cols;
	int height = img.rows;

	for (int x = 1;x<(width-1);x++)
	{
		for (int y = 1;y<(height-1);y++)
		{
			uchar *s =img.data + x*img.step.buf[1]+ y*img.step.buf[0];
			int dy = *(s+img.step.buf[0]) - *(s-img.step.buf[0]);
			int dx = *(s+img.step.buf[1]) - *(s-img.step.buf[1]);
			sType v = dx*dx + dy*dy;

			s += 1;
			sType dy2 = *(s+img.step.buf[0]) - *(s-img.step.buf[0]);
			sType dx2 = *(s+img.step.buf[1]) - *(s-img.step.buf[1]);
			sType v2 = dx2*dx2 + dy2*dy2;

			s += 1;
			sType dy3 = *(s+img.step.buf[0]) - *(s-img.step.buf[0]);
			sType dx3 = *(s+img.step.buf[1]) - *(s-img.step.buf[1]);
			sType v3 = dx3*dx3 + dy3*dy3;

			if (v2 > v) 
			{
				v = v2;
				dx = dx2;
				dy = dy2;
			} 
			if (v3 > v) 
			{
				v = v3;   
				dx = dx3;
				dy = dy3;
			}
			//best_o = locaMat[(255-dy)*511+(dx+255)];
			//v = sqrt(v);
			p_o[y*width+x] = locaMat[(255-dy)*511+(dx+255)];
			p_v[y*width+x] = sqrt(v);
		}
	}
}

void shapeRegression(Model& model,Mat& img,shapeXY& shape,_bbox& bbox, float &confident_score)
{
	basisImgShape2(model,img,shape,bbox);
#if fastMode
	model.best_o_t = new int[img.rows*img.cols];
	model.v_t = new sType[img.rows*img.cols];
	feature_cache(model, img);
#endif
	int iterNum;
	iterNum = (_iterNum<model.iterNum)?_iterNum:(model.iterNum);
	for (int i=0;i<iterNum;i++)
	{
		if (extractFeature(model,img,shape))
		{
			 updateShape(model,model.featMat,shape,i);
		}
		else
		{
			cout<< "break at the -"<<i<<"-th loop"<<endl;
			break;
		}
	}
	//compute the confident score
	confident_score = 0.0;
	for(int i=0; i<_featDim2; i++)
	{
		confident_score += model.score_w[i]*model.featMat.value[i];
	}
//	cout<<confident_score;
#if fastMode
	delete []model.best_o_t;
	delete []model.v_t;
	model.best_o_t = NULL;
	model.v_t = NULL;
#endif
}

void shapeRegression(Model& model, Mat img,_bbox bbox,shapeXY& shape, float &score, float& roll, float& yaw, float& pitch)
{
	vector<sType>p1;
	calcp1(p1,model.predictbb,bbox);

	sType resizescale = model.meanDE/p1[2];
	Mat img2;
	resize(img,img2,Size(),resizescale,resizescale,INTER_LINEAR);
	img = img2;
	initpts(shape, model.meanShape, p1, resizescale,bbox);

	shapeRegression(model,img,shape,bbox, score);

	for (int i=0;i<_pointNum;i++)
	{
		shape.shapeX[i] = (shape.shapeX[i] - 1- model.dx)/resizescale;
		shape.shapeY[i] = (shape.shapeY[i] - 1- model.dy)/resizescale;  
	}
	//pose estimation
	egp_NodePtrArr *nodes = new egp_NodePtrArr[_pointNum];
	for(int i=0; i<_pointNum; i++)
	{
		nodes[i].x = shape.shapeX[i];
		nodes[i].y = shape.shapeY[i];
	}
	PoseEstimation(nodes, _pointNum, &roll, &yaw, &pitch);
	delete []nodes;
}

void shapeRegression2(Model& model, Mat img,_bbox bbox,shapeXY& inputShape, shapeXY& shape, float &score, float& roll, float& yaw, float& pitch)
{
	vector<sType>p1;
	//calcp1(p1,model.predictbb,bbox);
	p1.push_back(0);
	p1.push_back(0);
//	p1.push_back(((bbox.rootcoord[2] - bbox.rootcoord[0])+(bbox.rootcoord[3] - bbox.rootcoord[1]))/2);
	p1.push_back((bbox.rootcoord[3] - bbox.rootcoord[1]));

	sType resizescale = model.meanDE*2/p1[2];
	Mat img2;
	resize(img,img2,Size(),resizescale,resizescale,INTER_LINEAR);
	img = img2;
	initpts2(shape, model.meanShape, inputShape, p1, resizescale,bbox);

	shapeRegression(model,img,shape,bbox, score);

	for (int i=0;i<_pointNum;i++)
	{
		shape.shapeX[i] = (shape.shapeX[i] - model.dx)/resizescale;
		shape.shapeY[i] = (shape.shapeY[i] - model.dy)/resizescale; 
	}

	//pose estimation
	egp_NodePtrArr *nodes = new egp_NodePtrArr[_pointNum];
	for(int i=0; i<_pointNum; i++)
	{
		nodes[i].x = shape.shapeX[i];
		nodes[i].y = shape.shapeY[i];
	}
	PoseEstimation(nodes, _pointNum, &roll, &yaw, &pitch);
	delete []nodes;
}