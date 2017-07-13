// stdafx.cpp : source file that includes just the standard includes
// GameServer.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

#pragma warning (push)
#pragma warning (disable : 4267)
#include "../../protocol/commonProtoLib/cpp/Game.pb.cc"
#pragma warning (pop)
#include "GameDataTable-odb.cxx"
#include "PlatformDB-odb.cxx"
