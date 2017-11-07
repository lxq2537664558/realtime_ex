#include <stdlib.h>
#include <sstream>

#include "libBaseCommon/logger.h"
#include "libBaseCommon/defer.h"
#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/string_util.h"

#include "google/protobuf/descriptor.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/util/json_util.h"
#include "google/protobuf/compiler/importer.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/io/tokenizer.h"

#include "proto_src/db_option.pb.h"

#include "db_protobuf.h"

namespace
{
	enum ESerializeType
	{
		eST_Unknown = 0,
		eST_Json = 1,
		eST_Protobuf_Bin = 2,
	};

	class CLoadProtobufErrorCollector :
		public google::protobuf::compiler::MultiFileErrorCollector
	{
	public:
		CLoadProtobufErrorCollector() {}
		~CLoadProtobufErrorCollector() {}

		void AddError(const std::string& filename, int line, int column, const std::string& message)
		{
			PrintWarning("protobuf error: {},{},{},{}", filename, line, column, message);
		}
	};

	class CProtobufTextParserErrorCollector :
		public google::protobuf::io::ErrorCollector
	{
	public:
		void AddError(int line, int column, const std::string& message)
		{
			PrintWarning("ERROR: Parse text. line = {}, column = {}, error = {}", line, column, message);
		}

		void AddWarning(int line, int column, const std::string& message)
		{
			PrintWarning("WARNING: Parse text. line = {}, column = {}, error = {}", line, column, message);
		}
	};

	class CMessageFactory
	{
	public:
		CMessageFactory();
		~CMessageFactory();

		static CMessageFactory* Inst();

		bool	init(const std::string& szDir, const std::vector<std::string>& vecFile);

		google::protobuf::Message*
				createMessage(const std::string& szName);

	private:
		google::protobuf::compiler::Importer*		m_pImporter;
		google::protobuf::DynamicMessageFactory*	m_pFactory;
	};

	CMessageFactory::CMessageFactory()
		: m_pImporter(nullptr)
		, m_pFactory(nullptr)
	{
	}

	CMessageFactory::~CMessageFactory()
	{
		SAFE_DELETE(this->m_pFactory);
		SAFE_DELETE(this->m_pImporter);
	}

	CMessageFactory* CMessageFactory::Inst()
	{
		static CMessageFactory s_Inst;

		return &s_Inst;
	}

	bool CMessageFactory::init(const std::string& szDir, const std::vector<std::string>& vecFile)
	{
		this->m_pFactory = new google::protobuf::DynamicMessageFactory();

		google::protobuf::compiler::DiskSourceTree sourceTree;
		CLoadProtobufErrorCollector errorColloctor;
		this->m_pImporter = new google::protobuf::compiler::Importer(&sourceTree, &errorColloctor);

		sourceTree.MapPath("", szDir);

		for (auto iter = vecFile.begin(); iter != vecFile.end(); ++iter)
		{
			DebugAstEx(this->m_pImporter->Import(*iter), false);
		}

		return true;
	}

	google::protobuf::Message* CMessageFactory::createMessage(const std::string& szName)
	{
		google::protobuf::Message* pMessage = nullptr;
		const google::protobuf::Descriptor* pDescriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(szName);
		if (pDescriptor != nullptr)
		{
			const google::protobuf::Message* pProto = google::protobuf::MessageFactory::generated_factory()->GetPrototype(pDescriptor);
			if (pProto != nullptr)
				pMessage = pProto->New();
		}
		else
		{
			pDescriptor = this->m_pImporter->pool()->FindMessageTypeByName(szName);
			if (pDescriptor != nullptr)
				pMessage = this->m_pFactory->GetPrototype(pDescriptor)->New();
		}

		return pMessage;
	}

