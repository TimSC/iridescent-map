///\file
#include "MapRender.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <algorithm>
#include <stdio.h>
#include "LabelEngine.h"
#include "CompletePoly.h"
using namespace std;

void StripIdsFromContour(const ContourWithIds &line, Contour &lineNoIds)
{
	lineNoIds.resize(line.size());
	for(unsigned int j=0; j< line.size(); j++)
		lineNoIds[j] = line[j].second;
}

void StripIdsFromContours(const ContoursWithIds &contoursWithIds, Contours &contoursNoIds)
{
	contoursNoIds.resize(contoursWithIds.size());
	for(unsigned int j=0; j< contoursWithIds.size(); j++)
		StripIdsFromContour(contoursWithIds[j], contoursNoIds[j]);
}

void StripIdsFromPolygon(const PolygonWithIds &polygonWithIds, Polygon &polygonNoIds)
{
	StripIdsFromContour(polygonWithIds.first, polygonNoIds.first);
	StripIdsFromContours(polygonWithIds.second, polygonNoIds.second);
}

void StripIdsFromPolygons(const std::vector<PolygonWithIds> &polygonsWithIds, std::vector<Polygon> &polygons)
{
	polygons.resize(polygonsWithIds.size());
	for(unsigned int i=0; i< polygonsWithIds.size(); i++)
		StripIdsFromPolygon(polygonsWithIds[i], polygons[i]);
}

// ***************************************

void MergeContours(const std::vector<ContourWithIds> &contours, bool allowReversing, std::vector<ContourWithIds> &mergedOut)
{
	mergedOut.clear();

	vector<bool> mask;
	mask.resize(contours.size());
	for(size_t i=0; i<mask.size(); i++)
		mask[i] = false;
	
	while(true)
	{
		//Select a way from input
		const ContourWithIds *initial = NULL;
		for(size_t i=0; i<mask.size(); i++)
		{
			if(mask[i]) continue;
			initial = &contours[i];
			mask[i] = true;
			break;
		}
		if(initial == NULL) break;
		ContourWithIds current = *initial;
		
		//Try to join other ways to the current one
		while(true)
		{
			bool progressMade = false;
			for(size_t i=0; i<mask.size(); i++)
			{
				if(mask[i]) continue;
				const ContourWithIds chk = contours[i];
				if(chk.size() == 0)
				{
					mask[i] = true;
					continue;
				}

				if(chk[0].first == current[current.size()-1].first)
				{
					current.insert(current.end(), chk.begin()++, chk.end());
					mask[i] = true;
					progressMade = true;
					continue;
				}

				if(chk[chk.size()-1].first == current[0].first)
				{
					current.insert(current.begin(), chk.begin(), chk.end()--);
					mask[i] = true;
					progressMade = true;
					continue;
				}

				if(allowReversing && chk[chk.size()-1].first == current[current.size()-1].first)
				{
					ContourWithIds tmp = chk;
					std::reverse(tmp.begin(), tmp.end());
					current.insert(current.end(), tmp.begin()++, tmp.end());
					mask[i] = true;
					progressMade = true;
					continue;
				}

				if(allowReversing && chk[0].first == current[0].first)
				{
					ContourWithIds tmp = chk;
					std::reverse(tmp.begin(), tmp.end());
					current.insert(current.begin(), tmp.begin(), tmp.end()--);
					mask[i] = true;
					progressMade = true;
					continue;
				}

			}
			if(!progressMade) break;
		}

		mergedOut.push_back(current);
	}
}

// ****************************************

DrawTreeNode::DrawTreeNode()
{

}


DrawTreeNode::DrawTreeNode(const class DrawTreeNode &a)
{
	styledPolygons = a.styledPolygons;
	styledLines = a.styledLines;
	styledText = a.styledText;
	children = a.children;
}

DrawTreeNode::~DrawTreeNode()
{

}

