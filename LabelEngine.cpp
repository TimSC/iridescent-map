#include "LabelEngine.h"
#include <iostream>
using namespace std;

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

void LabelEngine::WriteDrawCommands()
{
	vector<class LabelRect> rects;

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
		for(size_t j=0; j<rects.size(); j++)
		{
			foundOverlap = labelRect.Overlaps(rects[j]);
			if(foundOverlap) break;
		}
		if(foundOverlap) continue;

		//Ghost background
		class TextProperties backgroundProp(0.0,0.0,0.0);
		backgroundProp.a = 0.5;
		backgroundProp.outline = true;
		backgroundProp.lineWidth=2.0;
		if(this->output != NULL)
			this->output->AddDrawTextCmd(textStrs, backgroundProp);

		//Foreground text
		if(this->output != NULL)
			this->output->AddDrawTextCmd(textStrs, foregroundProp);

		//Add rect to list
		rects.push_back(labelRect);
	}
}

void LabelEngine::AddPolygonLabel(const std::vector<Polygon> &polygons, std::string &textName, TagMap &tags)
{

}

void LabelEngine::AddLineLabel(const Contour &line, std::string &textName, TagMap &tags)
{

}

void LabelEngine::AddPoiLabel(double sx, double sy, std::string &textName, TagMap &tags)
{
	this->poiLabels.push_back(PoiLabel(sx, sy, textName, tags));
}

