#include "stdafx.h"
#include "featpyramid.h"

#define eps 0.0001

sType round0(sType r) {
	return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}

sType uu[9] = {1.0000, 0.9397, 0.7660, 0.500, 0.1736, -0.1736, -0.5000, -0.7660, -0.9397};
sType vv[9] = {0.0000, 0.3420, 0.6428, 0.8660, 0.9848, 0.9848, 0.8660, 0.6428, 0.3420};

sType mins(sType x, sType y) 
{ 
	return (x <= y ? x : y); 
}
sType maxs(sType x, sType y) 
{ 
	return (x <= y ? y : x); 
}
int mins(int x, int y) 
{ 
	return (x <= y ? x : y); 
}
int maxs(int x, int y) 
{ 
	return (x <= y ? y : x); 
}

void features(const Mat& img,_feat& feat,int bins)
{
	int dims[3];
	dims[0]=img.rows;
	dims[1]=img.cols;
	dims[2]=img.channels();
	int blocks[2];
	blocks[0] = (int)round0((sType)dims[0]/(sType)bins);
	blocks[1] = (int)round0((sType)dims[1]/(sType)bins);
	sType *hist = new sType[blocks[0]*blocks[1]*18];
	sType *norm = new sType[blocks[0]*blocks[1]];
	memset(hist,0,blocks[0]*blocks[1]*18*sizeof(sType));
	memset(norm,0,blocks[0]*blocks[1]*sizeof(sType));

	int out[3];
	out[0] = maxs(blocks[0]-2, 0);
	out[1] = maxs(blocks[1]-2, 0);
	out[2] = 27+4+1;
// 	for (int i=0;i<3;i++)
// 	{
// 		feat.size[i]=out[i];
// 	}
// 
// 	feat.ft=new sType[out[0]*out[1]*out[2]];

	int visible[2];
	visible[0] = blocks[0]*bins;
	visible[1] = blocks[1]*bins;

	int buf[2];
	buf[0] = img.step.buf[0];
	buf[1] = img.step.buf[1];
	for (int x = 1; x < visible[1]-1; x++) {
		for (int y = 1; y < visible[0]-1; y++) {
			uchar *s =img.data + min(x, dims[1]-2)*buf[1]+ min(y, dims[0]-2)*buf[0] ;
			int dy = *(s+buf[0]) - *(s-buf[0]);
			int dx = *(s+buf[1]) - *(s-buf[1]);
			sType v = dx*dx + dy*dy;

			s +=1;
			sType dy2 = *(s+buf[0]) - *(s-buf[0]);
			sType dx2 = *(s+buf[1]) - *(s-buf[1]);
			sType v2 = dx2*dx2 + dy2*dy2;

			s += 1;
			sType dy3 = *(s+buf[0]) - *(s-buf[0]);
			sType dx3 = *(s+buf[1]) - *(s-buf[1]);
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

			sType best_dot = 0;
			int best_o = 0;
			for (int o = 0; o < 9; o++) 
			{
				sType dot = uu[o]*dx + vv[o]*dy;
				if (dot > best_dot)
				{
					best_dot = dot;
					best_o = o;
				} else if (-dot > best_dot) 
				{
					best_dot = -dot;
					best_o = o+9;
				}
			}

			sType xp = ((sType)x+0.5)/(sType)bins - 0.5;
			sType yp = ((sType)y+0.5)/(sType)bins - 0.5;
			int ixp = (int)floor(xp);
			int iyp = (int)floor(yp);
			sType vx0 = xp-ixp;
			sType vy0 = yp-iyp;
			sType vx1 = 1.0-vx0;
			sType vy1 = 1.0-vy0;
			v = sqrt(v);

			if (ixp >= 0 && iyp >= 0) 
			{
				*(hist + ixp*blocks[0] + iyp + best_o*blocks[0]*blocks[1]) += 
					vx1*vy1*v;
			}

			if (ixp+1 < blocks[1] && iyp >= 0)
			{
				*(hist + (ixp+1)*blocks[0] + iyp + best_o*blocks[0]*blocks[1]) += 
					vx0*vy1*v;
			}

			if (ixp >= 0 && iyp+1 < blocks[0]) 
			{
				*(hist + ixp*blocks[0] + (iyp+1) + best_o*blocks[0]*blocks[1]) += 
					vx1*vy0*v;
			}

			if (ixp+1 < blocks[1] && iyp+1 < blocks[0]) 
			{
				*(hist + (ixp+1)*blocks[0] + (iyp+1) + best_o*blocks[0]*blocks[1]) += 
					vx0*vy0*v;
			}
		}
	}

	for (int o = 0; o < 9; o++) 
	{
		sType *src1 = hist + o*blocks[0]*blocks[1];
		sType *src2 = hist + (o+9)*blocks[0]*blocks[1];
		sType *dst = norm;
		sType *end = norm + blocks[1]*blocks[0];
		while (dst < end) 
		{
			*(dst++) += (*src1 + *src2) * (*src1 + *src2);
			src1++;
			src2++;
		}
	}

	for (int x = 0; x < out[1]; x++) 
	{
		for (int y = 0; y < out[0]; y++) 
		{
			sType *dst = feat.ft + x*out[0] + y;      
			sType *src, *p, n1, n2, n3, n4;

			p = norm + (x+1)*blocks[0] + y+1;
			n1 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
			p = norm + (x+1)*blocks[0] + y;
			n2 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
			p = norm + x*blocks[0] + y+1;
			n3 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
			p = norm + x*blocks[0] + y;      
			n4 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);

			sType t1 = 0;
			sType t2 = 0;
			sType t3 = 0;
			sType t4 = 0;

			src = hist + (x+1)*blocks[0] + (y+1);
			for (int o = 0; o < 18; o++) 
			{
				sType h1 = mins(*src * n1, 0.2);
				sType h2 = mins(*src * n2, 0.2);
				sType h3 = mins(*src * n3, 0.2);
				sType h4 = mins(*src * n4, 0.2);
				*dst = 0.5 * (h1 + h2 + h3 + h4);
				t1 += h1;
				t2 += h2;
				t3 += h3;
				t4 += h4;
				dst += out[0]*out[1];
				src += blocks[0]*blocks[1];
			}

			src = hist + (x+1)*blocks[0] + (y+1);
			for (int o = 0; o < 9; o++) 
			{
				sType sum = *src + *(src + 9*blocks[0]*blocks[1]);
				sType h1 = mins(sum * n1, 0.2);
				sType h2 = mins(sum * n2, 0.2);
				sType h3 = mins(sum * n3, 0.2);
				sType h4 = mins(sum * n4, 0.2);
				*dst = 0.5 * (h1 + h2 + h3 + h4);
				dst += out[0]*out[1];
				src += blocks[0]*blocks[1];
			}

			*dst = 0.2357 * t1;
			dst += out[0]*out[1];
			*dst = 0.2357 * t2;
			dst += out[0]*out[1];
			*dst = 0.2357 * t3;
			dst += out[0]*out[1];
			*dst = 0.2357 * t4;

			dst += out[0]*out[1];
			*dst = 0;
		}
	}
	delete []hist;
	delete []norm;
}

