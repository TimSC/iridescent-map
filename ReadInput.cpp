#include "cppo5m/OsmData.h"
#include "TagPreprocessor.h"
#include "Regrouper.h"
#include <fstream>
#include <iostream>
using namespace std;



int main()
{
	std::ifstream fi("1374.o5m");

	class Regrouper regrouper;

	class TagPreprocessor tagPreprocessor;
	tagPreprocessor.output = &regrouper;	

	class O5mDecode dec(fi);
	dec.output = &tagPreprocessor;
	dec.DecodeHeader();

	while (!fi.eof())
		dec.DecodeNext();
	
	regrouper.UpdateIdMappings();

	IRegroupResultHandler handler;
	regrouper.FindAreas(&handler);
	
}

