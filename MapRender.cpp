#include "MapRender.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "LabelEngine.h"
using namespace std;

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

// **********************************************

MapRender::MapRender(class IDrawLib *output) : output(output)
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

MapRender::~MapRender()
{

}

void MapRender::ToDrawSpace(double nx, double ny, double &px, double &py)
{
	px = nx * width + extentx1;
	py = ny * height + extenty1;
}

void MapRender::Render(int zoom, class FeatureStore &featureStore, class ITransform &transform)
{
	class DrawTreeNode drawTree;
	class LabelEngine labelEngine(output);
	
	//Render polygons to draw tree
	for(size_t i=0;i<featureStore.areas.size();i++)
	{
		std::vector<Polygon> polygons;
		Contour outer;
		Contours inners;

		class FeatureArea &area = featureStore.areas[i];

		StyleDef styleDef;
		int recognisedStyle = style.GetStyle(zoom, area.tags, Style::Area, styleDef);
		if(!recognisedStyle) continue;

		std::vector<IdLatLonList> &outerShapes = area.outerShapes;
		for(size_t j=0;j<outerShapes.size();j++)
		{
			IdLatLonList &outerShape = outerShapes[j];
			this->IdLatLonListsToContour(outerShape, transform, outer);
		}
		Polygon polygon(outer, inners);
		polygons.push_back(polygon);

		//Integrate shape into draw tree
		this->DrawToTree(styleDef, polygons, drawTree);
	}

	//Render lines to draw tree
	for(size_t i=0;i<featureStore.lines.size();i++)
	{
		//Pretend these are polygons
		std::vector<Polygon> lineAsPolygons;
		Contour line1;
		Contours innersEmpty;

		class FeatureLine &line = featureStore.lines[i];

		StyleDef styleDef;
		int recognisedStyle = style.GetStyle(zoom, line.tags, Style::Line, styleDef);
		if(!recognisedStyle) continue;

		IdLatLonList &shape = line.shape;
		this->IdLatLonListsToContour(shape, transform, line1);
		Polygon lineAsPolygon(line1, innersEmpty);
		lineAsPolygons.push_back(lineAsPolygon);

		this->DrawToTree(styleDef, lineAsPolygons, drawTree);
	}

	//Render points to draw tree
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

		this->DrawPoiToLabelEngine(styleDef, px, py, labelEngine, poi.tags);
	}

	//Interate through draw tree to produce ordered draw commands
	drawTree.WriteDrawCommands(output);

	labelEngine.WriteDrawCommands();

	output->Draw();
}

void MapRender::IdLatLonListsToContour(IdLatLonList &shape, class ITransform &transform, Contour &line1)
{
	line1.clear();
	for(size_t j=0;j<shape.size();j++)
	{
		IdLatLon &pt = shape[j];
		double sx = 0.0, sy = 0.0;
		transform.LatLong2Screen(pt.lat, pt.lon, sx, sy);
		double px = 0.0, py = 0.0;
		this->ToDrawSpace(sx, sy, px, py);

		line1.push_back(Point(px, py));
	}
}

void MapRender::DrawToTree(StyleDef &styleDef, const std::vector<Polygon> &polygons, class DrawTreeNode &drawTree)
{
	//Integrate area shape into draw tree
	for(size_t j=0; j< styleDef.size(); j++)
	{
		StyleAndLayerDef &styleAndLayerDef = styleDef[j];
		LayerDef &layerDef = styleAndLayerDef.first;
		StyleAttributes &styleAttributes = styleAndLayerDef.second;

		class ShapeProperties prop(double(rand()%100) / 100.0, double(rand()%100) / 100.0, double(rand()%100) / 100.0);
		TagMap::const_iterator colIt = styleAttributes.find("polygon-fill");
		if(colIt != styleAttributes.end()) {
			int colOk = this->ColourStringToRgb(colIt->second.c_str(), prop.r, prop.g, prop.b);
			if(!colOk) continue;
		}
		else
			continue; //Don't draw if colour not specified

		class DrawTreeNode *node = drawTree.GetLayer(layerDef);
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

		class LineProperties lineProp1(1.0, 1.0, 1.0);

		TagMap::const_iterator attrIt = styleAttributes.find("line-color");
		if(attrIt != styleAttributes.end()) {
			int colOk = this->ColourStringToRgb(attrIt->second.c_str(), lineProp1.r, lineProp1.g, lineProp1.b);
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

		class DrawTreeNode *node = drawTree.GetLayer(layerDef);
		StyledLines::iterator sl = node->styledLines.find(lineProp1);
		if(sl == node->styledLines.end())
			node->styledLines[lineProp1] = lines1;
		else
			sl->second.insert(sl->second.end(), lines1.begin(), lines1.end());
	}
}

void MapRender::DrawPolygonsToLabelEngine(StyleDef &styleDef, const std::vector<Polygon> &polygons, 
	class LabelEngine &labelEngine, TagMap &tags)
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
			continue; //Don't draw if colour not specified

		labelEngine.AddPolygonLabel(polygons, textName, tags);
	}
}

void MapRender::DrawLineToLabelEngine(StyleDef &styleDef, const Contour &line, 
	class LabelEngine &labelEngine, TagMap &tags)
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
			continue; //Don't draw if colour not specified

		labelEngine.AddLineLabel(line, textName, tags);
	}
}

void MapRender::DrawPoiToLabelEngine(StyleDef &styleDef, double sx, double sy, 
	class LabelEngine &labelEngine, TagMap &tags)
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
			continue; //Don't draw if colour not specified

		labelEngine.AddPoiLabel(sx, sy, textName, tags);
	}
}

int MapRender::ColourStringToRgb(const char *colStr, double &r, double &g, double &b)
{
	if(colStr[0] == '\0')
		return 0;

	if(colStr[0] == '#')
	{
		if(strlen(colStr) == 7)
		{
			string sr(&colStr[1], 2);
			string sg(&colStr[3], 2);
			string sb(&colStr[5], 2);
			unsigned int tmp;
			sscanf(sr.c_str(), "%x", &tmp);
			r = tmp / 255.0;
			sscanf(sg.c_str(), "%x", &tmp);
			g = tmp / 255.0;
			sscanf(sb.c_str(), "%x", &tmp);
			b = tmp / 255.0;
			return 1;
		}

	}
	return 0;
}

