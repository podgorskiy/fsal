#include "fsal.h"
#include "FileInterface.h"

using namespace fsal;

File::File()
{
}

File::File(FileInterface* file): m_file(file)
{
}

bool File::ok() const
{
	return m_file != nullptr;
}

Status File::Read(uint8_t* destanation, size_t size, size_t* readBytes)
{
	return m_file->ReadData(destanation, size, readBytes);
}

Status File::Write(const uint8_t* source, size_t size)
{
	return m_file->WriteData(source, size);
}

Status File::Seek(ptrdiff_t offset, Origin origin)
{
	switch (origin)
	{
	case Origin::Beginning:
		return m_file->SetPosition(offset);
	case Origin::CurrentPosition:
		return m_file->SetPosition(m_file->GetPosition() + offset);
	case Origin::End:
		return m_file->SetPosition(m_file->GetSize() + offset);
	}
	return Status::Failed();
}

size_t File::Tell() const
{
	return m_file->GetPosition();
}

size_t File::GetSize() const
{
	return m_file->GetSize();
}

path File::GetPath() const
{
	return m_file->GetPath();
}

Status File::Flush()
{
	return m_file->FlushBuffer();
}
