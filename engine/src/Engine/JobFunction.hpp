#pragma once

struct Job;
class JobSystem;

using JobFunction = void(*)(Job*, JobSystem*);
