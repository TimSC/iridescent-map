#ifndef _MAP_RENDER_H
#define _MAP_RENDER_H

#include "MapRenderTypes.h"
#include "Transform.h"
#include "Style.h"
#include "Coast.h"
#include <string>

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
	CoastMap *coastMap; //Borrowed reference
	int x, y, zoom; //Coordinates of tile canvas to draw
	int datax, datay, dataZoom; //Coordinates of source data being rendered
	std::string resourceFilePath;

	MapRender(class IDrawLib *output, int x, int y, int zoom, int datax, int datay, int dataZoom, const char *resourceFilePathIn);
	virtual ~MapRender();

	///This (optionally) draws map shapes on to the drawing surface and (optionally) outputs an LabelsByImportance object.
	void Render(int zoom, class FeatureStore &featureStore, 
		bool renderObjects, bool outputLabels,
		LabelsByImportance &organisedLabelsOut);

	///This renders labels on to a drawing surface for a tile based on its own labels and the labels of surrounding tiles.
	void RenderLabels(const RenderLabelList &labelList,
		const RenderLabelListOffsets &labelOffsets);

	void SetCoastMap(CoastMap &coastMap);
};

#endif //_MAP_RENDER_H
