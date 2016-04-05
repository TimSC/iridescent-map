#ifndef _LABEL_ENGINE_H
#define _LABEL_ENGINE_H

#include "drawlib/drawlib.h"
#include "cppo5m/o5m.h"
#include "Style.h"

class LabelBounds
{
protected:
	TwistedTriangles bounds;
	double minX, minY, maxX, maxY;
	void UpdateRectBbox();

public:
	LabelBounds();
	LabelBounds(const TwistedTriangles &bounds);
	LabelBounds(const class LabelBounds &a);
	virtual ~LabelBounds();
	LabelBounds& operator=(const LabelBounds &arg);

	bool Overlaps(const LabelBounds &arg) const;
	void Translate(double tx, double ty);
};

///A label/icon group with low level drawing API information, with bounding info.
class LabelDef
{
public:
	class LabelBounds labelBounds;
	class TextProperties foregroundProp;
	class TextProperties backgroundProp;
	std::vector<class TextLabel> labels;
	std::vector<class TwistedTextLabel> twistedLabels;

	LabelDef(const class LabelBounds &labelBounds,
		const class TextProperties &foregroundProp,
		const class TextProperties &backgroundProp,
		const std::vector<class TextLabel> &labels);
	LabelDef(const class LabelBounds &labelBounds,
		const class TextProperties &foregroundProp,
		const class TextProperties &backgroundProp,
		const std::vector<class TwistedTextLabel> &twistedLabels);
	LabelDef(const class LabelDef &a);
	virtual ~LabelDef();
	LabelDef& operator=(const LabelDef &arg);

	void Translate(double tx, double ty);
};

typedef std::map<int, std::vector<class LabelDef> > LabelsByImportance;
void MergeLabelsByImportance(LabelsByImportance &mergeIntoThis, const LabelsByImportance &labelsToMerge);
void TranslateLabelsByImportance(const LabelsByImportance &labelsIn, double tx, double ty, LabelsByImportance &labelsOut);
void SmoothLabelPaths(std::vector<class PoiLabel> &poiLabels, double tolerance);

///An object to label in draw space, as well as tags and map style definition.
class PoiLabel
{
public:
	Contour shape;
	std::string textName;
	TagMap tags;
	StyleAttributes styleAttributes;

	PoiLabel();
	PoiLabel(const Contour &shape, const std::string &textName, const TagMap &tags, const StyleAttributes &styleAttributes);
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
