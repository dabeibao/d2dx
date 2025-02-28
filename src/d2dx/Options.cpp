/*
	This file is part of D2DX.

	Copyright (C) 2021  Bolrog

	D2DX is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	D2DX is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with D2DX.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <vector>
#include <string>
#include "pch.h"
#include "Options.h"
#include "Buffer.h"
#include "Utils.h"

#include "../../thirdparty/toml/toml.h"

using namespace d2dx;

Options::Options()
{
}

Options::~Options() noexcept
{
}

_Use_decl_annotations_
void Options::ApplyCfg(
	const char* cfg)
{
	auto cfgLen = strlen(cfg);

	if (cfgLen > 65536)
	{
		D2DX_FATAL_ERROR("Configuration file size limit exceeded.");
	}

	Buffer<char> cfgTemp{ cfgLen + 1, true };
	Buffer<char> errorMsg{ 1024, true };

	strcpy_s(cfgTemp.items, cfgTemp.capacity, cfg);

	auto root = toml_parse(cfgTemp.items, errorMsg.items, errorMsg.capacity);

	if (!root)
	{
		return;
	}

	toml_table_t* optouts = toml_table_in(root, "optouts");

	if (optouts)
	{
		toml_datum_t datum;

#define READ_OPTOUTS_FLAG(flag, cfgStringName) \
	datum = toml_bool_in(optouts, cfgStringName); \
	if (datum.ok) \
	{ \
		SetFlag(flag, datum.u.b); \
	}

		READ_OPTOUTS_FLAG(OptionsFlag::NoClipCursor, "noclipcursor");
		READ_OPTOUTS_FLAG(OptionsFlag::NoFpsFix, "nofpsfix");
		READ_OPTOUTS_FLAG(OptionsFlag::NoResMod, "noresmod");
		READ_OPTOUTS_FLAG(OptionsFlag::NoWide, "nowide");
		READ_OPTOUTS_FLAG(OptionsFlag::NoLogo, "nologo");
		READ_OPTOUTS_FLAG(OptionsFlag::NoVSync, "novsync");
		READ_OPTOUTS_FLAG(OptionsFlag::NoAntiAliasing, "noaa");
		READ_OPTOUTS_FLAG(OptionsFlag::NoCompatModeFix, "nocompatmodefix");
		READ_OPTOUTS_FLAG(OptionsFlag::NoTitleChange, "notitlechange");
		READ_OPTOUTS_FLAG(OptionsFlag::NoMotionPrediction, "nomotionprediction");
		READ_OPTOUTS_FLAG(OptionsFlag::NoHide, "nohide");

#undef READ_OPTOUTS_FLAG
	}

	auto game = toml_table_in(root, "game");

	if (game)
	{
		auto gameSize = toml_array_in(game, "size");
		if (gameSize)
		{
			auto w = toml_int_at(gameSize, 0);
			auto h = toml_int_at(gameSize, 1);

			if (w.ok && h.ok)
			{
				SetUserSpecifiedGameSize({ (int32_t)w.u.i, (int32_t)h.u.i });
			}
		}

		auto filtering = toml_int_in(game, "filtering");
		if (filtering.ok)
		{
			_filtering = (FilteringOption)filtering.u.i;
		}

                auto load = toml_array_in(game, "load");
                if (load)
                {
                    int n = toml_array_nelem(load);
                    for (int i = 0; i < n; i += 1)
                    {
                        auto name = toml_string_at(load, i);
                        if (name.ok)
                        {
                            LoadLibraryA(name.u.s);
                        }
                    }
                }
	}

	auto window = toml_table_in(root, "window");

	if (window)
	{
		auto windowScale = toml_double_in(window, "scale");
		if (windowScale.ok)
		{
			SetWindowScale(windowScale.u.d);
		}

		auto windowPosition = toml_array_in(window, "position");
		if (windowPosition)
		{
			auto x = toml_int_at(windowPosition, 0);
			auto y = toml_int_at(windowPosition, 1);
			
			if (x.ok && y.ok)
			{
				SetWindowPosition({ (int32_t)x.u.i, (int32_t)y.u.i });
			}
		}

		auto frameless = toml_bool_in(window, "frameless");
		if (frameless.ok)
		{
			SetFlag(OptionsFlag::Frameless, frameless.u.b);
		}
	}

	auto debug = toml_table_in(root, "debug");

	if (debug)
	{
		auto dumpTextures = toml_bool_in(debug, "dumptextures");
		if (dumpTextures.ok)
		{
			SetFlag(OptionsFlag::DbgDumpTextures, dumpTextures.u.b);
		}
	}


	toml_free(root);
}

namespace {

void splitCmdLine(const char * cmdLine, std::vector<std::string>& argv)
{
    int         start = 0;

    for (;;) {
        char    c;
        int     end;

        for (;;) {
            c = cmdLine[start];
            if (c == ' ' || c == '\t') {
                start += 1;
                continue;
            }
            break;
        }
        if (c == '\0') {
            break;
        }
        end = start + 1;
        for (;;) {
            c = cmdLine[end];
            if (c == '\0' || c == ' ' || c == '\t') {
                break;
            }
            end += 1;
        }
        argv.push_back(std::string(&cmdLine[start], end - start));
        start = end;
    }
}

}

_Use_decl_annotations_
void Options::ApplyCommandLine(
	const char* cmdLine)
{
	if (strstr(cmdLine, "-dxnoclipcursor")) SetFlag(OptionsFlag::NoClipCursor, true);
	if (strstr(cmdLine, "-dxnofpsfix")) SetFlag(OptionsFlag::NoFpsFix, true);
	if (strstr(cmdLine, "-dxnoresmod")) SetFlag(OptionsFlag::NoResMod, true);
	if (strstr(cmdLine, "-dxnowide")) SetFlag(OptionsFlag::NoWide, true);
	if (strstr(cmdLine, "-dxnologo")) SetFlag(OptionsFlag::NoLogo, true);
	if (strstr(cmdLine, "-dxnovsync")) SetFlag(OptionsFlag::NoVSync, true);
	if (strstr(cmdLine, "-dxnoaa")) SetFlag(OptionsFlag::NoAntiAliasing, true);
	if (strstr(cmdLine, "-dxnocompatmodefix")) SetFlag(OptionsFlag::NoCompatModeFix, true);
	if (strstr(cmdLine, "-dxnotitlechange")) SetFlag(OptionsFlag::NoTitleChange, true);
	if (strstr(cmdLine, "-dxnomop")) SetFlag(OptionsFlag::NoMotionPrediction, true);

	if (strstr(cmdLine, "-dxscale3")) SetWindowScale(3);
	else if (strstr(cmdLine, "-dxscale2")) SetWindowScale(2);

	if (strstr(cmdLine, "-dxdbg_dump_textures")) SetFlag(OptionsFlag::DbgDumpTextures, true);
	if (strstr(cmdLine, "-nohide")) SetFlag(OptionsFlag::NoHide, true);
        std::vector<std::string> argv;

        splitCmdLine(cmdLine, argv);
        unsigned int i = 0;

        while (i < argv.size()) {
            if (argv[i] == "-title" && i < argv.size() - 1) {
                _winTitle = argv[i + 1];
                i += 2;
                continue;
            }
            if (argv[i] == "-pdir" && i < argv.size() - 1) {
                _pDir = argv[i + 1];
                i += 2;
                continue;
            }
            i += 1;
            continue;
        }
}

_Use_decl_annotations_
bool Options::GetFlag(
	OptionsFlag flag) const
{
	return (_flags & (1 << (int32_t)flag)) != 0;
}

_Use_decl_annotations_
void Options::SetFlag(
	OptionsFlag flag,
	bool value) 
{
	uint32_t mask = 1 << (int32_t)flag;
	_flags &= ~mask;
	if (value)
	{
		_flags |= mask;
	}
}

double Options::GetWindowScale() const
{
	return _windowScale;
}

void Options::SetWindowScale(
	_In_ double windowScale)
{
	_windowScale = min(3, max(1, windowScale));
}

Offset Options::GetWindowPosition() const
{
	return _windowPosition;
}

void Options::SetWindowPosition(
	_In_ Offset windowPosition)
{
	_windowPosition = { max(-1, windowPosition.x), max(-1, windowPosition.y) };
}

Size Options::GetUserSpecifiedGameSize() const
{
	return _userSpecifiedGameSize;
}

void Options::SetUserSpecifiedGameSize(
	_In_ Size size)
{
	_userSpecifiedGameSize = { max(-1, size.width), max(-1, size.height) };
}

FilteringOption Options::GetFiltering() const
{
	return _filtering;
}
