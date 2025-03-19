#include "Debug/DebugGraph.hpp"

#include <limits>
#include <cstdio>

#include "Engine/Engine.hpp"
#include "Debug/Debug.hpp"

#include "System/Time.hpp"
#include "Debug/DebugVectorRenderer.hpp"

namespace kokko
{

DebugGraph::DebugGraph(Allocator* allocator, kokko::DebugVectorRenderer* vr) :
	vectorRenderer(vr),
	data(allocator),
	pointStorage(allocator),
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
	size_t count = data.GetCount();

	// Find range
	double min = std::numeric_limits<double>::max();
	double max = std::numeric_limits<double>::min();
	for (size_t i = 0; i < count; ++i)
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

	Color color(1.0f, 1.0f, 1.0f);

	if (cutoff < 0.0) cutoff = 0.0;

	if (count >= 2) // Can't draw unless there's at least 2 data points
	{
		pointStorage.Resize(count);

		for (size_t i = 0, count = data.GetCount(); i < count; ++i)
		{
			DataPoint& dataPoint = data.At(i);
			double y = (dataPoint.data - rangeMin) / rangeSize;
			double x = (dataPoint.time - cutoff) / this->timeRange;

			Vec3f pos;
			pos.x = drawArea.position.x + static_cast<float>(x) * drawArea.size.x;
			pos.y = drawArea.position.y + drawArea.size.y - (static_cast<float>(y) * drawArea.size.y);
			pos.z = 0.0f;

			pointStorage[i] = pos;
		}

		// TODO: Reimplement with Dear ImGui
		//vectorRenderer->DrawLineChainScreen(count, pointStorage.GetData(), color);
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

		for (size_t end = data.GetCount(); end > 0; --end)
		{
			const DataPoint point = data.At(end - 1);

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

} // namespace kokko