void DrawTreeNode::WriteDrawCommands(class IDrawLib *output)
{
	
	for(std::map<int, class DrawTreeNode>::iterator it = this->children.begin(); it != this->children.end(); it++)
	{
		class DrawTreeNode &child = it->second;
		child.WriteDrawCommands(output);
	}

	for(StyledPolygons::iterator it = this->styledPolygons.begin(); it != styledPolygons.end(); it ++)
	{
		std::vector<Polygon> &polys = it->second;
		const class ShapeProperties &prop = it->first;

		output->AddDrawPolygonsCmd(polys, prop);
	}

	for(StyledLines::iterator it = this->styledLines.begin(); it != this->styledLines.end(); it++)
	{
		Contours &lines = it->second;
		const class LineProperties &prop = it->first;

		output->AddDrawLinesCmd(lines, prop);
	}
}

class DrawTreeNode *DrawTreeNode::GetLayer(LayerDef &layerDef, int depth)
{
	if(layerDef.size() <= depth) return this;
	
	int requestedAddr = layerDef[depth];
	std::map<int, class DrawTreeNode>::iterator it = this->children.find(requestedAddr);
	if(it == this->children.end())
		this->children[requestedAddr] = DrawTreeNode();

	return this->children[requestedAddr].GetLayer(layerDef, depth+1);
}

// **************************************************

class IFeatureConverterResult
{
public:
	virtual void OutArea(StyleDef &styleDef, const std::vector<PolygonWithIds> &polygons, const TagMap &tags) = 0;
	virtual void OutLine(StyleDef &styleDef, const ContoursWithIds &lineAsPolygons, const TagMap &tags) = 0;
	virtual void OutPoi(StyleDef &styleDef, int64_t nid, double px, double py, const TagMap &tags) = 0;
};

// ***********************************************

///FeatureConverter takes FeatureStore data, finds an appropriate Style to the data, converts coordinates to screen space and produces a stream of objects that are returned to a visitor object derived from IFeatureConverterResult.
class FeatureConverter
{
public:
	class IDrawLib *output;
	double extentx1, extenty1, extentx2, extenty2;
	double width, height;
	std::vector<class IFeatureConverterResult *> shapesOutput;
	
	FeatureConverter(class IDrawLib *output);
	virtual ~FeatureConverter() {};

	void Convert(int zoom, class FeatureStore &featureStore, class ITransform &transform, class Style &style);
	void IdLatLonListsToContour(IdLatLonList &shape, class ITransform &transform, ContourWithIds &line1);
	void ToDrawSpace(double nx, double ny, double &px, double &py);
};

FeatureConverter::FeatureConverter(class IDrawLib *output) : output(output)
{
	extentx1 = 0.0;
	extenty1 = 0.0;
	extentx2 = 0.0;
	extenty2 = 0.0;
	int ret = output->GetDrawableExtents(extentx1,
		extenty1,
		extentx2,
		extenty2);
	if(ret != 0){
		extentx1 = 0.0, extenty1 = 1.0, extentx2 = 0.0, extenty2 = 1.0;
	}
	width = extentx2 - extentx1;
	height = extenty2 - extenty1;
}

