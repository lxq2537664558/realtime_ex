namespace base
{
	namespace string_util
	{

		template<class T>
		inline void push2Special(std::string& strfmt, T val)
		{
			static_assert(false, "not suppert val object");
			
		}

		template<> inline void push2Special(std::string& strfmt, int8_t val)
		{
			char sta_buffer[16];
			base::function_util::snprintf(sta_buffer, _countof(sta_buffer), "%d", val);

			// 替换参数
			std::string::size_type pos = 0;
			if (std::string::npos != (pos = strfmt.find("{}")))
			{
				strfmt.replace(pos, 2, sta_buffer);
			}
		}

		template<> inline void push2Special(std::string& strfmt, uint8_t val)
		{
			char sta_buffer[16];
			base::function_util::snprintf(sta_buffer, _countof(sta_buffer), "%u", val);

			// 替换参数
			std::string::size_type pos = 0;
			if (std::string::npos != (pos = strfmt.find("{}")))
			{
				strfmt.replace(pos, 2, sta_buffer);
			}
		}

		template<> inline void push2Special(std::string& strfmt, int16_t val)
		{
			char sta_buffer[16];
			base::function_util::snprintf(sta_buffer, _countof(sta_buffer), "%d", val);

			// 替换参数
			std::string::size_type pos = 0;
			if (std::string::npos != (pos = strfmt.find("{}")))
			{
				strfmt.replace(pos, 2, sta_buffer);
			}
		}

		template<> inline void push2Special(std::string& strfmt, uint16_t val)
		{
			char sta_buffer[16];
			base::function_util::snprintf(sta_buffer, _countof(sta_buffer), "%u", val);

			// 替换参数
			std::string::size_type pos = 0;
			if (std::string::npos != (pos = strfmt.find("{}")))
			{
				strfmt.replace(pos, 2, sta_buffer);
			}
		}

		template<> inline void push2Special(std::string& strfmt, int32_t val)
		{
			char sta_buffer[16];
			base::function_util::snprintf(sta_buffer, _countof(sta_buffer), "%d", val);

			// 替换参数
			std::string::size_type pos = 0;
			if (std::string::npos != (pos = strfmt.find("{}")))
			{
				strfmt.replace(pos, 2, sta_buffer);
			}
		}

		template<> inline void push2Special(std::string& strfmt, uint32_t val)
		{
			char sta_buffer[16];
			base::function_util::snprintf(sta_buffer, _countof(sta_buffer), "%u", val);

			// 替换参数
			std::string::size_type pos = 0;
			if (std::string::npos != (pos = strfmt.find("{}")))
			{
				strfmt.replace(pos, 2, sta_buffer);
			}
		}

		template<> inline void push2Special(std::string& strfmt, int64_t val)
		{
			char sta_buffer[32];
			base::function_util::snprintf(sta_buffer, _countof(sta_buffer), "%I64d", val);

			// 替换参数
			std::string::size_type pos = 0;
			if (std::string::npos != (pos = strfmt.find("{}")))
			{
				strfmt.replace(pos, 2, sta_buffer);
			}
		}

		template<> inline void push2Special(std::string& strfmt, uint64_t val)
		{
			char sta_buffer[32];

			sprintf_s(sta_buffer, "%I64u", val);

			// 替换参数
			std::string::size_type pos = 0;
			if (std::string::npos != (pos = strfmt.find("{}")))
			{
				strfmt.replace(pos, 2, sta_buffer);
			}
		}

		template<> inline void push2Special(std::string& strfmt, double val)
		{
			char sta_buffer[16];
			sprintf_s(sta_buffer, "%f", val);

			// 替换参数
			std::string::size_type pos = 0;
			if (std::string::npos != (pos = strfmt.find("{}")))
			{
				strfmt.replace(pos, 2, sta_buffer);
			}
		}

		template<> inline void push2Special(std::string& strfmt, float val)
		{
			push2Special(strfmt, static_cast<double>(val));
		}

		template<> inline void push2Special(std::string& strfmt, char* val)
		{
			push2Special(strfmt, val);
		}

		template<> inline void push2Special(std::string& strfmt, const char* val)
		{
			char sta_buffer[1024];
			sprintf_s(sta_buffer, "%s", val);

			// 替换参数
			std::string::size_type pos = 0;
			if (std::string::npos != (pos = strfmt.find("{}")) )
			{
				strfmt.replace(pos, 2, sta_buffer);
			}

		}

		template<> inline void push2Special(std::string& strfmt, const std::string& val)
		{
			push2Special(strfmt, val.c_str());
		}

		template<> inline void push2Special(std::string& strfmt, std::string val)
		{
			push2Special(strfmt, val.c_str());
		}


		struct PushArgs
		{
			template<class T, class...Args>
			static bool push(std::string& strfmt, T& t, Args&... args)
			{
				push2Special(strfmt, t);

				return PushArgs::push(strfmt, args...);
			}

			static bool push(std::string& strfmt)
			{
				return true;
			}
		};

		template<class ...Args>
		std::string	format(const char* szfmt, Args... args)
		{
			std::string str_format = szfmt;

			PushArgs::push(str_format, args...);


			return str_format;
		}

		template<class ...Args>
		std::string	format(const std::string& strfmt, Args... args)
		{

			return format(strfmt.c_str(), args...);
		}
	}
}