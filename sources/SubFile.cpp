#include "fsal.h"
#include "SubFile.h"

#include <cstdio>

using namespace fsal;

SubFile::SubFile(): m_file(nullptr)
{

}

SubFile::~SubFile()
{}

bool SubFile::ok() const
{
	return m_file->ok();
}

path SubFile::GetPath() const
{
	return m_file->GetPath();
}

Status SubFile::ReadData(uint8_t* dst, size_t size, size_t* bytesRead)
{

	return m_file->ReadData(dst, size, bytesRead);
}

Status SubFile::WriteData(const uint8_t* src, size_t size)
{
	return m_file->WriteData(src, size);
}

Status SubFile::SetPosition(size_t position) const
{
}

size_t SubFile::GetPosition() const
{
}

size_t SubFile::GetSize() const
{
}

Status SubFile::FlushBuffer() const
{
	return m_file->FlushBuffer();
}