	bool getBasicTypeFieldValue(const google::protobuf::Message* pMessage, const google::protobuf::FieldDescriptor* pFieldDescriptor, const google::protobuf::Reflection* pReflection, std::string& szValue)
	{
		DebugAstEx(pMessage != nullptr, false);
		DebugAstEx(pFieldDescriptor != nullptr, false);
		DebugAstEx(pReflection != nullptr, false);

		switch (pFieldDescriptor->type())
		{
		case google::protobuf::FieldDescriptor::TYPE_INT32:
		case google::protobuf::FieldDescriptor::TYPE_SINT32:
		case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
		{
			int32_t nValue = pReflection->GetInt32(*pMessage, pFieldDescriptor);
			szValue = base::string_util::convert_to_str(nValue);
		}
		break;

		case google::protobuf::FieldDescriptor::TYPE_UINT32:
		case google::protobuf::FieldDescriptor::TYPE_FIXED32:
		{
			uint32_t nValue = pReflection->GetUInt32(*pMessage, pFieldDescriptor);
			szValue = base::string_util::convert_to_str(nValue);
		}
		break;

		case google::protobuf::FieldDescriptor::TYPE_INT64:
		case google::protobuf::FieldDescriptor::TYPE_SINT64:
		case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
		{
			int64_t nValue = pReflection->GetInt64(*pMessage, pFieldDescriptor);
			szValue = base::string_util::convert_to_str(nValue);
		}
		break;

		case google::protobuf::FieldDescriptor::TYPE_UINT64:
		case google::protobuf::FieldDescriptor::TYPE_FIXED64:
		{
			uint64_t nValue = pReflection->GetUInt64(*pMessage, pFieldDescriptor);
			szValue = base::string_util::convert_to_str(nValue);
		}
		break;

		case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
		{
			double nValue = pReflection->GetDouble(*pMessage, pFieldDescriptor);
			szValue = base::string_util::convert_to_str(nValue);
		}
		break;

		case google::protobuf::FieldDescriptor::TYPE_FLOAT:
		{
			float nValue = pReflection->GetFloat(*pMessage, pFieldDescriptor);
			szValue = base::string_util::convert_to_str(nValue);
		}
		break;

		case google::protobuf::FieldDescriptor::TYPE_STRING:
		case google::protobuf::FieldDescriptor::TYPE_BYTES:
			szValue = pReflection->GetString(*pMessage, pFieldDescriptor);
			break;

		default:
			DebugAstEx(false, false);
		}

		return true;
	}

	bool setBasicTypeFieldValue(google::protobuf::Message* pMessage, const google::protobuf::Reflection* pReflection, const google::protobuf::FieldDescriptor* pFieldDescriptor, const std::string& szValue)
	{
		DebugAstEx(pMessage != nullptr, false);
		DebugAstEx(pReflection != nullptr, false);
		DebugAstEx(pFieldDescriptor != nullptr, false);

		switch (pFieldDescriptor->type())
		{
		case google::protobuf::FieldDescriptor::TYPE_INT32:
		case google::protobuf::FieldDescriptor::TYPE_SINT32:
		case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
		{
			int32_t nValue = 0;
			if (!base::string_util::convert_to_value(szValue, nValue))
				return false;

			pReflection->SetInt32(pMessage, pFieldDescriptor, nValue);
		}
		break;

		case google::protobuf::FieldDescriptor::TYPE_UINT32:
		case google::protobuf::FieldDescriptor::TYPE_FIXED32:
		{
			uint32_t nValue = 0;
			if (!base::string_util::convert_to_value(szValue, nValue))
				return false;

			pReflection->SetUInt32(pMessage, pFieldDescriptor, nValue);
		}
		break;

		case google::protobuf::FieldDescriptor::TYPE_INT64:
		case google::protobuf::FieldDescriptor::TYPE_SINT64:
		case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
		{
			int64_t nValue = 0;
			if (!base::string_util::convert_to_value(szValue, nValue))
				return false;

			pReflection->SetInt64(pMessage, pFieldDescriptor, nValue);
		}
		break;

		case google::protobuf::FieldDescriptor::TYPE_UINT64:
		case google::protobuf::FieldDescriptor::TYPE_FIXED64:
		{
			uint64_t nValue = 0;
			if (!base::string_util::convert_to_value(szValue, nValue))
				return false;

			pReflection->SetUInt64(pMessage, pFieldDescriptor, nValue);
		}
		break;

		case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
		{
			double nValue = 0.0;
			if (!base::string_util::convert_to_value(szValue, nValue))
				return false;

			pReflection->SetDouble(pMessage, pFieldDescriptor, nValue);
		}
		break;

		case google::protobuf::FieldDescriptor::TYPE_FLOAT:
		{
			float nValue = 0.0f;
			if (!base::string_util::convert_to_value(szValue, nValue))
				return false;

			pReflection->SetDouble(pMessage, pFieldDescriptor, nValue);
		}
		break;

		case google::protobuf::FieldDescriptor::TYPE_STRING:
		case google::protobuf::FieldDescriptor::TYPE_BYTES:
		{
			pReflection->SetString(pMessage, pFieldDescriptor, szValue);
		}
		break;

		default:
			DebugAstEx(false, false);
		}

		return true;
	}

