namespace base
{
	namespace string_util
	{
		template<size_t N, class T>
		inline void push2Special(std::string& strfmt, const T& val)
		{
			//static_assert(false, "not suppert val object");
			std::ostringstream oss;
			oss << val;

			// 替换参数
			std::string::size_type pos = 0;
			if (std::string::npos != (pos = strfmt.find("{}")))
			{
				strfmt.replace(pos, 2, oss.str().c_str());
			}
		}

		template<class T>
		inline void push2Special(std::string& strfmt, const T& val)
		{
			//static_assert(false, "not suppert val object");
			std::ostringstream oss;
			oss << val;

			// 替换参数
			std::string::size_type pos = 0;
			if (std::string::npos != (pos = strfmt.find("{}")))
			{
				strfmt.replace(pos, 2, oss.str().c_str());
			}
		}

		template<> inline void push2Special(std::string& strfmt, const bool& val)
		{
			// 替换参数
			std::string::size_type pos = 0;
			if (std::string::npos != (pos = strfmt.find("{}")))
			{
				if (val)
				{
					strfmt.replace(pos, 2, "true");
				}
				else
				{
					strfmt.replace(pos, 2, "false");
				}
			}
		}

		template<> inline void push2Special(std::string& strfmt, const int8_t& val)
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

		template<> inline void push2Special(std::string& strfmt, const uint8_t& val)
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

		template<> inline void push2Special(std::string& strfmt, const int16_t& val)
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

		template<> inline void push2Special(std::string& strfmt, const uint16_t& val)
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

		template<> inline void push2Special(std::string& strfmt, const int32_t& val)
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

		template<> inline void push2Special(std::string& strfmt, const uint32_t& val)
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

		template<> inline void push2Special(std::string& strfmt, const int64_t& val)
		{
			char sta_buffer[32];
			base::function_util::snprintf(sta_buffer, _countof(sta_buffer), INT64FMT, val);

			// 替换参数
			std::string::size_type pos = 0;
			if (std::string::npos != (pos = strfmt.find("{}")))
			{
				strfmt.replace(pos, 2, sta_buffer);
			}
		}

		template<> inline void push2Special(std::string& strfmt, const uint64_t& val)
		{
			char sta_buffer[32];
			base::function_util::snprintf(sta_buffer, _countof(sta_buffer), UINT64FMT, val);

			// 替换参数
			std::string::size_type pos = 0;
			if (std::string::npos != (pos = strfmt.find("{}")))
			{
				strfmt.replace(pos, 2, sta_buffer);
			}
		}

		template<> inline void push2Special(std::string& strfmt, const double& val)
		{
			char sta_buffer[16];
			base::function_util::snprintf(sta_buffer, _countof(sta_buffer), "%f", val);

			// 替换参数
			std::string::size_type pos = 0;
			if (std::string::npos != (pos = strfmt.find("{}")))
			{
				strfmt.replace(pos, 2, sta_buffer);
			}
		}

		template<> inline void push2Special(std::string& strfmt, const float& val)
		{
			push2Special(strfmt, static_cast<double>(val));
		}

		template<size_t N> inline void push2Special(std::string& strfmt, const char(&val)[N])
		{
			push2Special(strfmt, reinterpret_cast<const char* const>(val));
		}

		template<> inline void push2Special(std::string& strfmt, const char* const& val)
		{
			char sta_buffer[1024];
			if (val == nullptr)
			{
				base::function_util::snprintf(sta_buffer, _countof(sta_buffer), "%s", "NULL");
			}
			else
			{
				base::function_util::snprintf(sta_buffer, _countof(sta_buffer), "%s", val);
			}

			// 替换参数
			std::string::size_type pos = 0;
			if (std::string::npos != (pos = strfmt.find("{}")))
			{
				if (val == nullptr)
				{
					strfmt.replace(pos, 2, "NULL");
				}
				else
				{
					strfmt.replace(pos, 2, val);
				}
			}
		}

		template<> inline void push2Special(std::string& strfmt, char* const& val)
		{
			push2Special(strfmt, reinterpret_cast<const char* const>(val));
		}

		template<> inline void push2Special(std::string& strfmt, const std::string& val)
		{
			push2Special(strfmt, val.c_str());
		}

		struct PushArgs
		{
			template<class T, class...Args>
			static bool push(std::string& strfmt, const T& t, const Args&... args)
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
		std::string	format(const char* szfmt, const Args&...args)
		{
			std::string str_format = szfmt;

			PushArgs::push(str_format, args...);


			return str_format;
		}

		template<class ...Args>
		std::string	format(const std::string& strfmt, const Args&... args)
		{

			return format(strfmt.c_str(), args...);
		}
	}
}