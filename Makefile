all: PlotData.cpp
	g++ -g -std=c++11 -Wall -o PlotData PlotData.cpp

clean:
	$(RM) PlotData
