#include "utils.h"
#include <iostream>
#include <fstream>
#include <string>
#include <iostream>
#include <filesystem>
#include <stdio.h>
#include "inout.h"
#include <vector>
#include <array>
#define INI_STRNICMP(s1, s2, cnt) (strcmp(s1, s2))
#include "ini.h"
#include <SDL2/SDL.h>
#ifdef _WIN32
#include <windows.h>
#include "fex/fex.h"
#include "MemoryModulePP.h"
#else
#include <unistd.h>
#endif
using namespace std;

#ifdef _WIN32
static std::string_view SHLIB_EXTENSION = ".dll";
#else
static std::string_view SHLIB_EXTENSION = ".so";
#endif

namespace MudUtil
{

	std::string replace_all(std::string str, const std::string &from, const std::string &to)
	{
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos)
		{
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
		}
		return str;
	}

	unsigned get_filesize(const char *path)
	{
		std::ifstream is(path, std::ifstream::binary);
		if (is)
		{
			// get length of file:
			is.seekg(0, is.end);
			unsigned t = is.tellg();
			is.close();
			return t;
		}
		return 0;
	}

	std::string get_wtfwegname()
	{
#if defined(__linux__) // check defines for your setup
		std::array<char, 1024 * 4> buf{};
		auto written = readlink("/proc/self/exe", buf.data(), buf.size());
		if (written == -1)
			string("");
		return string(buf.data());
#elif defined(_WIN32)
		std::array<char, 1024 * 4> buf{};
		GetModuleFileNameA(nullptr, buf.data(), buf.size());
		return string(buf.data());
#else
		static_assert(false, "unrecognized platform");
#endif
	}

	uint32_t pow2up(uint32_t v)
	{
		return pow(2, ceil(log2(v)));
	}

	int clz(uint32_t x)
	{
		static const char debruijn32[32] = {
			0, 31, 9, 30, 3, 8, 13, 29, 2, 5, 7, 21, 12, 24, 28, 19,
			1, 10, 4, 14, 6, 22, 25, 20, 11, 15, 23, 26, 16, 27, 17, 18};
		return debruijn32[((x & -x) * 0x077CB531) >> 27];
	}

	void vector_appendbytes(std::vector<uint8_t> &vec, uint8_t *bytes, size_t len)
	{
		vec.insert(vec.end(), bytes, bytes + len);
	}

	std::vector<uint8_t> load_data(const char *path)
	{
		std::ifstream input(path, std::ifstream::binary);
		if (!input)
			return {};
		input.seekg(0, input.end);
		unsigned Size = input.tellg();
		input.seekg(0, input.beg);
		std::vector<uint8_t> Memory(Size, 0);
		input.read((char *)&Memory[0], Size);
		input.close();
		return Memory;
	}

	bool save_data(unsigned char *data, unsigned size, const char *path)
	{
		std::ofstream input(path, std::ofstream::binary | std::ios::trunc);
		if (!input.good())
			return false;
		input.write((char *)data, size);
		input.close();
		return true;
	}

	const char *get_filename_ext(const char *filename)
	{
		const char *dot = strrchr(filename, '.');
		if (!dot || dot == filename)
			return "";
		return dot + 1;
	}

	void *openlib(const char *path)
	{
#ifdef _WIN32
#ifndef DEBUG
		PMEMORYMODULE handle;
		if (strcmp(get_filename_ext(path), "zip") == 0 || strcmp(get_filename_ext(path), "7z") == 0 ||
			strcmp(get_filename_ext(path), "rar") == 0)
		{
			fex_t *fex = NULL;
			fex_err_t err = fex_open(&fex, path);
			if (err == NULL)
				while (!fex_done(fex))
				{
					if (fex_has_extension(fex_name(fex), ".dll"))
					{
						fex_stat(fex);
						int sz = fex_size(fex);
						char *buf = (char *)malloc(sz);
						fex_read(fex, buf, sz);
						handle = MemoryLoadLibrary(buf, sz);
						free(buf);
						if (!handle)
						{
							fex_close(fex);
							fex = NULL;
						}
						else
						{
							fex_close(fex);
							fex = NULL;
							return handle;
						}
						fex_next(fex);
					}
				}
			fex_close(fex);
		}

		std::vector<uint8_t> dll_ptr = load_data(path);
		handle = MemoryLoadLibrary(dll_ptr.data(), dll_ptr.size());
		if (!handle)
			return NULL;
		return handle;
#else
		void *handle = SDL_LoadObject(path);
		if (!handle)
			return NULL;
		return handle;
#endif

#else
		void *handle = SDL_LoadObject(path);
		if (!handle)
			return NULL;
		return handle;
#endif
	}
	void *getfunc(void *handle, const char *funcname)
	{
#ifdef _WIN32
#ifndef DEBUG
		return (void *)MemoryGetProcAddress((PMEMORYMODULE)handle, funcname);
#else
		return SDL_LoadFunction(handle, funcname);
#endif
#else
		return SDL_LoadFunction(handle, funcname);
#endif
	}
	void freelib(void *handle)
	{
#ifdef _WIN32
#ifndef DEBUG
		MemoryFreeLibrary((PMEMORYMODULE)handle);
#else
		SDL_UnloadObject(handle);
#endif
#else
		SDL_UnloadObject(handle);
#endif
	}

	const char *b64tb = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	std::string base64_encode(const std::string &in)
	{
		std::string out;
		int val = 0, valb = -6;
		for (unsigned char c : in)
		{
			val = (val << 8) + c;
			valb += 8;
			while (valb >= 0)
			{
				out.push_back(b64tb[(val >> valb) & 0x3F]);
				valb -= 6;
			}
		}
		if (valb > -6)
			out.push_back(b64tb[((val << 8) >> (valb + 8)) & 0x3F]);
		while (out.size() % 4)
			out.push_back('=');
		return out;
	}

	std::string base64_decode(const std::string &in)
	{
		std::string out;
		std::vector<int> T(256, -1);
		for (int i = 0; i < 64; i++)
			T[b64tb[i]] = i;
		int val = 0, valb = -8;
		for (unsigned char c : in)
		{
			if (T[c] == -1)
				break;
			val = (val << 6) + T[c];
			valb += 6;
			if (valb >= 0)
			{
				out.push_back(char((val >> valb) & 0xFF));
				valb -= 8;
			}
		}
		return out;
	}

}