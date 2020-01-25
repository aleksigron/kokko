#include "Debug/DebugGraph.hpp"

#include <limits>
#include <cstdio>

#include "Engine.hpp"
#include "Debug/Debug.hpp"
#include "Debug/DebugLog.hpp"

#include "System/Time.hpp"
#include "Debug/DebugVectorRenderer.hpp"

DebugGraph::DebugGraph(DebugVectorRenderer* vr) :
	vectorRenderer(vr),
	timeRange(5.0)
{
}

DebugGraph::~DebugGraph()
{
}

void DebugGraph::Update()
{
	double timeNow = Time::GetRunningTime();
	double cutoff = timeNow - this->timeRange;

	while (data.GetCount() > 0 && data.Peek().time < cutoff)
	{
		data.Pop();
	}
}

void DebugGraph::DrawToVectorRenderer()
{
	// Find range
	double min = std::numeric_limits<double>::max();
	double max = std::numeric_limits<double>::min();
	for (unsigned i = 0, count = data.GetCount(); i < count; ++i)
	{
		double value = data.At(i).data;
		if (value < min) min = value;
		if (value > max) max = value;
	}

	double diff = max - min;
	double rangeMin = min - diff * 0.1;
	double rangeMax = max + diff * 0.1;
	double rangeSize = rangeMax - rangeMin;

	double timeNow = Time::GetRunningTime();
	double cutoff = timeNow - timeRange;

	if (cutoff < 0.0) cutoff = 0.0;

	if (data.GetCount() >= 2) // Can't draw unless there's at least 2 data points
	{
		Vec2f prev;
		{
			DataPoint& dataPoint = data.At(0);
			double y = (dataPoint.data - rangeMin) / rangeSize;
			double x = (dataPoint.time - cutoff) / this->timeRange;

			prev.x = drawArea.position.x + static_cast<float>(x) * drawArea.size.x;
			prev.y = drawArea.position.y + drawArea.size.y - (static_cast<float>(y) * drawArea.size.y);
		}

		for (unsigned i = 1, count = data.GetCount(); i < count; ++i)
		{
			DataPoint& dataPoint = data.At(i);
			double y = (dataPoint.data - rangeMin) / rangeSize;
			double x = (dataPoint.time - cutoff) / this->timeRange;

			Vec2f pos;
			pos.x = drawArea.position.x + static_cast<float>(x) * drawArea.size.x;
			pos.y = drawArea.position.y + drawArea.size.y - (static_cast<float>(y) * drawArea.size.y);

			vectorRenderer->DrawLineScreen(prev, pos, Color(1.0f, 1.0f, 1.0f));

			prev = pos;
		}
	}
}

void DebugGraph::AddDataPoint(double value)
{
	DataPoint& dataPoint = data.Push();
	dataPoint.data = value;
	dataPoint.time = Time::GetRunningTime();
}

double DebugGraph::GetAverageOverLastSeconds(double seconds)
{
	if (data.GetCount() > 0)
	{
		double lastTime = data.At(data.GetCount() - 1).time - seconds;

		double total = 0.0;
		int framesAdded = 0;

		for (int i = data.GetCount() - 1; i >= 0; --i)
		{
			const DataPoint point = data.At(i);

			if (point.time > lastTime)
			{
				total += point.data;
				++framesAdded;
			}
			else
				break;
		}

		return total / framesAdded;
	}
	else
		return 0.0;
}
