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

	bool Overlaps(const LabelRect &arg) const;
	void Print() const;
};

class LabelDef
{
public:
	class LabelRect labelRect;
	class TextProperties foregroundProp;
	class TextProperties backgroundProp;
	std::vector<class TextLabel> labels;

	LabelDef();
	LabelDef(const class LabelRect &labelRect,
		const class TextProperties &foregroundProp,
		const class TextProperties &backgroundProp,
		const std::vector<class TextLabel> &labels);
	LabelDef(const class LabelDef &a);
	virtual ~LabelDef();
	LabelDef& operator=(const LabelDef &arg);
};

typedef std::map<int, std::vector<class LabelDef> > LabelsByImportance;
void MergeLabelsByImportance(LabelsByImportance &mergeIntoThis, const LabelsByImportance &labelsToMerge);

class PoiLabel
{
public:
	double sx;
	double sy;
	std::string textName;
	TagMap tags;
	int importance;

	PoiLabel();
	PoiLabel(double sx, double sy, const std::string &textName, const TagMap &tags, int importance);
	PoiLabel(const class PoiLabel &a);
	virtual ~PoiLabel();
	PoiLabel& operator=(const PoiLabel &arg);
};

class LabelEngine
{
protected:
	class IDrawLib *output; //Borrowed reference

public:
	LabelEngine(class IDrawLib *output);
	virtual ~LabelEngine();

	void LabelPoisToStyledLabel(std::vector<class PoiLabel> &poiLabels, LabelsByImportance &organisedLabelsOut);
	void WriteDrawCommands(const LabelsByImportance &organisedLabels);
	void RemoveOverlapping(const LabelsByImportance &organisedLabelsTmp, LabelsByImportance &organisedLabelsOut);
};

#endif //_LABEL_ENGINE_H