void features_lookup(const Mat& img,_feat& feat,int bins, uchar* locaMat)
{
	int dims[3];
	dims[0]=img.rows;
	dims[1]=img.cols;
	dims[2]=img.channels();
	int blocks[2];
	blocks[0] = (int)round0((sType)dims[0]/(sType)bins);
	blocks[1] = (int)round0((sType)dims[1]/(sType)bins);
	sType *hist = new sType[blocks[0]*blocks[1]*18];
	sType *norm = new sType[blocks[0]*blocks[1]];
	memset(hist,0,blocks[0]*blocks[1]*18*sizeof(sType));
	memset(norm,0,blocks[0]*blocks[1]*sizeof(sType));

	int out[3];
	out[0] = maxs(blocks[0]-2, 0);
	out[1] = maxs(blocks[1]-2, 0);
	out[2] = 27+4+1;

	int visible[2];
	visible[0] = blocks[0]*bins;
	visible[1] = blocks[1]*bins;

	int ixp = -1;
	int iyp = -1;
	sType vx0 = 0;
	sType vy0 = 0;

	int numBinX = 0;
	int numBinY = 0;

	int best_o;
	for (int x = 1; x < visible[1]-1; x++) 
	{
		for (int y = 1; y < visible[0]-1; y++) 
		{
			uchar *s =img.data + min(x, dims[1]-2)*img.step.buf[1]+ min(y, dims[0]-2)*img.step.buf[0] ;
			int dy = *(s+img.step.buf[0]) - *(s-img.step.buf[0]);
			int dx = *(s+img.step.buf[1]) - *(s-img.step.buf[1]);
			sType v = dx*dx + dy*dy;

			s +=1;
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

			best_o = locaMat[(255-dy)*511+(dx+255)];
            v = sqrt(v);

			sType xp = ((sType)x+0.5)/(sType)bins - 0.5;
			sType yp = ((sType)y+0.5)/(sType)bins - 0.5;
			int ixp = (int)floor(xp);
			int iyp = (int)floor(yp);
			sType vx0 = xp-ixp;
			sType vy0 = yp-iyp;
			sType vx1 = 1.0-vx0;
			sType vy1 = 1.0-vy0;

			if (ixp >= 0 && iyp >= 0) 
			{
				*(hist + ixp*blocks[0] + iyp + best_o*blocks[0]*blocks[1]) += vx1*vy1*v;
			}

			if (ixp+1 < blocks[1] && iyp >= 0)
			{
				*(hist + (ixp+1)*blocks[0] + iyp + best_o*blocks[0]*blocks[1]) += vx0*vy1*v;
			}

			if (ixp >= 0 && iyp+1 < blocks[0]) 
			{
				*(hist + ixp*blocks[0] + (iyp+1) + best_o*blocks[0]*blocks[1]) += vx1*vy0*v;
			}

			if (ixp+1 < blocks[1] && iyp+1 < blocks[0]) 
			{
				*(hist + (ixp+1)*blocks[0] + (iyp+1) + best_o*blocks[0]*blocks[1]) += vx0*vy0*v;
			}
		}
	}

	for (int o = 0; o < 9; o++) 
	{
		sType *src1 = hist + o*blocks[0]*blocks[1];
		sType *src2 = hist + (o+9)*blocks[0]*blocks[1];
		sType *dst = norm;
		sType *end = norm + blocks[1]*blocks[0];
		while (dst < end) 
		{
			*(dst++) += (*src1 + *src2) * (*src1 + *src2);
			src1++;
			src2++;
		}
	}

	for (int x = 0; x < out[1]; x++) 
	{
		for (int y = 0; y < out[0]; y++) 
		{
			sType *dst = feat.ft + x*out[0] + y;      
			sType *src, *p, n1, n2, n3, n4;

			p = norm + (x+1)*blocks[0] + y+1;
			n1 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
			p = norm + (x+1)*blocks[0] + y;
			n2 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
			p = norm + x*blocks[0] + y+1;
			n3 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
			p = norm + x*blocks[0] + y;      
			n4 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);

			sType t1 = 0;
			sType t2 = 0;
			sType t3 = 0;
			sType t4 = 0;

			src = hist + (x+1)*blocks[0] + (y+1);
			for (int o = 0; o < 18; o++) 
			{
				sType h1 = mins(*src * n1, 0.2);
				sType h2 = mins(*src * n2, 0.2);
				sType h3 = mins(*src * n3, 0.2);
				sType h4 = mins(*src * n4, 0.2);
				*dst = 0.5 * (h1 + h2 + h3 + h4);
				t1 += h1;
				t2 += h2;
				t3 += h3;
				t4 += h4;
				dst += out[0]*out[1];
				src += blocks[0]*blocks[1];
			}

			src = hist + (x+1)*blocks[0] + (y+1);
			for (int o = 0; o < 9; o++) 
			{
				sType sum = *src + *(src + 9*blocks[0]*blocks[1]);
				sType h1 = mins(sum * n1, 0.2);
				sType h2 = mins(sum * n2, 0.2);
				sType h3 = mins(sum * n3, 0.2);
				sType h4 = mins(sum * n4, 0.2);
				*dst = 0.5 * (h1 + h2 + h3 + h4);
				dst += out[0]*out[1];
				src += blocks[0]*blocks[1];
			}

			*dst = 0.2357 * t1;
			dst += out[0]*out[1];
			*dst = 0.2357 * t2;
			dst += out[0]*out[1];
			*dst = 0.2357 * t3;
			dst += out[0]*out[1];
			*dst = 0.2357 * t4;

			dst += out[0]*out[1];
			*dst = 0;
		}
	}
	delete []hist;
	delete []norm;
}

