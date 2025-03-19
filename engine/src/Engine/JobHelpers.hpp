#pragma once

#include "Engine/JobSystem.hpp"

namespace kokko
{

class JobHelpers
{
public:
	template <typename InstanceData, typename ConstantData>
	static Job* CreateParallelFor(
		JobSystem* jobSystem,
		ConstantData* constantData,
		InstanceData* instanceData,
		size_t count,
		void (*function)(ConstantData*, InstanceData*, size_t),
		size_t splitCount)
	{
		using JobData = ParallerForJobData<InstanceData, ConstantData>;
		const JobData jobData{ constantData, instanceData, count, function, splitCount };

		return jobSystem->CreateJobWithData(ParallelForJob<JobData>, jobData);
	}

private:
	template <typename InstanceData, typename ConstantData>
	struct ParallerForJobData
	{
		ConstantData* constantData;
		InstanceData* instanceData;
		size_t count;
		void (*function)(ConstantData*, InstanceData*, size_t);
		size_t splitCount;
	};

	template <typename JobData>
	static void ParallelForJob(Job* job, JobSystem* jobSystem)
	{
		const JobData* data = job->GetPtrToData<JobData>();

		if (data->count > data->splitCount)
		{
			// Split data range in two

			const size_t leftCount = data->count / 2;
			const JobData leftData
			{
				data->constantData,
				data->instanceData,
				leftCount,
				data->function,
				data->splitCount
			};

			Job* left = jobSystem->CreateJobAsChildWithData(job, ParallelForJob<JobData>, leftData);
			jobSystem->Enqueue(left);

			const size_t rightCount = data->count - leftCount;
			const JobData rightData
			{
				data->constantData,
				&data->instanceData[leftCount],
				rightCount,
				data->function,
				data->splitCount
			};

			Job* right = jobSystem->CreateJobAsChildWithData(job, ParallelForJob<JobData>, rightData);
			jobSystem->Enqueue(right);
		}
		else
		{
			// Execute the function on the range of data
			data->function(data->constantData, data->instanceData, data->count);
		}
	}
};

} // namespace kokko
