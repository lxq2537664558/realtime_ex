#pragma once

#include "core_common.h"

namespace core
{
	enum EHttpMethodType
	{
		eHMT_Unknown,
		eHMT_Get,
		eHMT_Post,
		eHMT_Put,
		eHMT_Delete
	};

	enum EHttpVersion
	{
		eHV_Unknown,
		eHV_Http10,
		eHV_Http11
	};

	enum EHttpParseState
	{
		eHPS_RequestLine,
		eHPS_Headers,
		eHPS_Body,
		eHPS_Finish,
	};
}