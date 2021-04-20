#pragma once

struct CameraId
{
	unsigned int i;

	bool operator==(CameraId other) { return i == other.i; }
	bool operator!=(CameraId other) { return !operator==(other); }

	static const CameraId Null;
};
