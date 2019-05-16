#include "stdafx.h"
#include "config.h"
#include "model.h"

class _bbox
{
public:
	sType rootcoord[4];
	sType score;
	int _componentid;
	//sType _partPos[_partnum*4];

	RECT parts[partnum];
	_bbox()
	{
		score = 0.0;
		_componentid = 1;
	}
	~_bbox()
	{}
};
void shapeRegression(Model&, Mat,_bbox,shapeXY&, float&, float&, float &, float &);
void shapeRegression2(Model&, Mat,_bbox,shapeXY&, shapeXY&, float&, float&, float &, float &);