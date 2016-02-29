#ifndef _MAP_RENDER_H
#define _MAP_RENDER_H

#include "drawlib/drawlib.h"
#include "Regrouper.h"
#include "Transform.h"
#include "Style.h"

typedef std::map<class ShapeProperties, std::vector<Polygon> > StyledPolygons;
typedef std::map<class LineProperties, Contours> StyledLines;
typedef std::map<class TextProperties, std::vector<class TextLabel> > StyledText;

class DrawTreeNode
{
public:
	StyledPolygons styledPolygons;
	StyledLines styledLines;
	StyledText styledText;
	
	std::map<int, class DrawTreeNode> children;

	DrawTreeNode();
	DrawTreeNode(const class DrawTreeNode &a);
	virtual ~DrawTreeNode();

	void WriteDrawCommands(class IDrawLib *output);
	class DrawTreeNode *GetLayer(LayerDef &layerDef, int depth = 0);
};

class MapRender
{
public:
	class IDrawLib *output;
	double extentx1, extenty1, extentx2, extenty2;
	double width, height;
	class Style style;

	MapRender(class IDrawLib *output);
	virtual ~MapRender();

	void Render(int zoom, class FeatureStore &featureStore, class ITransform &transform);
	void ToDrawSpace(double nx, double ny, double &px, double &py);
	int ColourStringToRgb(const char *colStr, double &r, double &g, double &b);
};

#endif //_MAP_RENDER_H