void FeatureConverter::Convert(int zoom, class FeatureStore &featureStore, class ITransform &transform, class Style &style)
{
	//Find polygons, get associated style, send to output renderers
	for(size_t i=0;i<featureStore.areas.size();i++)
	{
		ContoursWithIds outers;
		ContoursWithIds inners;

		class FeatureArea &area = featureStore.areas[i];

		StyleDef styleDef;
		int recognisedStyle = style.GetStyle(zoom, area.tags, Style::Area, styleDef);
		if(!recognisedStyle) continue;

		std::vector<IdLatLonList> &outerShapes = area.outerShapes;
		for(size_t j=0;j<outerShapes.size();j++)
		{
			ContourWithIds outer;
			IdLatLonList &outerShape = outerShapes[j];
			this->IdLatLonListsToContour(outerShape, transform, outer);
			outers.push_back(outer);
		}

		std::vector<IdLatLonList> &innerShapes = area.innerShapes;
		for(size_t j=0;j<innerShapes.size();j++)
		{
			ContourWithIds inner;
			IdLatLonList &innerShape = innerShapes[j];
			this->IdLatLonListsToContour(innerShape, transform, inner);
			inners.push_back(inner);
		}
		
		//This includes inner shapes in each outer way, which is probably ok for drawing
		//purposes. Some inner ways will not even be within the other shape.

		std::vector<PolygonWithIds> polygons;
		for(size_t j=0; j<outers.size(); j++)
		{
			PolygonWithIds polygon(outers[j], inners);
			polygons.push_back(polygon);
		}

		for(size_t j=0; j < shapesOutput.size(); j++)
			shapesOutput[j]->OutArea(styleDef, polygons, area.tags);
	}

	//Find lines, get associated style, send to output renderers
	for(size_t i=0;i<featureStore.lines.size();i++)
	{
		ContourWithIds line1;
		class FeatureLine &line = featureStore.lines[i];

		StyleDef styleDef;
		int recognisedStyle = style.GetStyle(zoom, line.tags, Style::Line, styleDef);
		if(!recognisedStyle) continue;

		IdLatLonList &shape = line.shape;
		this->IdLatLonListsToContour(shape, transform, line1);

		ContoursWithIds tmp;
		tmp.push_back(line1);
		for(size_t j=0; j < shapesOutput.size(); j++)
			shapesOutput[j]->OutLine(styleDef, tmp, line.tags);

	}

	//Find points, get associated style, send to output renderers
	for(size_t i=0;i<featureStore.pois.size();i++)
	{
		class FeaturePoi &poi = featureStore.pois[i];

		StyleDef styleDef;
		int recognisedStyle = style.GetStyle(zoom, poi.tags, Style::Poi, styleDef);
		if(!recognisedStyle) continue;

		double sx = 0.0, sy = 0.0;
		transform.LatLong2Screen(poi.lat, poi.lon, sx, sy);
		double px = 0.0, py = 0.0;
		this->ToDrawSpace(sx, sy, px, py);

		for(size_t j=0; j < shapesOutput.size(); j++)
			shapesOutput[j]->OutPoi(styleDef, poi.nid, px, py, poi.tags);

	}

}

void FeatureConverter::IdLatLonListsToContour(IdLatLonList &shape, class ITransform &transform, ContourWithIds &line1)
{
	line1.clear();
	for(size_t j=0;j<shape.size();j++)
	{
		IdLatLon &pt = shape[j];
		double sx = 0.0, sy = 0.0;
		transform.LatLong2Screen(pt.lat, pt.lon, sx, sy);
		double px = 0.0, py = 0.0;
		this->ToDrawSpace(sx, sy, px, py);

		line1.push_back(PointWithId(pt.objId, Point(px, py)));
	}
}

void FeatureConverter::ToDrawSpace(double nx, double ny, double &px, double &py)
{
	px = nx * width + extentx1;
	py = ny * height + extenty1;
}

// **************************************************************

/**Takes a stream of objects with associated styles from FeatureConverter 
and converts them into draw commands that reside in a DrawTreeNode based drawTree.
*/
class FeaturesToDrawCmds : public IFeatureConverterResult
{
public:
	class DrawTreeNode *drawTree;

	FeaturesToDrawCmds(class DrawTreeNode *drawTree) : drawTree(drawTree) {};
	virtual ~FeaturesToDrawCmds() {};
	
	void OutArea(StyleDef &styleDef, const std::vector<PolygonWithIds> &polygons, const TagMap &tags);
	void OutLine(StyleDef &styleDef, const ContoursWithIds &lineAsPolygons, const TagMap &tags);
	void OutPoi(StyleDef &styleDef, int64_t nid, double px, double py, const TagMap &tags) {};
	void DrawToTree(StyleDef &styleDef, const std::vector<Polygon> &polygons);
};