void features_lookup(_feat& feat,int bins, int* best_o_t, sType* v_t, int x_min,int y_min,int buf)
{
	int blocks[2]={3,3};

	sType *hist = new sType[blocks[0]*blocks[1]*18];
	sType *norm = new sType[blocks[0]*blocks[1]];
	memset(hist,0,blocks[0]*blocks[1]*18*sizeof(sType));
	memset(norm,0,blocks[0]*blocks[1]*sizeof(sType));

	int out[3];
	out[0] = 1;
	out[1] = 1;
	out[2] = 32;

	int visible[2];
	visible[0] = blocks[0]*bins;
	visible[1] = blocks[1]*bins;

	int ixp = -1;
	int iyp = -1;
	sType vx0 = 0;
	sType vy0 = 0;

	int numBinX = 0;
	int numBinY = 0;

	int best_o;
	sType v;
	int tindex  = 0;

	for (int x = 1; x < visible[1]-1; x++) 
	{
		for (int y = 1; y < visible[0]-1; y++) 
		{
			tindex = (y+y_min)*buf+x+x_min;

			best_o = best_o_t[tindex];
			v = v_t[tindex];

			sType xp = ((sType)x+0.5)/(sType)bins - 0.5;
			sType yp = ((sType)y+0.5)/(sType)bins - 0.5;
			int ixp = (int)floor(xp);
			int iyp = (int)floor(yp);
			sType vx0 = xp-ixp;
			sType vy0 = yp-iyp;
			sType vx1 = 1.0-vx0;
			sType vy1 = 1.0-vy0;

			if (ixp >= 0 && iyp >= 0) 
			{
				*(hist + ixp*blocks[0] + iyp + best_o*blocks[0]*blocks[1]) += vx1*vy1*v;
			}

			if (ixp+1 < blocks[1] && iyp >= 0)
			{
				*(hist + (ixp+1)*blocks[0] + iyp + best_o*blocks[0]*blocks[1]) += vx0*vy1*v;
			}

			if (ixp >= 0 && iyp+1 < blocks[0]) 
			{
				*(hist + ixp*blocks[0] + (iyp+1) + best_o*blocks[0]*blocks[1]) += vx1*vy0*v;
			}

			if (ixp+1 < blocks[1] && iyp+1 < blocks[0]) 
			{
				*(hist + (ixp+1)*blocks[0] + (iyp+1) + best_o*blocks[0]*blocks[1]) += vx0*vy0*v;
			}
		}
	}

	for (int o = 0; o < 9; o++) 
	{
		sType *src1 = hist + o*blocks[0]*blocks[1];
		sType *src2 = hist + (o+9)*blocks[0]*blocks[1];
		sType *dst = norm;
		sType *end = norm + blocks[1]*blocks[0];
		while (dst < end) 
		{
			*(dst++) += (*src1 + *src2) * (*src1 + *src2);
			src1++;
			src2++;
		}
	}

	for (int x = 0; x < out[1]; x++) 
	{
		for (int y = 0; y < out[0]; y++) 
		{
			sType *dst = feat.ft + x*out[0] + y;      
			sType *src, *p, n1, n2, n3, n4;

			p = norm + (x+1)*blocks[0] + y+1;
			n1 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
			p = norm + (x+1)*blocks[0] + y;
			n2 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
			p = norm + x*blocks[0] + y+1;
			n3 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
			p = norm + x*blocks[0] + y;      
			n4 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);

			sType t1 = 0;
			sType t2 = 0;
			sType t3 = 0;
			sType t4 = 0;

			src = hist + (x+1)*blocks[0] + (y+1);
			for (int o = 0; o < 18; o++) 
			{
				sType h1 = mins(*src * n1, 0.2);
				sType h2 = mins(*src * n2, 0.2);
				sType h3 = mins(*src * n3, 0.2);
				sType h4 = mins(*src * n4, 0.2);
				*dst = 0.5 * (h1 + h2 + h3 + h4);
				t1 += h1;
				t2 += h2;
				t3 += h3;
				t4 += h4;
				dst += out[0]*out[1];
				src += blocks[0]*blocks[1];
			}

			src = hist + (x+1)*blocks[0] + (y+1);
			for (int o = 0; o < 9; o++) 
			{
				sType sum = *src + *(src + 9*blocks[0]*blocks[1]);
				sType h1 = mins(sum * n1, 0.2);
				sType h2 = mins(sum * n2, 0.2);
				sType h3 = mins(sum * n3, 0.2);
				sType h4 = mins(sum * n4, 0.2);
				*dst = 0.5 * (h1 + h2 + h3 + h4);
				dst += out[0]*out[1];
				src += blocks[0]*blocks[1];
			}

			*dst = 0.2357 * t1;
			dst += out[0]*out[1];
			*dst = 0.2357 * t2;
			dst += out[0]*out[1];
			*dst = 0.2357 * t3;
			dst += out[0]*out[1];
			*dst = 0.2357 * t4;

			dst += out[0]*out[1];
			*dst = 0;
		}
	}
	delete []hist;
	delete []norm;
}


