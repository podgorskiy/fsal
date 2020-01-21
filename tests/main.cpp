#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <fsal.h>
#include "doctest.h"


TEST_CASE("Filepath normalization")
{
		CHECK(fsal::NormalizePath("./test_folder/folder_inside/../folder_inside/./") == "test_folder/folder_inside/");
		CHECK(fsal::NormalizePath("test_folder/folder_inside/../folder_inside/") == "test_folder/folder_inside/");
		CHECK(fsal::NormalizePath("test_folder/../test_folder/./folder_inside/../folder_inside/") == "test_folder/folder_inside/");
		CHECK(fsal::NormalizePath("test_folder/folder_inside/../folder_inside/.") == "test_folder/folder_inside");
}

TEST_CASE("Filepaths")
{
	using fs = fsal::FileSystem;
	using loc = fsal::Location;
	printf("RoamingAppData: %s\n", fs::GetSystemPath(loc::kStorageSynced).string().c_str());
	printf("LocalAppData: %s\n", fs::GetSystemPath(loc::kStorageLocal).string().c_str());
	printf("UserPictures: %s\n", fs::GetSystemPath(loc::kPictures).string().c_str());
	printf("UserMusic: %s\n", fs::GetSystemPath(loc::kMusic).string().c_str());
	printf("ProgramData: %s\n", fs::GetSystemPath(loc::kWin_ProgramData).string().c_str());
	printf("ProgramFiles: %s\n", fs::GetSystemPath(loc::kWin_ProgramFiles).string().c_str());
	printf("Temp: %s\n", fs::GetSystemPath(loc::kTemp).string().c_str());
}

TEST_CASE("ReadWriteToFile")
{
	fsal::FileSystem fs;
	fs.PushSearchPath("../");

	{
		fsal::File f = fs.Open(fsal::Location("test.txt", fsal::Location::kCurrentDirectory), fsal::kWrite);
				CHECK(f);
		f = std::string("asdasdasd");
	}
	{
		fsal::File f = fs.Open(fsal::Location("test.txt", fsal::Location::kCurrentDirectory), fsal::kRead);
				CHECK(f);
		std::string str = f;
				CHECK(str == std::string("asdasdasd"));
	}
}

TEST_CASE("OpenZIP")
{
	fsal::FileSystem fs;
	fs.PushSearchPath("../");
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
		for (const auto& l: l1)
		{
			printf("%s\n", l.c_str());
		}
		auto l2 = zip.ListDirectory(".");
		for (const auto& l: l2)
		{
			printf("%s\n", l.c_str());
		}
	}
}

TEST_CASE("MountZIP")
{
	fsal::FileSystem fs;
	fs.PushSearchPath("../");
	{
		fsal::ZipReader zip;
		auto zipfile = fs.Open("test_archive.zip");
		CHECK(zipfile);

		fs.MountArchive(OpenZipArchive(zipfile));

		fsal::File file = fs.Open("test_folder/folder_inside/test_file.txt", fsal::kRead);
		CHECK(file);
		std::string str = file;
		CHECK(str == "test");
	}
}

TEST_CASE("MountVpk")
{
	fsal::FileSystem fs;
	fs.PushSearchPath("../");
	{
		auto archive = fsal::OpenVpkArchive(fs, "/mnt/StorageExt4/SteamLibrary/steamapps/common/Counter-Strike Global Offensive/csgo");
		fs.MountArchive(archive);
		fsal::File file = fs.Open("materials/panorama/images/map_icons/map_icon_de_dust2.svg", fsal::kRead);
		std::string content = file;
		fs.Open("map_icon_de_dust2.svg", fsal::Mode::kWrite) = content;

//
//		auto zipfile = fs.Open("test_archive.zip");
//		CHECK(zipfile);
//
//		fs.MountArchive(zipfile);
//
//		fsal::File file = fs.Open("test_folder/folder_inside/test_file.txt", fsal::kRead);
//		CHECK(file);
//		std::string str = file;
//		CHECK(str == "test");
	}
}