void FeaturesToDrawCmds::OutArea(StyleDef &styleDef, const std::vector<PolygonWithIds> &polygons, const TagMap &tags)
{
	//Strip ID information
	std::vector<Polygon> polygonsNoId;
	StripIdsFromPolygons(polygons, polygonsNoId);

	//Integrate shape into draw tree
	this->DrawToTree(styleDef, polygonsNoId);
}

void FeaturesToDrawCmds::OutLine(StyleDef &styleDef, const ContoursWithIds &lines, const TagMap &tags)
{
	//Integrate shape into draw tree, pretend this are polygons
	std::vector<Polygon> lineAsPolygons;
	Contours emptyInnerContours;
	for(size_t i =0; i < lines.size(); i++)
	{
		const ContourWithIds &line = lines[i];

		//Strip ID information
		Contour lineNoIds;
		StripIdsFromContour(line, lineNoIds);
		
		lineAsPolygons.push_back(Polygon(lineNoIds, emptyInnerContours));
	}

	this->DrawToTree(styleDef, lineAsPolygons);
}

void FeaturesToDrawCmds::DrawToTree(StyleDef &styleDef, const std::vector<Polygon> &polygons)
{
	//Integrate area shape into draw tree
	for(size_t j=0; j< styleDef.size(); j++)
	{
		StyleAndLayerDef &styleAndLayerDef = styleDef[j];
		LayerDef &layerDef = styleAndLayerDef.first;
		StyleAttributes &styleAttributes = styleAndLayerDef.second;

		//Surpress coastline
		TagMap::const_iterator natural = styleAttributes.find("coastline");
		if(natural != styleAttributes.end() && natural->second == "yes")
			continue;

		class ShapeProperties prop(double(rand()%100) / 100.0, double(rand()%100) / 100.0, double(rand()%100) / 100.0);
		TagMap::const_iterator colIt = styleAttributes.find("polygon-fill");
		if(colIt != styleAttributes.end()) {
			int colOk = ColourStringToRgba(colIt->second.c_str(), prop.r, prop.g, prop.b, prop.a);
			if(!colOk) continue;
		}
		else
			continue; //Don't draw if colour not specified

		class DrawTreeNode *node = drawTree->GetLayer(layerDef);
		StyledPolygons::iterator sp = node->styledPolygons.find(prop);
		if(sp == node->styledPolygons.end())
			node->styledPolygons[prop] = polygons;
		else
			sp->second.insert(sp->second.end(), polygons.begin(), polygons.end());
	}

	//Integrate line shape into draw tree
	for(size_t j=0; j< styleDef.size(); j++)
	{
		StyleAndLayerDef &styleAndLayerDef = styleDef[j];
		LayerDef &layerDef = styleAndLayerDef.first;
		StyleAttributes &styleAttributes = styleAndLayerDef.second;

		//Surpress coastline
		TagMap::const_iterator natural = styleAttributes.find("coastline");
		if(natural != styleAttributes.end() && natural->second == "yes")
			continue;

		class LineProperties lineProp1(1.0, 1.0, 1.0);

		TagMap::const_iterator attrIt = styleAttributes.find("line-color");
		if(attrIt != styleAttributes.end()) {
			int colOk = ColourStringToRgba(attrIt->second.c_str(), lineProp1.r, lineProp1.g, lineProp1.b, lineProp1.a);
			if(!colOk) continue;
		}
		else
			continue; //Don't draw if colour not specified

		int lineWidth = 1.0;
		attrIt = styleAttributes.find("line-width");
		if(attrIt != styleAttributes.end())
			lineWidth = atof(attrIt->second.c_str());
		lineProp1.lineWidth = lineWidth;
		
		std::string lineCap = "butt";
		attrIt = styleAttributes.find("line-cap");
		if(attrIt != styleAttributes.end())
			lineCap = attrIt->second;
		lineProp1.lineCap = lineCap;

		std::string lineJoin = "miter";
		attrIt = styleAttributes.find("line-join");
		if(attrIt != styleAttributes.end())
			lineJoin = attrIt->second;
		lineProp1.lineJoin = lineJoin;

		Contours lines1;
		for(size_t i=0;i<polygons.size();i++)
			lines1.push_back(polygons[i].first);

		class DrawTreeNode *node = this->drawTree->GetLayer(layerDef);
		StyledLines::iterator sl = node->styledLines.find(lineProp1);
		if(sl == node->styledLines.end())
			node->styledLines[lineProp1] = lines1;
		else
			sl->second.insert(sl->second.end(), lines1.begin(), lines1.end());
	}
}

