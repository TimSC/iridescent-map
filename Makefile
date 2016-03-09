
all: test
test: cppo5m/o5m.cpp cppo5m/varint.cpp cppo5m/OsmData.cpp ReadInput.cpp ReadGzip.cpp TagPreprocessor.cpp Regrouper.cpp drawlib/drawlibcairo.cpp drawlib/drawlib.cpp MapRender.cpp Transform.cpp Style.cpp LabelEngine.cpp
	g++ cppo5m/o5m.cpp cppo5m/varint.cpp cppo5m/OsmData.cpp ReadInput.cpp ReadGzip.cpp TagPreprocessor.cpp Regrouper.cpp drawlib/drawlibcairo.cpp drawlib/drawlib.cpp MapRender.cpp Transform.cpp Style.cpp  LabelEngine.cpp `pkg-config --cflags --libs gtk+-2.0` -lz -o test

