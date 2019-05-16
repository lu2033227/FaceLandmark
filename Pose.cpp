#include "stdafx.h"
#include "Pose.h"
#include "irr/quaternion.h"

#define _USE_MATH_DEFINES
#include <math.h>

typedef struct tagPoint3DF {
	float x;
	float y;
	float z;
} TPoint3DF;

typedef struct tagPointF {
	float x;
	float y;
} TPointF;


const TPoint3DF KPS[] = {
	{-99.712110, 86363.855269, 96279.801425},
	{-57513.767462, -40301.325378, 80472.182318},
	{-38662.386413, -48791.494751, 100783.858839},
	{-19345.311019, -45338.933858, 108324.582419},
	{18272.403180, -45574.086519, 108260.851160},
	{37696.302081, -49140.705153, 100549.237894},
	{56731.460075, -40457.626503, 80323.410893},
	{-43531.934024, -25934.928690, 87251.899025},
	{-32621.201777, -29839.912857, 95503.568964},
	{-19645.061388, -25604.958237, 93105.299869},
	{-31996.553324, -23262.108976, 94162.521865},
	{18359.757712, -25608.428527, 92784.013528},
	{31505.044030, -29887.668513, 95177.568346},
	{42730.126216, -25819.911199, 87187.481291},
	{30973.398262, -23324.306197, 94027.483394},
	{-260.905498, -26333.482714, 111411.218295},
	{-216.724333, -14285.270538, 120398.489336},
	{-110.514720, -2285.393971, 129726.856635},
	{-114.867362, 7999.345890, 131937.237236},
	{-12262.216492, 16963.916996, 109804.534197},
	{-283.630385, 19356.876572, 116415.716655},
	{11582.025998, 16901.190447, 109755.766424},
	{-25777.628406, 41231.679533, 99206.102755},
	{-11375.421993, 33336.679871, 113458.946568},
	{-264.001711, 32509.756660, 117079.454588},
	{10796.356757, 33341.009996, 113397.307754},
	{24596.278729, 41268.219463, 98994.298542},
	{11766.640446, 46866.718139, 110708.944572},
	{-239.078278, 48816.873825, 113861.016110},
	{-12139.792501, 46862.467021, 110875.893581},
	{-379.065877, 37913.094531, 113163.105327},
	{-401.142441, 40866.494577, 112449.691094},
};

// solve Ax = b by Cholesky decomposition
// result is saved in b
static bool solve(float* A, int m, float* b)
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
            return false;
        L[i * m + i] = 1.0f / sqrtf(s);
    }
    
    if( !b )
        return true;
    
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
    
    return true;
}

