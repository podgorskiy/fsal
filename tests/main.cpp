#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <fsal.h>
#include "doctest.h"


TEST_CASE("testing the factorial function")
{
	fsal::FileSystem fs;
	fs.PushSearchPath("../");

	{
		fsal::File f = fs.Open(fsal::Location("test.txt", fsal::Location::kCurrentDirectory), fsal::kWrite);
		CHECK(f);
		f << std::string("asdasdasd");
	}
	{
		fsal::File f = fs.Open(fsal::Location("test.txt", fsal::Location::kCurrentDirectory), fsal::kRead);
		CHECK(f);
		std::string str;
		f >> str;
		CHECK(str == std::string("asdasdasd"));
	}
	{
		CHECK(fsal::NormalizePath("./test_folder/folder_inside/../folder_inside/./") == "test_folder/folder_inside/");
		CHECK(fsal::NormalizePath("test_folder/folder_inside/../folder_inside/") == "test_folder/folder_inside/");
		CHECK(fsal::NormalizePath("test_folder/../test_folder/./folder_inside/../folder_inside/") == "test_folder/folder_inside/");
		//CHECK(fsal::NormalizePath("test_folder/folder_inside/../folder_inside/.") == "test_folder/folder_inside/");
	}
	{
		fsal::ZipReader zip;
		auto zipfile = fs.Open("test_archive.zip");
		CHECK(zipfile);

		fsal::Status r = zip.OpenArchive(zipfile);
		CHECK(r.ok());

		auto file = zip.OpenFile("test_folder/folder_inside/../folder_inside/./test_file.txt");
		CHECK(file);

		std::string str = file;
		CHECK(str == "test");

		auto l1 = zip.ListDirectory("./test_folder/folder_inside/../folder_inside/./");
		for (auto l: l1)
		{
			printf("%s\n", l.c_str());
		}
	}
}
