#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <iomanip>
#include <filesystem>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <cstdlib>

int main()
{
	/*
	TODO:
	- comment #define SERVER if it's not already
	- build
	- delete old SenderClient.exe ???
	- rename Sender.exe to SenderClient.exe
	- uncomment #define SERVER
	- build
	- run Sender.exe and SenderClient.exe
	*/
#if 1 // NOTE: comment #define SERVER if it's not already
	{
		std::stringstream oss;
		{
			std::ifstream ifs("main.cpp");
			std::string line;
			while (std::getline(ifs, line)) {
				if (line == "#define SERVER") {
					line.insert(0, "//");
				}
				oss << line << std::endl;
			}
		}
		{
			std::ofstream ofs("main.cpp");
			ofs << oss.rdbuf();
		}
	}
#endif
#if 1 // NOTE: build
	{
		std::system("msbuild Sender.vcxproj");
	}
#endif
#if 1 // NOTE: delete old SenderClient.exe ???
	{
		std::system("del bin\\Debug_x86\\SenderClient.exe");
	}
#endif
#if 1 // NOTE: rename Sender.exe to SenderClient.exe
	{
		std::system("rename bin\\Debug_x86\\Sender.exe SenderClient.exe");
	}
#endif
#if 1 // NOTE: uncomment #define SERVER
	{
		std::stringstream oss;
		{
			std::ifstream ifs("main.cpp");
			std::string line;
			while (std::getline(ifs, line)) {
				if (line == "//#define SERVER") {
					line.erase(0,2);
				}
				oss << line << std::endl;
			}
		}
		{
			std::ofstream ofs("main.cpp");
			ofs << oss.rdbuf();
		}
	}
#endif
#if 1 // NOTE: build
	{
		std::system("msbuild Sender.vcxproj");
	}
#endif
#if 1 // NOTE: run Sender.exe and SenderClient.exe
	{
		std::system("start bin\\Debug_x86\\Sender.exe");
		std::system("start bin\\Debug_x86\\SenderClient.exe");
	}
#endif
}