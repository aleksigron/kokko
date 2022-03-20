#pragma once

#include "System/FilesystemResolver.hpp"

namespace kokko
{

class FilesystemResolverIdentity : public FilesystemResolver
{
public:
	virtual bool ResolvePath(const char*, String&) { return false; }
};

} // namespace kokko
