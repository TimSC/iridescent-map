#include "MapRender.h"
#include <iostream>
#include <stdlib.h>
using namespace std;

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

void MapRender::Render(int layerNum, class FeatureStore &featureStore, class ITransform &transform)
{
	for(size_t i=0;i<featureStore.areas.size();i++)
	{
		std::vector<Polygon> polygons;
		Contour outer;
		Contours inners;

		class FeatureArea &area = featureStore.areas[i];
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
		Polygon polygon(outer, inners);
		polygons.push_back(polygon);

		class ShapeProperties prop(double(rand()%100) / 100.0, double(rand()%100) / 100.0, double(rand()%100) / 100.0);
		this->output->AddDrawPolygonsCmd(polygons, prop);

	}	

	for(size_t i=0;i<featureStore.pois.size();i++)
	{
		class FeaturePoi &poi = featureStore.pois[i];
		double sx = 0.0, sy = 0.0;
		//transform.LatLong2Screen(poi.lat, poi.lon, sx, sy);
	}

	for(size_t i=0;i<featureStore.lines.size();i++)
	{
		class FeatureLine &line = featureStore.lines[i];
		IdLatLonList &shape = line.shape;
		for(size_t j=0;j<shape.size();j++)
		{
			
		}
	}	

	output->Draw();
}


