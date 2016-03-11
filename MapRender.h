#ifndef _MAP_RENDER_H
#define _MAP_RENDER_H

#include "drawlib/drawlib.h"
#include "Regrouper.h"
#include "Transform.h"
#include "Style.h"
#include "LabelEngine.h"

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
	class Style style;

	MapRender(class IDrawLib *output);
	virtual ~MapRender();

	void Render(int zoom, class FeatureStore &featureStore, 
		bool renderObjects, bool outputLabels,
		class ITransform &transform, OrganisedLabels &organisedLabelsOut);
	void RenderLabels(const OrganisedLabels &organisedLabels);
};

#endif //_MAP_RENDER_H
