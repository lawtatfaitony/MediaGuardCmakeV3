#pragma once
#include <string>
#include "easylogging++.h"

namespace Config
{
	class EasyLogHelper
	{
	public:
		EasyLogHelper();
		virtual ~EasyLogHelper();

		/*FOR log.conf 形式配置*/
		static bool InitConLog();

		static bool InitLogging();
	};
}




