#pragma once

#include "Queue.hpp"
#include "Rectangle.hpp"

class DebugVectorRenderer;

class DebugGraph
{
private:
	struct DataPoint
	{
		double data;
		double time;
	};

	DebugVectorRenderer* vectorRenderer;
	Rectanglef drawArea;
	Queue<DataPoint> data;

	double timeRange;

public:
	DebugGraph(DebugVectorRenderer* vr);
	~DebugGraph();

	void SetDrawArea(const Rectanglef& area) { drawArea = area; }

	void Update();
	void DrawToVectorRenderer();

	void AddDataPoint(double data);

	double GetAverageOverLastSeconds(double seconds);
};
