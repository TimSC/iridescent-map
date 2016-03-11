#include "LabelEngine.h"
#include <iostream>
using namespace std;

LabelDef::LabelDef()
{

}

LabelDef::LabelDef(const class LabelRect &labelRect,
		const class TextProperties &foregroundProp,
		const class TextProperties &backgroundProp,
		const std::vector<class TextLabel> &labels) : labelRect(labelRect),
		foregroundProp(foregroundProp),
		backgroundProp(backgroundProp),
		labels(labels)
{

}

LabelDef::LabelDef(const class LabelDef &a)
{
	*this = a;
}

LabelDef::~LabelDef()
{

}

LabelDef& LabelDef::operator=(const LabelDef &arg)
{
	labelRect = arg.labelRect;
	foregroundProp = arg.foregroundProp;
	backgroundProp = arg.backgroundProp;
	labels = arg.labels;
}

// **********************************************

LabelRect::LabelRect() : x(0.0), y(0.0), w(1.0), h(1.0)
{
	
}

LabelRect::LabelRect(double x, double y, double w, double h) : x(x), y(y), w(w), h(h)
{

}

LabelRect::LabelRect(const class LabelRect &a)
{
	*this = a;
}

LabelRect::~LabelRect()
{

}

LabelRect& LabelRect::operator=(const LabelRect &arg)
{
	x = arg.x;
	y = arg.y;
	w = arg.w;
	h = arg.h;
	return *this;
}

bool LabelRect::Overlaps(const LabelRect &arg)
{
	if (x + w < arg.x) return false;
	if (x >= arg.x + arg.w) return false;
	if (y + h < arg.y) return false;
	if (y >= arg.y + arg.w) return false;
	return true;
}

void LabelRect::Print()
{
	cout << x << "," << y << "," << w << "," << h << endl;
}


// ***************************************************

PoiLabel::PoiLabel()
{
	sx = 0.0;
	sy = 0.0;
}

PoiLabel::PoiLabel(double sx, double sy, const std::string &textName, const TagMap &tags):
	sx(sx), sy(sy), textName(textName), tags(tags)
{
	
}

PoiLabel::PoiLabel(const class PoiLabel &a)
{
	*this = a;
}

PoiLabel::~PoiLabel()
{

}

PoiLabel& PoiLabel::operator=(const PoiLabel &arg)
{
	sx = arg.sx;
	sy = arg.sy;
	textName = arg.textName;
	tags = arg.tags;
}

// *******************************************************

LabelEngine::LabelEngine(class IDrawLib *output):
	output(output)
{

}

LabelEngine::~LabelEngine()
{

}

void LabelEngine::OrganiseLabels(OrganisedLabels &organisedLabelsOut)
{
	organisedLabelsOut.clear();

	for(size_t i=0;i < this->poiLabels.size(); i++)
	{
		class PoiLabel &label = this->poiLabels[i];

		std::string &textName = label.textName;
		std::string outString;
		if(textName[0] == '[' && textName[textName.size()-1] == ']') {
			std::string keyName(&textName[1], textName.size()-2);
			TagMap::const_iterator it = label.tags.find(keyName);
			if(it == label.tags.end()) continue;
			outString = it->second;
		}
		else
			outString = textName;

		//Get bounds
		double width=0.0, height=0.0;
		class TextProperties foregroundProp(1.0,1.0,1.0);
		if(this->output != NULL)
		{			
			int ret = this->output->GetTextExtents(outString.c_str(), foregroundProp, 
				width, height);
			if(ret != 0)
			{
				width=0.0, height=0.0;
			}
		}
		
		std::vector<class TextLabel> textStrs;
		double lx = label.sx - width / 2.0;
		double ly = label.sy;
		textStrs.push_back(TextLabel(outString, lx, ly));

		bool foundOverlap = false;
		class LabelRect labelRect(lx, ly, width, height);
		for(OrganisedLabels::iterator itr = organisedLabelsOut.begin(); itr != organisedLabelsOut.end(); itr++)
		{
			vector<class LabelDef> &lbs = itr->second;
			for(size_t j=0; j<lbs.size(); j++)
			{
				foundOverlap = labelRect.Overlaps(lbs[j].labelRect);
				if(foundOverlap) break;
			}
		}
		if(foundOverlap) continue;

		class TextProperties backgroundProp(0.0,0.0,0.0);
		backgroundProp.a = 0.5;
		backgroundProp.outline = true;
		backgroundProp.lineWidth=2.0;

		//Add label definition to list
		OrganisedLabels::iterator it = organisedLabelsOut.find(0);
		if(it == organisedLabelsOut.end())
			organisedLabelsOut[0] = vector<class LabelDef>();
		organisedLabelsOut[0].push_back(LabelDef(labelRect, foregroundProp, backgroundProp, textStrs));
	}
}

void LabelEngine::WriteDrawCommands(const OrganisedLabels &organisedLabels)
{

	for(OrganisedLabels::const_iterator itr = organisedLabels.begin(); itr != organisedLabels.end(); itr++)
	{
		const vector<class LabelDef> &lbs = itr->second;
		for(size_t j=0; j<lbs.size(); j++)
		{
			const class LabelDef &labelDef = lbs[j];

			//Ghost background
			if(this->output != NULL)
				this->output->AddDrawTextCmd(labelDef.labels, labelDef.backgroundProp);

			//Foreground text
			if(this->output != NULL)
				this->output->AddDrawTextCmd(labelDef.labels, labelDef.foregroundProp);
		}
	}
}


void LabelEngine::AddPolygonLabel(const std::vector<Polygon> &polygons, std::string &textName, const TagMap &tags)
{

}

void LabelEngine::AddLineLabel(const Contour &line, std::string &textName, const TagMap &tags)
{

}

void LabelEngine::AddPoiLabel(double sx, double sy, std::string &textName, const TagMap &tags)
{
	this->poiLabels.push_back(PoiLabel(sx, sy, textName, tags));
}

