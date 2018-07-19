#pragma once
#include "fsal_common.h"
#include "FileInterface.h"

#include <cstdio>

namespace fsal
{
	class StdFile : public FileInterface
	{
	public:
		StdFile();

		virtual ~StdFile() override;

		virtual bool ok() const override;

		virtual path GetPath() const override;

		virtual Status Open(path filepath, Mode mode) override;

		virtual Status ReadData(uint8_t* dst, size_t size, size_t* bytesRead) override;

		virtual Status WriteData(const uint8_t* src, size_t size)  override;

		virtual Status SetPosition(size_t position) const  override;

		virtual size_t GetPosition() const  override;

		virtual size_t GetSize() const  override;

		virtual Status FlushBuffer()  override;

		virtual const uint8_t* GetDataPointer() const  override;

		virtual uint8_t* GetDataPointer()  override;

	private:
		FILE* m_file;
		path m_path;
	};
}