	bool setFieldValue(google::protobuf::Message* pMessage, const google::protobuf::Reflection* pReflection, const google::protobuf::FieldDescriptor* pFieldDescriptor, const std::string& szValue)
	{
		DebugAstEx(pMessage != nullptr, false);
		DebugAstEx(pReflection != nullptr, false);
		DebugAstEx(pFieldDescriptor != nullptr, false);

		if (pFieldDescriptor->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
		{
			ESerializeType eSerializeType = (ESerializeType)pFieldDescriptor->options().GetExtension(serialize_type);

			switch (eSerializeType)
			{
			case eST_Protobuf_Bin:
			{
				google::protobuf::Message* pSubMessage = pReflection->MutableMessage(pMessage, pFieldDescriptor);
				DebugAstEx(pSubMessage != nullptr, false);
				if (!szValue.empty())
				{
					DebugAstEx(pSubMessage->ParseFromArray(szValue.c_str(), (int32_t)szValue.size()), false);
				}
			}
			break;

			case eST_Json:
			{
				google::protobuf::Message* pSubMessage = pReflection->MutableMessage(pMessage, pFieldDescriptor);
				DebugAstEx(pSubMessage != nullptr, false);
				if (!szValue.empty())
				{
					DebugAstEx(google::protobuf::util::JsonStringToMessage(szValue, pSubMessage).ok(), false);
				}
			}
			break;

			default:
				DebugAstEx(false, false);
			}
		}
		else
		{
			DebugAstEx(setBasicTypeFieldValue(pMessage, pReflection, pFieldDescriptor, szValue), false);
		}

		return true;
	}
}

namespace base
{
	bool importProtobuf(const std::string& szDir, const std::vector<std::string>& vecFile)
	{
		return CMessageFactory::Inst()->init(szDir, vecFile);
	}

	std::string getMessageNameByTableName(const std::string& szTableName)
	{
		return std::string(_DB_NAMESPACE) + szTableName;
	}

	bool getTableNameByMessageName(const std::string& szMessageName, std::string& szTableName)
	{
		static size_t nPrefixLen = strlen(_DB_NAMESPACE);

		size_t pos = szMessageName.find(_DB_NAMESPACE);
		if (pos == std::string::npos)
			return false;

		szTableName = szMessageName.substr(pos + nPrefixLen);
		return true;
	}

