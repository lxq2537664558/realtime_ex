namespace base
{
	namespace string_util
	{
		template <class T>
		std::string convert_to_str(const T& v)
		{
			std::ostringstream oss;
			oss << v;
			return oss.str();
		}

		std::string convert_to_str(const CVariant& v)
		{
			switch (v.getType())
			{
			case eVVT_String:
				{
					if ((const char*)v == nullptr)
						return "";
					return std::string((const char*)v, v.getSize());
				}
				break;

			case eVVT_Int32:
			case eVVT_Int64:
				{
					std::ostringstream oss;
					oss << (int64_t)v;
					return oss.str();
				}
				break;

			case eVVT_UInt32:
			case eVVT_UInt64:
				{
					std::ostringstream oss;
					oss << (uint64_t)v;
					return oss.str();
				}
				break;

			case eVVT_Double:
				{
					std::ostringstream oss;
					oss << (double)v;
					return oss.str();
				}
				break;

			default:
				break;
			}

			return "";
		}

		template <class T>
		bool convert_to_value(const std::string& str, T& v)
		{
			std::istringstream iss(str);
			iss >> v;
			if (iss.fail())
				return false;

			return true;
		}

		template <class T>
		bool convert_to_value(const char* str, T& v)
		{
			if (str == nullptr)
				return false;

			std::istringstream iss(str);
			iss >> v;
			if (iss.fail())
				return false;

			return true;
		}

		std::string& to_lower(std::string& str)
		{
			std::transform(str.begin(), str.end(), str.begin(), ::tolower);
			return str;
		}

		std::string& to_upper(std::string& str)
		{
			std::transform(str.begin(), str.end(), str.begin(), ::toupper);
			return str;
		}

		bool start_with(const std::string& str, const std::string& substr)
		{
			return str.find(substr) == 0;
		}

		bool end_with(const std::string& str, const std::string& substr)
		{
			return str.rfind(substr) == (str.length() - substr.length());
		}

		std::string& ltrim(std::string& str, const char* delim)
		{
			str.erase(0, str.find_first_not_of(delim));
			return str;
		}

		std::string& rtrim(std::string& str, const char* delim)
		{
			str.erase(str.find_last_not_of(delim) + 1);
			return str;
		}

		std::string& trim(std::string& str, const char* delim)
		{
			ltrim(str, delim);
			rtrim(str, delim);
			return str;
		}

		std::string& erase(std::string& str, char c)
		{
			str.erase(std::remove_if(str.begin(), str.end(), std::bind2nd(std::equal_to<char>(), c)), str.end());
			return str;
		}

		std::string& replace_all(std::string& str, const char* delim, const char* s)
		{
			size_t len = strlen(delim);
			size_t pos = str.find(delim);
			while (pos != std::string::npos)
			{
				str.replace(pos, len, s);
				pos = str.find(delim, pos);
			}

			return str;
		}

		void split(const std::string& str, std::vector<std::string>& result, const char* delim)
		{
			if (str.empty())
				return;

			size_t size = base::function_util::strnlen(delim, _TRUNCATE);

			std::string::const_iterator substart = str.begin(), subend;
			while (true)
			{
				subend = std::search(substart, str.end(), delim, delim + size);
				std::string temp(substart, subend);

				if (!temp.empty())
				{
					result.push_back(temp);
				}

				if (subend == str.end())
					break;

				substart = subend + size;
			}
		}

		struct PushArgs
		{
			template<class T, class...Args>
			static bool push(std::vector<std::string> &vec, const T& t, const Args&... args)
			{
				std::ostringstream oss;
				oss << t;
				vec.push_back(oss.str().c_str());

				return PushArgs::push(vec, args...);
			}

			static bool push(std::vector<std::string> &vec)
			{
				return true;
			}
		};

		template<class ...Args>
		std::string	format(const char* fmt, const Args&... args)
		{
			std::vector<std::string> vecArg;
			PushArgs::push(vecArg, args...);

			// ½âÎö²ÎÊı
			size_t size = vecArg.size();
			std::string::size_type pos = 0;
			size_t index = 0;
			std::string szResult = fmt;

			while (std::string::npos != (pos = szResult.find("{}", pos)) && index < size)
			{
				szResult.replace(pos, 2, vecArg[index]);
				pos += vecArg[index].size();
				index++;
			}

			return szResult;
		}

		template<class ...Args>
		std::string	format(const std::string& fmt, const Args&... args)
		{
			return format(fmt.c_str(), args...);
		}

		bool string_cmp_nocase::operator ()(const std::string& lhs, const std::string& rhs) const
		{
			std::string::const_iterator p = lhs.begin();
			std::string::const_iterator p2 = rhs.begin();

			while (p != lhs.end() && p2 != rhs.end())
			{
				if (toupper(*p) != toupper(*p2))
					return (toupper(*p) < toupper(*p2) ? 1 : 0);
				++p;
				++p2;
			}

			return (lhs.size() == rhs.size()) ? 0 : (lhs.size() < rhs.size()) ? 1 : 0;
		}

	}
}