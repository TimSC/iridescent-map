#ifndef _MAP_RENDER_H
#define _MAP_RENDER_H

#include "MapRenderTypes.h"
#include "Transform.h"
#include "Style.h"

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

	///This (optionally) draws map shapes on to the drawing surface and (optionally) outputs an LabelsByImportance object.
	void Render(int zoom, class FeatureStore &featureStore, 
		bool renderObjects, bool outputLabels,
		class ITransform &transform, LabelsByImportance &organisedLabelsOut);

	///This renders labels on to a drawing surface for a tile based on its own labels and the labels of surrounding tiles.
	void RenderLabels(const RenderLabelList &labelList,
		const RenderLabelListOffsets &labelOffsets);
};

#endif //_MAP_RENDER_H
