#ifndef _LABEL_ENGINE_H
#define _LABEL_ENGINE_H

#include "drawlib/drawlib.h"
#include "cppo5m/o5m.h"

class LabelRect
{
public:
	double x, y, w, h;

	LabelRect();
	LabelRect(double x, double y, double w, double h);
	LabelRect(const class LabelRect &a);
	virtual ~LabelRect();
	LabelRect& operator=(const LabelRect &arg);

	bool Overlaps(const LabelRect &arg);
	void Print();
};

class PoiLabel
{
public:
	double sx;
	double sy;
	std::string textName;
	TagMap tags;

	PoiLabel();
	PoiLabel(double sx, double sy, const std::string &textName, const TagMap &tags);
	PoiLabel(const class PoiLabel &a);
	virtual ~PoiLabel();
	PoiLabel& operator=(const PoiLabel &arg);
};

class LabelEngine
{
protected:
	class IDrawLib *output; //Borrowed reference
	std::vector<class PoiLabel> poiLabels;

public:
	LabelEngine(class IDrawLib *output);
	virtual ~LabelEngine();

	void WriteDrawCommands();

	void AddPolygonLabel(const std::vector<Polygon> &polygons, std::string &textName, TagMap &tags);
	void AddLineLabel(const Contour &line, std::string &textName, TagMap &tags);
	void AddPoiLabel(double sx, double sy, std::string &textName, TagMap &tags);
};

#endif //_LABEL_ENGINE_H