void AffineP(const TPointF pt2d[], const TPoint3DF pt3d[], int n, float P[])
{
	int i, j, k;
	float A[16], A1[64];

	for (i = 0; i < 16; i++) {
		A[i] = 0.0f;
	}
	for (i = 0; i < 8; i++) {
		P[i] = 0.0f;
	}
	for(i = 0; i < n; i++) {
		A[0] += pt3d[i].x * pt3d[i].x;
		A[1] += pt3d[i].x * pt3d[i].y;
		A[2] += pt3d[i].x * pt3d[i].z;
		A[3] += pt3d[i].x;

		// A[4] = A[1]
		A[5] += pt3d[i].y * pt3d[i].y;
		A[6] += pt3d[i].y * pt3d[i].z;
		A[7] += pt3d[i].y;

		// A[8] = A[2]
		// A[9] = A[6]
		A[10] += pt3d[i].z * pt3d[i].z;
		A[11] += pt3d[i].z;

		// A[12] = A[3]
		// A[13] = A[7]
		// A[14] = A[11]
		A[15] += 1.0f;

		P[0] += pt3d[i].x * pt2d[i].x;
		P[1] += pt3d[i].y * pt2d[i].x;
		P[2] += pt3d[i].z * pt2d[i].x;
		P[3] += pt2d[i].x;
		P[4] += pt3d[i].x * pt2d[i].y;
		P[5] += pt3d[i].y * pt2d[i].y;
		P[6] += pt3d[i].z * pt2d[i].y;
		P[7] += pt2d[i].y;
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

	solve(A1, 8, P);
}

void OrthRTS(const TPointF pt2d[], const TPoint3DF pt3d[], int n, float R[], float t[], float& s)
{
	int i;
	float m2dx, m2dy;
	float m3dx, m3dy, m3dz;

	TPointF* p2d = new TPointF[n];
	TPoint3DF* p3d = new TPoint3DF[n];
	// zero center pt2d and pt3d
	m2dx = 0.0f;
	m2dy = 0.0f;
	m3dx = 0.0f;
	m3dy = 0.0f;
	m3dz = 0.0f;
	for (i = 0; i < n; i++) {
		m2dx += pt2d[i].x;
		m2dy += pt2d[i].y;

		m3dx += pt3d[i].x;
		m3dy += pt3d[i].y;
		m3dz += pt3d[i].z;
	}
	m2dx /= n;
	m2dy /= n;
	m3dx /= n;
	m3dy /= n;
	m3dz /= n;
	for (i = 0; i < n; i++) {
		p2d[i].x = pt2d[i].x - m2dx;
		p2d[i].y = pt2d[i].y - m2dy;

		p3d[i].x = pt3d[i].x - m3dx;
		p3d[i].y = pt3d[i].y - m3dy;
		p3d[i].z = pt3d[i].z - m3dz;
	}

	float P[8];
	AffineP(p2d, p3d, n, P);

	// ortho-normalization
	float ac1 = 0.0f, ac2 = 0.0f, cc = 0.0f;
	for (i = 0; i < 3; i++) {
		ac1 += P[i] * P[i];
		ac2 += P[4 + i] * P[4 + i];
		cc += P[i] * P[4 + i];
	}
	float d1 = sqrtf(ac1 * ac2 - cc * cc);
	float d2 = ac1 * ac2 + sqrtf(ac1 * ac2) * d1 - cc * cc;
	float a, b, c;
	a = (sqrtf(ac1) + sqrtf(ac2)) / (2 * sqrtf(ac1)) + sqrtf(ac2) * cc * cc / (2 * sqrtf(ac1) * d2);
	b = cc / (2 * d1);
	c = (sqrtf(ac1) + sqrtf(ac2)) / (2 * sqrtf(ac2)) + sqrtf(ac1) * cc * cc / (2 * sqrtf(ac2) * d2);
	s = 0.0f;
	for (i = 0; i < 3; i++) {
		R[i] = a * P[i] - b * P[4 + i];
		R[3 + i] = -b * P[i] + c * P[4 + i];
		s += R[i] * R[i];
	}
	s = sqrtf(s);
	for (i = 0; i < 6; i++) {
		R[i] /= s;
	}
	// cross product
	// 0 1 2
	// 3 4 5
	// 6 7 8
	R[6] = R[1] * R[5] - R[2] * R[4];
	R[7] = R[2] * R[3] - R[0] * R[5];
	R[8] = R[0] * R[4] - R[1] * R[3];
	// translation
	t[0] = m2dx - s * (R[0] * m3dx + R[1] * m3dy + R[2] * m3dz);
	t[1] = m2dy - s * (R[3] * m3dx + R[4] * m3dy + R[5] * m3dz);
	t[2] = -s * (R[6] * m3dx + R[7] * m3dy + R[8] * m3dz);

	delete[] p2d;
	delete[] p3d;
}

void PoseEstimation(egp_NodePtrArr* nodes, int pt_num, float* roll, float* yaw, float* pitch)
{
	int i;
	int seq;
	float R[9];
	float t[3];
	float s;

	TPointF* pt2d = new TPointF[pt_num];
	TPoint3DF* pt3d = new TPoint3DF[pt_num];

	// get corresponding points
	for (i = 0; i < pt_num; i++) {
		pt2d[i].x = nodes[i].x;
		pt2d[i].y = nodes[i].y;

		seq = i;
		pt3d[i] = KPS[seq];
	}

	// estimate pose
	OrthRTS(pt2d, pt3d, pt_num, R, t, s);

	delete[] pt3d;
	delete[] pt2d;

	// rotation matrix to Euler angle
	irr::core::matrix4 r;
	float* data = (float*)r.pointer();
	data[0] = R[0]; data[1] = R[3]; data[2] = R[6];
	data[4] = R[1]; data[5] = R[4]; data[6] = R[7];
	data[8] = R[2]; data[9] = R[5]; data[10] = R[8];
	irr::core::quaternion _r = r;

	irr::core::vector3df angles;
	_r.toEuler(angles);
	*roll = angles.Z * 180.0f / M_PI;
	*yaw = angles.Y * 180.0f / M_PI;
	*pitch = angles.X * 180.0f / M_PI;

}
