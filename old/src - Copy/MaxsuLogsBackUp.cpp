#include "MaxsuLogsBackUp.h"


namespace MaxsuLogsBackUpNamespace
{

	bool CreateBackUpLogs(const std::wstring log_path, unsigned int logs_num)
	{
		if (!std::filesystem::exists(log_path)) 
			return false;



		for (unsigned int i = logs_num - 1; i >= 0; i--)
		{
			auto const thisbackfile = GetBackUpLogNameByIndex(log_path, i);

			auto const lastbackfile = GetBackUpLogNameByIndex(log_path, i - 1);

			if (i == 0)
			{
				if (std::filesystem::exists(log_path))
					std::filesystem::rename(log_path, thisbackfile);

				return true;
			}


			if (std::filesystem::exists(lastbackfile))
				std::filesystem::rename(lastbackfile, thisbackfile);

		}

		return false;

		//--------------------------------------------------------------

	}



}