void features_lookup(_feat& feat,int bins, int* best_o_t, sType* v_t, int x_min,int y_min,int buf,_vxvy_r &vxvy)
{
	int blocks[2]={3,3};

	sType *hist = new sType[blocks[0]*blocks[1]*18];
	sType *norm = new sType[blocks[0]*blocks[1]];
	memset(hist,0,blocks[0]*blocks[1]*18*sizeof(sType));
	memset(norm,0,blocks[0]*blocks[1]*sizeof(sType));

	int out[3];
	out[0] = 1;
	out[1] = 1;
	out[2] = 32;

	int visible[2];
	visible[0] = blocks[0]*bins;
	visible[1] = blocks[1]*bins;

	int ixp = -1;
	int iyp = -1;
	sType vx0 = 0;
	sType vy0 = 0;

	int numBinX = 0;
	int numBinY = 0;

	int best_o;
	sType v;
	int tindex  = 0;

	int biasx = 0;
	int biasy = 0;
	int biasX = 0;
	int biasY = 0;
	int block01 = blocks[0]*blocks[1];
	for (int x = 1; x < visible[1]-1; x++) 
	{
		biasx += 1;
		if (biasx==vxvy.patchwidth)
		{
			biasx = 0;
			biasX = biasX+1;
		}
		biasy = 0;
		biasY = 0;
		for (int y = 1; y < visible[0]-1; y++) 
		{
			biasy += 1;
			if (biasy==vxvy.patchwidth)
			{
				biasy = 0;
				biasY = biasY+1;
			}

			tindex = (y+y_min)*buf+x+x_min;

			best_o = best_o_t[tindex];
			v = v_t[tindex];

			ixp = vxvy.ixp[biasx]+biasX;
			iyp = vxvy.iyp[biasy]+biasY;

			if (ixp >= 0 && iyp >= 0) 
			{
				*(hist + ixp*blocks[0] + iyp + best_o*block01) += vxvy.vx1vy1[biasy][biasx]*v;
			}

			if (ixp+1 < blocks[1] && iyp >= 0)
			{
				*(hist + (ixp+1)*blocks[0] + iyp + best_o*block01) += vxvy.vx0vy1[biasy][biasx]*v;
			}

			if (ixp >= 0 && iyp+1 < blocks[0]) 
			{
 				*(hist + ixp*blocks[0] + (iyp+1) + best_o*block01) += vxvy.vx1vy0[biasy][biasx]*v;
			}

			if (ixp+1 < blocks[1] && iyp+1 < blocks[0]) 
			{
				*(hist + (ixp+1)*blocks[0] + (iyp+1) + best_o*block01) += vxvy.vx0vy0[biasy][biasx]*v;
			}
		}
	}

	for (int o = 0; o < 9; o++) 
	{
		sType *src1 = hist + o*block01;
		sType *src2 = hist + (o+9)*block01;
		sType *dst = norm;
		sType *end = norm + block01;
		while (dst < end) 
		{
			*(dst++) += (*src1 + *src2) * (*src1 + *src2);
			src1++;
			src2++;
		}
	}

	for (int x = 0; x < out[1]; x++) 
	{
		for (int y = 0; y < out[0]; y++) 
		{
			sType *dst = feat.ft + x*out[0] + y;      
			sType *src, *p, n1, n2, n3, n4;

			p = norm + (x+1)*blocks[0] + y+1;
			n1 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
			p = norm + (x+1)*blocks[0] + y;
			n2 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
			p = norm + x*blocks[0] + y+1;
			n3 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
			p = norm + x*blocks[0] + y;      
			n4 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);

			sType t1 = 0;
			sType t2 = 0;
			sType t3 = 0;
			sType t4 = 0;

			src = hist + (x+1)*blocks[0] + (y+1);
			for (int o = 0; o < 18; o++) 
			{
				sType h1 = mins(*src * n1, 0.2);
				sType h2 = mins(*src * n2, 0.2);
				sType h3 = mins(*src * n3, 0.2);
				sType h4 = mins(*src * n4, 0.2);
				*dst = 0.5 * (h1 + h2 + h3 + h4);
				t1 += h1;
				t2 += h2;
				t3 += h3;
				t4 += h4;
				dst += out[0]*out[1];
				src += block01;
			}

			src = hist + (x+1)*blocks[0] + (y+1);
			for (int o = 0; o < 9; o++) 
			{
				sType sum = *src + *(src + 9*block01);
				sType h1 = mins(sum * n1, 0.2);
				sType h2 = mins(sum * n2, 0.2);
				sType h3 = mins(sum * n3, 0.2);
				sType h4 = mins(sum * n4, 0.2);
				*dst = 0.5 * (h1 + h2 + h3 + h4);
				dst += out[0]*out[1];
				src += block01;
			}

			*dst = 0.2357 * t1;
			dst += out[0]*out[1];
			*dst = 0.2357 * t2;
			dst += out[0]*out[1];
			*dst = 0.2357 * t3;
			dst += out[0]*out[1];
			*dst = 0.2357 * t4;

			dst += out[0]*out[1];
			*dst = 0;
		}
	}
	delete []hist;
	delete []norm;
}


