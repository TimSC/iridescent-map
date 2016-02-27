#include "MapRender.h"
#include <iostream>
using namespace std;

MapRender::MapRender(class IDrawLib *output) : output(output)
{

}

MapRender::~MapRender()
{

}

void MapRender::Render(int layerNum, class FeatureStore &featureStore, class ITransform &transform)
{

	std::vector<Polygon> polygons;

	for(size_t i=0;i<featureStore.areas.size();i++)
	{
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
				outer.push_back(Point(sx*640.0, sy*640.0));
			}
		}
		Polygon polygon(outer, inners);
		polygons.push_back(polygon);
	}	

	class ShapeProperties prop(1.0, 0.0, 0.0);
	this->output->AddDrawPolygonsCmd(polygons, prop);

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


