#include "fsal.h"
#include "FileInterface.h"

using namespace fsal;

File::File()
{
}

File::File(FileInterface* file): m_file(file)
{
}

File::File(FileInterface* file, File::borrow): m_file(file, null_deleter)
{
}

File::operator bool() const
{
	return m_file != nullptr;
}

File::operator std::string() const
{
	std::string buff;
	buff.resize(GetSize());
	Seek(0);
	Read(const_cast<uint8_t*>((const uint8_t*)(buff.data())), buff.size());
	return buff;
}

void File::operator =(const std::string& x)
{
	Seek(0);
	Write(const_cast<uint8_t*>((const uint8_t*)(x.data())), x.size());
}

Status File::Read(uint8_t* destanation, size_t size, size_t* readBytes) const
{
	return m_file->ReadData(destanation, size, readBytes);
}

Status File::Write(const uint8_t* source, size_t size)
{
	return m_file->WriteData(source, size);
}

Status File::Seek(ptrdiff_t offset, Origin origin) const
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

Status File::Flush() const
{
	return m_file->FlushBuffer();
}

const uint8_t* File::GetDataPointer() const
{
	return m_file->GetDataPointer();
}

uint8_t* File::GetDataPointer()
{
	return m_file->GetDataPointer();
}