#ifndef _MAP_RENDER_H
#define _MAP_RENDER_H

#include "drawlib/drawlib.h"
#include "Regrouper.h"
#include "Transform.h"
#include "Style.h"

class MapRender
{
public:
	class IDrawLib *output;
	double extentx1, extenty1, extentx2, extenty2;
	double width, height;
	class Style style;

	MapRender(class IDrawLib *output);
	virtual ~MapRender();

	void Render(int layerNum, class FeatureStore &featureStore, class ITransform &transform);
	void ToDrawSpace(double nx, double ny, double &px, double &py);
};

#endif //_MAP_RENDER_H
