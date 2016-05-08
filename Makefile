
all: test
test: cppo5m/o5m.cpp cppo5m/varint.cpp cppo5m/OsmData.cpp ReadInput.cpp ReadGzip.cpp TagPreprocessor.cpp Regrouper.cpp drawlib/drawlibcairo.cpp drawlib/drawlib.cpp drawlib/cairotwisted.cpp drawlib/RdpSimplify.cpp drawlib/LineLineIntersect.cpp MapRender.cpp Transform.cpp Style.cpp LabelEngine.cpp TriTri2d.cpp CompletePoly.cpp Coast.cpp test.cpp
	g++ cppo5m/o5m.cpp cppo5m/varint.cpp cppo5m/OsmData.cpp ReadInput.cpp ReadGzip.cpp TagPreprocessor.cpp Regrouper.cpp drawlib/drawlibcairo.cpp drawlib/drawlib.cpp drawlib/cairotwisted.cpp drawlib/RdpSimplify.cpp drawlib/LineLineIntersect.cpp MapRender.cpp Transform.cpp Style.cpp  LabelEngine.cpp TriTri2d.cpp CompletePoly.cpp Coast.cpp test.cpp `pkg-config --cflags --libs gtk+-2.0` -lz -o test

