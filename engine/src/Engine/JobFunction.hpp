#pragma once

namespace kokko
{
struct Job;
class JobSystem;
using JobFunction = void(*)(Job*, JobSystem*);
}
