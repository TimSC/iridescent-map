
all: test
test: cppo5m/o5m.cpp cppo5m/varint.cpp cppo5m/OsmData.cpp ReadInput.cpp TagPreprocessor.cpp Regrouper.cpp LayerDrawManager.cpp
	g++ cppo5m/o5m.cpp cppo5m/varint.cpp cppo5m/OsmData.cpp ReadInput.cpp TagPreprocessor.cpp Regrouper.cpp LayerDrawManager.cpp -o test