void features_lookup(_feat& feat,int bins, int* best_o_t, sType* v_t, int x_min,int y_min,int buf,_vxvy_p &vxvy)
{
	int blocks[2]={3,3};

	sType *hist = new sType[blocks[0]*blocks[1]*18];
	sType *norm = new sType[blocks[0]*blocks[1]];
	memset(hist,0,blocks[0]*blocks[1]*18*sizeof(sType));
	memset(norm,0,blocks[0]*blocks[1]*sizeof(sType));

	int out[3];
	out[0] = 1;
	out[1] = 1;
	out[2] = 32;

	int visible[2];
	visible[0] = blocks[0]*bins;
	visible[1] = blocks[1]*bins;

	int ixp = -1;
	int iyp = -1;
	sType vx0 = 0;
	sType vy0 = 0;

	int numBinX = 0;
	int numBinY = 0;

	int best_o;
	sType v;
	int tindex  = 0;

	int biasx = 0;
	int biasy = 0;
	int biasX = 0;
	int biasY = 0;
	int block01 = blocks[0]*blocks[1];
	for (int x = 1; x < visible[1]-1; x++) 
	{
		biasx += 1;
		if (biasx==vxvy.patchwidth)
		{
			biasx = 0;
			biasX = biasX+1;
		}
		biasy = 0;
		biasY = 0;
		for (int y = 1; y < visible[0]-1; y++) 
		{
			biasy += 1;
			if (biasy==vxvy.patchwidth)
			{
				biasy = 0;
				biasY = biasY+1;
			}

			tindex = (y+y_min)*buf+x+x_min;

			best_o = best_o_t[tindex];
			v = v_t[tindex];

			ixp = vxvy.ixp[biasx]+biasX;
			iyp = vxvy.iyp[biasy]+biasY;

			if (ixp >= 0 && iyp >= 0) 
			{
				*(hist + ixp*blocks[0] + iyp + best_o*block01) += vxvy.vx1vy1[biasy][biasx]*v;
			}

			if (ixp+1 < blocks[1] && iyp >= 0)
			{
				*(hist + (ixp+1)*blocks[0] + iyp + best_o*block01) += vxvy.vx0vy1[biasy][biasx]*v;
			}

			if (ixp >= 0 && iyp+1 < blocks[0]) 
			{
				*(hist + ixp*blocks[0] + (iyp+1) + best_o*block01) += vxvy.vx1vy0[biasy][biasx]*v;
			}

			if (ixp+1 < blocks[1] && iyp+1 < blocks[0]) 
			{
				*(hist + (ixp+1)*blocks[0] + (iyp+1) + best_o*block01) += vxvy.vx0vy0[biasy][biasx]*v;
			}
		}
	}

	for (int o = 0; o < 9; o++) 
	{
		sType *src1 = hist + o*block01;
		sType *src2 = hist + (o+9)*block01;
		sType *dst = norm;
		sType *end = norm + block01;
		while (dst < end) 
		{
			*(dst++) += (*src1 + *src2) * (*src1 + *src2);
			src1++;
			src2++;
		}
	}

	for (int x = 0; x < out[1]; x++) 
	{
		for (int y = 0; y < out[0]; y++) 
		{
			sType *dst = feat.ft + x*out[0] + y;      
			sType *src, *p, n1, n2, n3, n4;

			p = norm + (x+1)*blocks[0] + y+1;
			n1 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
			p = norm + (x+1)*blocks[0] + y;
			n2 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
			p = norm + x*blocks[0] + y+1;
			n3 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);
			p = norm + x*blocks[0] + y;      
			n4 = 1.0 / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + eps);

			sType t1 = 0;
			sType t2 = 0;
			sType t3 = 0;
			sType t4 = 0;

			src = hist + (x+1)*blocks[0] + (y+1);
			for (int o = 0; o < 18; o++) 
			{
				sType h1 = mins(*src * n1, 0.2);
				sType h2 = mins(*src * n2, 0.2);
				sType h3 = mins(*src * n3, 0.2);
				sType h4 = mins(*src * n4, 0.2);
				*dst = 0.5 * (h1 + h2 + h3 + h4);
				t1 += h1;
				t2 += h2;
				t3 += h3;
				t4 += h4;
				dst += out[0]*out[1];
				src += block01;
			}

			src = hist + (x+1)*blocks[0] + (y+1);
			for (int o = 0; o < 9; o++) 
			{
				sType sum = *src + *(src + 9*block01);
				sType h1 = mins(sum * n1, 0.2);
				sType h2 = mins(sum * n2, 0.2);
				sType h3 = mins(sum * n3, 0.2);
				sType h4 = mins(sum * n4, 0.2);
				*dst = 0.5 * (h1 + h2 + h3 + h4);
				dst += out[0]*out[1];
				src += block01;
			}

			*dst = 0.2357 * t1;
			dst += out[0]*out[1];
			*dst = 0.2357 * t2;
			dst += out[0]*out[1];
			*dst = 0.2357 * t3;
			dst += out[0]*out[1];
			*dst = 0.2357 * t4;

			dst += out[0]*out[1];
			*dst = 0;
		}
	}
	delete []hist;
	delete []norm;
}