// **********************************************

/*\brief Takes a stream of objects with associated styles from FeatureConverter 
and stores where labels should be drawn (output to member poiLabels).
It also filters the objects so that only named objects are passed on.
The result is passed to LabelEngine.
*/
class FeaturesToLabelEngine : public IFeatureConverterResult
{
public:
	class LabelEngine *labelEngine;
	std::vector<class PoiLabel> poiLabels;

	FeaturesToLabelEngine(class LabelEngine *labelEngine) : labelEngine(labelEngine) {};
	virtual ~FeaturesToLabelEngine() {};
	
	void OutArea(StyleDef &styleDef, const std::vector<PolygonWithIds> &polygons, const TagMap &tags);
	void OutLine(StyleDef &styleDef, const ContoursWithIds &lineAsPolygons, const TagMap &tags);
	void OutPoi(StyleDef &styleDef, int64_t nid, double px, double py, const TagMap &tags);
};

void FeaturesToLabelEngine::OutArea(StyleDef &styleDef, const std::vector<PolygonWithIds> &polygons, const TagMap &tags)
{
	//Transfer area markers and labels to label engine	
	for(size_t j=0; j< styleDef.size(); j++)
	{
		StyleAndLayerDef &styleAndLayerDef = styleDef[j];
		LayerDef &layerDef = styleAndLayerDef.first;
		StyleAttributes &styleAttributes = styleAndLayerDef.second;

		string textName = "";
		TagMap::const_iterator attrIt = styleAttributes.find("text-name");
		if(attrIt != styleAttributes.end()) {
			textName = attrIt->second;
		}
		else
			continue; //Don't draw if name not specified

		//Get average position of outer ways
		for(size_t i=0; i< polygons.size(); i++)
		{
			double px = 0.0, py = 0.0;
			const ContourWithIds &outerWithIds = polygons[i].first;
			for(size_t k=0;k<outerWithIds.size();k++)
			{
				px += outerWithIds[k].second.first;
				py += outerWithIds[k].second.second;
			}
			px /= outerWithIds.size();
			py /= outerWithIds.size();
			Contour shape;
			shape.push_back(Point(px, py));
			poiLabels.push_back(PoiLabel(shape, textName, tags, styleAttributes));
		}
	}
}

void FeaturesToLabelEngine::OutLine(StyleDef &styleDef, const ContoursWithIds &lines, const TagMap &tags)
{
	//Transfer line labels to label engine	
	for(size_t j=0; j< styleDef.size(); j++)
	{
		StyleAndLayerDef &styleAndLayerDef = styleDef[j];
		LayerDef &layerDef = styleAndLayerDef.first;
		StyleAttributes &styleAttributes = styleAndLayerDef.second;

		string textName = "";
		TagMap::const_iterator attrIt = styleAttributes.find("text-name");
		if(attrIt != styleAttributes.end()) {
			textName = attrIt->second;
		}
		else
			continue; //Don't draw if name not specified
				
		for(size_t i=0; i< lines.size(); i++)
		{
			const ContourWithIds &line = lines[i];

			//Check label paths are the correct way up (left to right generally)
			Contour shape;
			StripIdsFromContour(line, shape);
			if(line.size() >= 2)
			{								
				const Point &start = shape[0];
				const Point &end = shape[shape.size()-1];
				if(start.first > end.first) //First value is x coordinate
				{
					//This needs to be reversed
					std::reverse(shape.begin(), shape.end());
				}
			}

			poiLabels.push_back(PoiLabel(shape, textName, tags, styleAttributes));
		}
	}
}

