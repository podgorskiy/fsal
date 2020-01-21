#pragma once
#include "fsal_common.h"
#include "FileInterface.h"

#include <cstdio>

namespace fsal
{
	class SubFile : public FileInterface
	{
	public:
		SubFile();

		~SubFile() override;

		bool ok() const override;

		path GetPath() const override;

		Status Open(path filepath, Mode mode) override { return Status::Failed(); };

		Status ReadData(uint8_t* dst, size_t size, size_t* bytesRead) override;

		Status WriteData(const uint8_t* src, size_t size)  override;

		Status SetPosition(size_t position) const  override;

		size_t GetPosition() const  override;

		size_t GetSize() const  override;

		Status FlushBuffer() const override;

		const uint8_t* GetDataPointer() const  override { return nullptr; };

		uint8_t* GetDataPointer()  override { return nullptr; };
	private:
		std::shared_ptr<FileInterface> m_file;
		path m_path;

		size_t m_size;
		size_t m_offset;
		mutable size_t m_pointer;
	};
}
