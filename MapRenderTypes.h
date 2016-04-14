#ifndef _MAP_RENDER_TYPES_H
#define _MAP_RENDER_TYPES_H

#include "drawlib/drawlib.h"
#include "Regrouper.h"
#include "LabelEngine.h"

typedef std::map<class ShapeProperties, std::vector<Polygon> > StyledPolygons;
typedef std::map<class LineProperties, Contours> StyledLines;
typedef std::map<class TextProperties, std::vector<class TextLabel> > StyledText;
typedef std::vector<LabelsByImportance> RenderLabelList;
typedef std::vector<std::pair<double, double> > RenderLabelListOffsets;
typedef std::pair<int64_t, Point> PointWithId;
typedef std::vector<PointWithId> ContourWithIds;
typedef std::vector<ContourWithIds> ContoursWithIds;
typedef std::pair<ContourWithIds, ContoursWithIds> PolygonWithIds;

#endif //_MAP_RENDER_TYPES_H