void FeaturesToLabelEngine::OutPoi(StyleDef &styleDef, int64_t nid, double px, double py, const TagMap &tags)
{
	//Transfer POI markers and labels to label engine	
	for(size_t j=0; j< styleDef.size(); j++)
	{
		StyleAndLayerDef &styleAndLayerDef = styleDef[j];
		LayerDef &layerDef = styleAndLayerDef.first;
		StyleAttributes &styleAttributes = styleAndLayerDef.second;

		string textName = "";
		TagMap::const_iterator attrIt = styleAttributes.find("text-name");
		if(attrIt != styleAttributes.end()) {
			textName = attrIt->second;
		}
		else
			continue; //Don't draw if name not specified
		
		Contour shape;
		shape.push_back(Point(px, py));
		poiLabels.push_back(PoiLabel(shape, textName, tags, styleAttributes));
	}
}

// **************************************************************

///Takes a stream of objects with associated styles from FeatureConverter, filters by coastlines and stores them internally for
///analysis.
class FeaturesToLandPolys : public IFeatureConverterResult
{
protected:
	std::vector<ContourWithIds> coastlines;

public:
	FeaturesToLandPolys() {};
	virtual ~FeaturesToLandPolys() {};
	
	void OutArea(StyleDef &styleDef, const std::vector<PolygonWithIds> &polygons, const TagMap &tags);
	void OutLine(StyleDef &styleDef, const ContoursWithIds &lineAsPolygons, const TagMap &tags);
	void OutPoi(StyleDef &styleDef, int64_t nid, double px, double py, const TagMap &tags) {};

	void Draw(class IDrawLib *output);
};

void FeaturesToLandPolys::OutArea(StyleDef &styleDef, const std::vector<PolygonWithIds> &polygons, const TagMap &tags)
{
	//Filter to find coastlines
	for(size_t i=0; i<styleDef.size();i++)
	{
		const StyleAndLayerDef &styleAndLayerDef = styleDef[i];
		const StyleAttributes &styleAttributes = styleAndLayerDef.second;

		TagMap::const_iterator natural = styleAttributes.find("coastline");
		if(natural == styleAttributes.end())
			continue;
		for(size_t i=0; i<polygons.size(); i++)
		{
			const PolygonWithIds &poly = polygons[i];
			ContourWithIds outerContour = poly.first;

			//Add the initial node to the last to indicate it is a closed path (supposedly)
			outerContour.push_back(outerContour[0]);

			this->coastlines.push_back(outerContour);
		}
	}
}

void FeaturesToLandPolys::OutLine(StyleDef &styleDef, const ContoursWithIds &lines, const TagMap &tags)
{
	//Filter to find coastlines
	for(size_t i=0; i<styleDef.size();i++)
	{
		const StyleAndLayerDef &styleAndLayerDef = styleDef[i];
		const StyleAttributes &styleAttributes = styleAndLayerDef.second;

		TagMap::const_iterator natural = styleAttributes.find("coastline");
		if(natural == styleAttributes.end())
			continue;

		for(size_t i=0; i<lines.size(); i++)
		{
			const ContourWithIds &line = lines[i];
			this->coastlines.push_back(line);
		}
	}

}