	bool getMessageFieldInfos(const google::protobuf::Message* pMessage, std::vector<SFieldInfo>& vecFieldInfo)
	{
		DebugAstEx(pMessage != nullptr, false);

		const google::protobuf::Descriptor* pDescriptor = pMessage->GetDescriptor();
		DebugAstEx(pDescriptor != nullptr, false);

		const google::protobuf::Reflection* pReflection = pMessage->GetReflection();
		DebugAstEx(pReflection != nullptr, false);

		for (int32_t i = 0; i < pDescriptor->field_count(); ++i)
		{
			const google::protobuf::FieldDescriptor* pFieldDescriptor = pDescriptor->field(i);
			DebugAstEx(pFieldDescriptor != nullptr, false);

			if (!pReflection->HasField(*pMessage, pFieldDescriptor))
				continue;

			if (pFieldDescriptor->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
			{
				ESerializeType eSerializeType = (ESerializeType)pFieldDescriptor->options().GetExtension(serialize_type);

#ifdef GetMessage
#undef GetMessage
#endif
				std::string szBuf;
				switch (eSerializeType)
				{
				case eST_Protobuf_Bin:
				{
					const google::protobuf::Message& subMessage = pReflection->GetMessage(*pMessage, pFieldDescriptor);
					szBuf.resize(subMessage.ByteSize());
					if (subMessage.ByteSize() > 0 && !subMessage.SerializeToArray(&szBuf[0], (int32_t)szBuf.size()))
					{
						PrintWarning("SerializeToArray fail.[{}:{}]", pMessage->GetTypeName(), pFieldDescriptor->type_name());
						return false;
					}
				}
				break;

				case eST_Json:
				{
					const google::protobuf::Message& subMessage = pReflection->GetMessage(*pMessage, pFieldDescriptor);

					google::protobuf::util::JsonOptions sOptions;
					sOptions.preserve_proto_field_names = true;
					if (!google::protobuf::util::MessageToJsonString(subMessage, &szBuf, sOptions).ok())
					{
						PrintWarning("MessageToJsonString fail.[{}:{}]", pMessage->GetTypeName(), pFieldDescriptor->type_name());
						return false;
					}
				}
				break;

				default:
				{
					PrintWarning("Message[{}] field{}] hasn't serialize type.", pMessage->GetTypeName(), pFieldDescriptor->name());
					return false;
				}
				}

				SFieldInfo sFieldInfo;
				sFieldInfo.szName = pFieldDescriptor->name();
				sFieldInfo.szValue = std::move(szBuf);
				sFieldInfo.bStr = true;

				vecFieldInfo.push_back(sFieldInfo);
			}
			else
			{
				std::string szValue;
				if (!getBasicTypeFieldValue(pMessage, pFieldDescriptor, pReflection, szValue))
				{
					PrintWarning("ERROR: message[{}] can't get field[{}] value", pMessage->GetTypeName(), pFieldDescriptor->name());
					return false;
				}

				SFieldInfo sFieldInfo;
				sFieldInfo.szName = pFieldDescriptor->name();
				sFieldInfo.szValue = std::move(szValue);
				sFieldInfo.bStr = (pFieldDescriptor->type() == google::protobuf::FieldDescriptor::TYPE_STRING || pFieldDescriptor->type() == google::protobuf::FieldDescriptor::TYPE_BYTES);

				vecFieldInfo.push_back(sFieldInfo);
			}
		}

		return true;
	}

	std::string getPrimaryName(const google::protobuf::Message* pMessage)
	{
		DebugAstEx(pMessage != nullptr, "");

		const google::protobuf::Reflection* pReflection = pMessage->GetReflection();
		DebugAstEx(pReflection != nullptr, "");

		const google::protobuf::Descriptor* pDescriptor = pMessage->GetDescriptor();
		DebugAstEx(pDescriptor != nullptr, "");

		return pDescriptor->options().GetExtension(primary_key);
	}

	bool getPrimaryValue(const google::protobuf::Message* pMessage, uint64_t& nValue)
	{
		if (pMessage == nullptr)
			return false;

		const google::protobuf::Reflection* pReflection = pMessage->GetReflection();
		if (pReflection == nullptr)
			return false;

		const google::protobuf::Descriptor* pDescriptor = pMessage->GetDescriptor();
		if (pDescriptor == nullptr)
			return false;

		std::string szPrimaryFieldName = pDescriptor->options().GetExtension(primary_key);
		if (szPrimaryFieldName.empty())
			return false;

		const google::protobuf::FieldDescriptor* pFieldDescriptor = pMessage->GetDescriptor()->FindFieldByName(szPrimaryFieldName);
		if (pFieldDescriptor == nullptr)
			return false;

		switch (pFieldDescriptor->type())
		{
		case google::protobuf::FieldDescriptor::TYPE_INT32:
		case google::protobuf::FieldDescriptor::TYPE_SINT32:
		case google::protobuf::FieldDescriptor::TYPE_SFIXED32:
			nValue = pReflection->GetInt32(*pMessage, pFieldDescriptor);
			break;

		case google::protobuf::FieldDescriptor::TYPE_UINT32:
		case google::protobuf::FieldDescriptor::TYPE_FIXED32:
			nValue = pReflection->GetUInt32(*pMessage, pFieldDescriptor);
			break;

		case google::protobuf::FieldDescriptor::TYPE_INT64:
		case google::protobuf::FieldDescriptor::TYPE_SINT64:
		case google::protobuf::FieldDescriptor::TYPE_SFIXED64:
			nValue = pReflection->GetInt64(*pMessage, pFieldDescriptor);
			break;

		case google::protobuf::FieldDescriptor::TYPE_UINT64:
		case google::protobuf::FieldDescriptor::TYPE_FIXED64:
			nValue = pReflection->GetUInt64(*pMessage, pFieldDescriptor);
			break;

		default:
			return false;
		}

		return true;
	}

	bool fillRepeatMessage(CDbRecordset* pDbRecordset, google::protobuf::Message* pMessage)
	{
		DebugAstEx(pDbRecordset != nullptr, false);
		DebugAstEx(pMessage != nullptr, false);

		std::string szMessageName = pMessage->GetTypeName();
		szMessageName = szMessageName.substr(0, szMessageName.size() - _QUERY_SET_SUFFIX_LEN);

		const google::protobuf::Reflection* pMainReflection = pMessage->GetReflection();
		if (pMainReflection == nullptr)
		{
			PrintWarning("message[{}] can't get reflection.", pMessage->GetTypeName());
			return false;
		}

		const google::protobuf::Descriptor* pMainDescriptor = pMessage->GetDescriptor();
		if (pMainDescriptor == nullptr)
		{
			PrintWarning("message[{}] can't get descriptor.", pMessage->GetTypeName());
			return false;
		}

		if (pMainDescriptor->field_count() != 1)
		{
			PrintWarning("message[{}] field count isn't one.", pMessage->GetTypeName());
			return false;
		}

		const google::protobuf::FieldDescriptor* pMainFieldDescriptor = pMainDescriptor->field(0);
		if (pMainFieldDescriptor == nullptr)
		{
			PrintWarning("message[{}] can't get field descriptor.", pMessage->GetTypeName());
			return false;
		}

		if (pMainFieldDescriptor->label() != google::protobuf::FieldDescriptor::LABEL_REPEATED ||
			pMainFieldDescriptor->type() != google::protobuf::FieldDescriptor::TYPE_MESSAGE)
		{
			PrintWarning("message[{}] main field prototy is wrong.", pMessage->GetTypeName());
			return false;
		}

		for (uint64_t i = 0; i < pDbRecordset->getRowCount(); ++i)
		{
			pDbRecordset->fatchNextRow();
			google::protobuf::Message* pSubMessage = pMainReflection->AddMessage(pMessage, pMainFieldDescriptor);
			if (pSubMessage == nullptr || pSubMessage->GetTypeName() != szMessageName)
			{
				PrintWarning("message[{}] AddMessage failed.", pMessage->GetTypeName());
				return false;
			}

			const google::protobuf::Reflection* pReflection = pSubMessage->GetReflection();
			if (pReflection == nullptr)
			{
				PrintWarning("message[{}] can't get reflection.", pSubMessage->GetTypeName());
				return false;
			}

			const google::protobuf::Descriptor* pDescriptor = pSubMessage->GetDescriptor();
			if (pDescriptor == nullptr)
			{
				PrintWarning("message[{}] can't get descriptor.", pSubMessage->GetTypeName());
				return false;
			}

			for (uint32_t i = 0; i < pDbRecordset->getFieldCount(); ++i)
			{
				std::string szFieldName = pDbRecordset->getFieldName(i);
				std::string szValue = pDbRecordset->getData(i);

				const google::protobuf::FieldDescriptor* pFieldDescriptor = pDescriptor->FindFieldByName(szFieldName);
				if (pFieldDescriptor == nullptr)
				{
					PrintWarning("field[{}.{}] descriptor is nullptr.", pSubMessage->GetTypeName(), szFieldName);
					return false;
				}

				if (!setFieldValue(pSubMessage, pReflection, pFieldDescriptor, szValue))
				{
					PrintWarning("setFieldValue[{}.{}] failed.", pSubMessage->GetTypeName(), szFieldName);
					return false;
				}
			}
		}

		return true;
	}

	bool fillNormalMessage(CDbRecordset* pDbRecordset, google::protobuf::Message* pMessage)
	{
		DebugAstEx(pDbRecordset != nullptr, false);
		DebugAstEx(pDbRecordset->getRowCount() <= 1, false);

		if (pDbRecordset->getRowCount() == 0)
			return true;

		const google::protobuf::Reflection* pReflection = pMessage->GetReflection();
		if (pReflection == nullptr)
		{
			PrintWarning("message[{}] can't get reflection.", pMessage->GetTypeName());
			return false;
		}

		const google::protobuf::Descriptor* pDescriptor = pMessage->GetDescriptor();
		if (pDescriptor == nullptr)
		{
			PrintWarning("message[{}] can't get descriptor.", pMessage->GetTypeName());
			return false;
		}

		pDbRecordset->fatchNextRow();

		for (uint32_t i = 0; i < pDbRecordset->getFieldCount(); ++i)
		{
			std::string szFieldName = pDbRecordset->getFieldName(i);
			std::string szValue = pDbRecordset->getData(i);

			const google::protobuf::FieldDescriptor* pFieldDescriptor = pDescriptor->FindFieldByName(szFieldName);
			if (pFieldDescriptor == nullptr)
			{
				PrintWarning("field[{}.{}] descriptor is nullptr.", pMessage->GetTypeName(), szFieldName);
				return false;
			}

			if (!setFieldValue(pMessage, pReflection, pFieldDescriptor, szValue))
			{
				PrintWarning("setFieldValue[{}.{}] failed.", pMessage->GetTypeName(), szFieldName);
				return false;
			}
		}

		return true;
	}

	google::protobuf::Message* createMessage(const std::string& szName)
	{
		return CMessageFactory::Inst()->createMessage(szName);
	}
}