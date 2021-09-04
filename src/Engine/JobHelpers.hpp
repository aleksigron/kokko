#pragma once

#include "Engine/JobSystem.hpp"

class JobHelpers
{
public:
	template <typename DataType>
	static Job* CreateParallelFor(
		JobSystem* jobSystem,
		DataType* data,
		size_t count,
		void (*function)(DataType*, size_t),
		size_t splitCount)
	{
		using JobData = ParallerForJobData<DataType>;
		const JobData jobData{ data, count, function, splitCount };

		return jobSystem->CreateJobWithData(ParallelForJob<JobData>, jobData);
	}

private:
	template <typename DataType>
	struct ParallerForJobData
	{
		using ParallelForJobFunction = void(*)(DataType*, size_t);

		DataType* data;
		size_t count;
		ParallelForJobFunction function;
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
			const JobData leftData{ data->data, leftCount, data->function, data->splitCount };
			Job* left = jobSystem->CreateJobAsChildWithData(job, ParallelForJob<JobData>, leftData);
			jobSystem->Enqueue(left);

			const size_t rightCount = data->count - leftCount;
			const JobData rightData{ data->data + leftCount, rightCount, data->function, data->splitCount };
			Job* right = jobSystem->CreateJobAsChildWithData(job, ParallelForJob<JobData>, rightData);
			jobSystem->Enqueue(right);
		}
		else
		{
			// Execute the function on the range of data
			data->function(data->data, data->count);
		}
	}
};
