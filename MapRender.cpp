#include "MapRender.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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
	for(size_t i=0;i < this->styledPolygons.size();i++)
	{
		StyledPolygons &sp = styledPolygons[i];
		std::vector<Polygon> &polys = sp.first;
		class ShapeProperties &prop = sp.second;

		output->AddDrawPolygonsCmd(polys, prop);
	}

	for(size_t i=0;i < this->styledLines.size();i++)
	{
		StyledLines &sl = styledLines[i];
		Contours &lines = sl.first;
		class LineProperties &prop = sl.second;

		output->AddDrawLinesCmd(lines, prop);
	}
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
			for(size_t k=0;k < outerShape.size(); k++)
			{
				IdLatLon &pt = outerShape[k];
				double sx = 0.0, sy = 0.0;
				transform.LatLong2Screen(pt.lat, pt.lon, sx, sy);
				//cout << sx << ","<< sy << endl;
				double px = 0.0, py = 0.0;
				this->ToDrawSpace(sx, sy, px, py);
				outer.push_back(Point(px, py));
			}
		}

		//Integrate shape into draw tree
		for(size_t j=0; j< styleDef.size(); j++)
		{
			StyleAndLayerDef &styleAndLayerDef = styleDef[j];
			LayerDef &layerDef = styleAndLayerDef.first;
			StyleAttributes &styleAttributes = styleAndLayerDef.second;

			Polygon polygon(outer, inners);
			polygons.push_back(polygon);

			class ShapeProperties prop(double(rand()%100) / 100.0, double(rand()%100) / 100.0, double(rand()%100) / 100.0);
			TagMap::const_iterator colIt = styleAttributes.find("fill-color");
			if(colIt != styleAttributes.end()) {
				int colOk = this->ColourStringToRgb(colIt->second.c_str(), prop.r, prop.g, prop.b);
				if(!colOk) continue;
			}

			drawTree.styledPolygons.push_back(StyledPolygons(polygons, prop));	
		}
	}	

	for(size_t i=0;i<featureStore.lines.size();i++)
	{
		Contour line1;

		class FeatureLine &line = featureStore.lines[i];

		StyleDef styleDef;
		int recognisedStyle = style.GetStyle(zoom, line.tags, Style::Line, styleDef);
		if(!recognisedStyle) continue;

		IdLatLonList &shape = line.shape;
		for(size_t j=0;j<shape.size();j++)
		{
			IdLatLon &pt = shape[j];
			double sx = 0.0, sy = 0.0;
			transform.LatLong2Screen(pt.lat, pt.lon, sx, sy);
			double px = 0.0, py = 0.0;
			this->ToDrawSpace(sx, sy, px, py);

			line1.push_back(Point(px, py));
		}

		//Integrate shape into draw tree
		for(size_t j=0; j< styleDef.size(); j++)
		{
			StyleAndLayerDef &styleAndLayerDef = styleDef[j];
			LayerDef &layerDef = styleAndLayerDef.first;
			StyleAttributes &styleAttributes = styleAndLayerDef.second;

			class LineProperties lineProp1(double(rand()%100) / 100.0, double(rand()%100) / 100.0, double(rand()%100) / 100.0, 3.0);
			TagMap::const_iterator colIt = styleAttributes.find("line-color");
			if(colIt != styleAttributes.end()) {
				int colOk = this->ColourStringToRgb(colIt->second.c_str(), lineProp1.r, lineProp1.g, lineProp1.b);
				if(!colOk) continue;
			}
		
			Contours lines1;
			lines1.push_back(line1);
			drawTree.styledLines.push_back(StyledLines(lines1, lineProp1));
		}
	}	

	for(size_t i=0;i<featureStore.pois.size();i++)
	{
		class FeaturePoi &poi = featureStore.pois[i];
		double sx = 0.0, sy = 0.0;


	}

	//Interate through draw tree to produce ordered draw commands
	drawTree.WriteDrawCommands(output);

	output->Draw();
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