void FeaturesToLandPolys::Draw(class IDrawLib *output)
{
	//Merge ways into continues paths, where possible
	std::vector<ContourWithIds> merged;
	MergeContours(this->coastlines, false, merged);

	std::vector<std::vector<class PointInfo> > collectedLoops;
	std::vector<std::vector<class PointInfo> > internalLoops;
	std::vector<std::vector<class PointInfo> > reverseInternalLoops;

	//left,bottom,right,top
	double x1, x2, y1, y2;
	output->GetDrawableExtents(x1,
		y1,
		x2,
		y2);
	std::vector<double> bbox;
	bbox.push_back(x1);
	bbox.push_back(y2);
	bbox.push_back(x2);
	bbox.push_back(y1);

	CompletePolygonsInBbox(merged, 
		bbox, 
		1,
		1e-6,
		collectedLoops,
		internalLoops,
		reverseInternalLoops);

	ShapeProperties properties(1.0, 1.0, 1.0);
	std::vector<Polygon> landPolys;
	PointInfoVecToPolygons(collectedLoops, landPolys);
	output->AddDrawPolygonsCmd(landPolys, properties);

	std::vector<Polygon> islands;
	PointInfoVecToPolygons(internalLoops, islands);
	output->AddDrawPolygonsCmd(islands, properties);

	ShapeProperties seaPolyPoperties(0.0, 0.0, 0.3);
	std::vector<Polygon> inlandSea;
	PointInfoVecToPolygons(reverseInternalLoops, inlandSea);
	output->AddDrawPolygonsCmd(inlandSea, seaPolyPoperties);
	
}

// **********************************************

MapRender::MapRender(class IDrawLib *output) : output(output)
{

}

MapRender::~MapRender()
{

}

void MapRender::Render(int zoom, class FeatureStore &featureStore, 
	bool renderObjects, bool outputLabels,
	class ITransform &transform, LabelsByImportance &organisedLabelsOut)
{
	class DrawTreeNode drawTree;
	class LabelEngine labelEngine(this->output);
	
	//Converts objects to draw tree and label info based on style definition
	class FeatureConverter featureConverter(this->output);

	class FeaturesToDrawCmds featuresToDrawCmds(&drawTree);
	class FeaturesToLandPolys featuresToLandPolys;
	if(renderObjects)
	{
		featureConverter.shapesOutput.push_back(&featuresToDrawCmds);
		featureConverter.shapesOutput.push_back(&featuresToLandPolys);
	}

	class FeaturesToLabelEngine featuresToLabelEngine(&labelEngine);
	if(outputLabels)
		featureConverter.shapesOutput.push_back(&featuresToLabelEngine);

	featureConverter.Convert(zoom, featureStore, transform, this->style);

	if(renderObjects)
	{
		featuresToLandPolys.Draw(this->output);

		//Interate through draw tree to produce ordered draw commands
		drawTree.WriteDrawCommands(this->output);
		this->output->Draw();
	}

	if(outputLabels)
	{
		organisedLabelsOut.clear();
		LabelsByImportance organisedLabelsTmp;

		//Merge equivalent labels?
		//Only label paths beginning on this tile are considered for merging to prevent differences in merging between tiles
		//TODO

		//Smooth label paths
		SmoothLabelPaths(featuresToLabelEngine.poiLabels, 5.0);

		//Convert label defs to draw commands
		labelEngine.LabelPoisToStyledLabel(featuresToLabelEngine.poiLabels, organisedLabelsTmp);

		//Remove overlapping labels
		labelEngine.RemoveOverlapping(organisedLabelsTmp, organisedLabelsOut);
	}
}

void MapRender::RenderLabels(const RenderLabelList &labelList,
	const RenderLabelListOffsets &labelOffsets)
{
	if(labelList.size() != labelOffsets.size())
		throw std::runtime_error("List lengths should match");
	class LabelEngine labelEngine(this->output);

	//Combine labels into a unified list
	LabelsByImportance combinedLabels;
	for(size_t i = 0; i< labelList.size(); i++)
	{
		const std::pair<double, double> &offset = labelOffsets[i];
		LabelsByImportance translatedLabels;
		TranslateLabelsByImportance(labelList[i], offset.first, offset.second, translatedLabels);
		MergeLabelsByImportance(combinedLabels, translatedLabels);
	}

	///Remove overlapping labels
	LabelsByImportance deoverlappedLabels;
	labelEngine.RemoveOverlapping(combinedLabels, deoverlappedLabels);

	///Do label drawing
	labelEngine.WriteDrawCommands(deoverlappedLabels);
	this->output->Draw();
}


