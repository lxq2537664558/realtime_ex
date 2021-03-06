// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: d2g_player_token_notify.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "d2g_player_token_notify.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
class d2g_player_token_notifyDefaultTypeInternal : public ::google::protobuf::internal::ExplicitlyConstructed<d2g_player_token_notify> {
} _d2g_player_token_notify_default_instance_;

namespace protobuf_d2g_5fplayer_5ftoken_5fnotify_2eproto {


namespace {

::google::protobuf::Metadata file_level_metadata[1];

}  // namespace

PROTOBUF_CONSTEXPR_VAR ::google::protobuf::internal::ParseTableField
    const TableStruct::entries[] = {
  {0, 0, 0, ::google::protobuf::internal::kInvalidMask, 0, 0},
};

PROTOBUF_CONSTEXPR_VAR ::google::protobuf::internal::AuxillaryParseTableField
    const TableStruct::aux[] = {
  ::google::protobuf::internal::AuxillaryParseTableField(),
};
PROTOBUF_CONSTEXPR_VAR ::google::protobuf::internal::ParseTable const
    TableStruct::schema[] = {
  { NULL, NULL, 0, -1, -1, false },
};

const ::google::protobuf::uint32 TableStruct::offsets[] = {
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(d2g_player_token_notify, _has_bits_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(d2g_player_token_notify, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(d2g_player_token_notify, player_id_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(d2g_player_token_notify, gas_id_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(d2g_player_token_notify, token_),
  1,
  2,
  0,
};

static const ::google::protobuf::internal::MigrationSchema schemas[] = {
  { 0, 8, sizeof(d2g_player_token_notify)},
};

static ::google::protobuf::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::google::protobuf::Message*>(&_d2g_player_token_notify_default_instance_),
};

namespace {

void protobuf_AssignDescriptors() {
  AddDescriptors();
  ::google::protobuf::MessageFactory* factory = NULL;
  AssignDescriptors(
      "d2g_player_token_notify.proto", schemas, file_default_instances, TableStruct::offsets, factory,
      file_level_metadata, NULL, NULL);
}

void protobuf_AssignDescriptorsOnce() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &protobuf_AssignDescriptors);
}

void protobuf_RegisterTypes(const ::std::string&) GOOGLE_ATTRIBUTE_COLD;
void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::internal::RegisterAllTypes(file_level_metadata, 1);
}

}  // namespace

void TableStruct::Shutdown() {
  _d2g_player_token_notify_default_instance_.Shutdown();
  delete file_level_metadata[0].reflection;
}

void TableStruct::InitDefaultsImpl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::internal::InitProtobufDefaults();
  _d2g_player_token_notify_default_instance_.DefaultConstruct();
}

void InitDefaults() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &TableStruct::InitDefaultsImpl);
}
void AddDescriptorsImpl() {
  InitDefaults();
  static const char descriptor[] = {
      "\n\035d2g_player_token_notify.proto\"K\n\027d2g_p"
      "layer_token_notify\022\021\n\tplayer_id\030\001 \002(\004\022\016\n"
      "\006gas_id\030\002 \002(\r\022\r\n\005token\030\003 \002(\t"
  };
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
      descriptor, 108);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "d2g_player_token_notify.proto", &protobuf_RegisterTypes);
  ::google::protobuf::internal::OnShutdown(&TableStruct::Shutdown);
}

void AddDescriptors() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &AddDescriptorsImpl);
}
// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer {
  StaticDescriptorInitializer() {
    AddDescriptors();
  }
} static_descriptor_initializer;

}  // namespace protobuf_d2g_5fplayer_5ftoken_5fnotify_2eproto


// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int d2g_player_token_notify::kPlayerIdFieldNumber;
const int d2g_player_token_notify::kGasIdFieldNumber;
const int d2g_player_token_notify::kTokenFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

d2g_player_token_notify::d2g_player_token_notify()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  if (GOOGLE_PREDICT_TRUE(this != internal_default_instance())) {
    protobuf_d2g_5fplayer_5ftoken_5fnotify_2eproto::InitDefaults();
  }
  SharedCtor();
  // @@protoc_insertion_point(constructor:d2g_player_token_notify)
}
d2g_player_token_notify::d2g_player_token_notify(const d2g_player_token_notify& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      _has_bits_(from._has_bits_),
      _cached_size_(0) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  token_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.has_token()) {
    token_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.token_);
  }
  ::memcpy(&player_id_, &from.player_id_,
    reinterpret_cast<char*>(&gas_id_) -
    reinterpret_cast<char*>(&player_id_) + sizeof(gas_id_));
  // @@protoc_insertion_point(copy_constructor:d2g_player_token_notify)
}

void d2g_player_token_notify::SharedCtor() {
  _cached_size_ = 0;
  token_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ::memset(&player_id_, 0, reinterpret_cast<char*>(&gas_id_) -
    reinterpret_cast<char*>(&player_id_) + sizeof(gas_id_));
}

d2g_player_token_notify::~d2g_player_token_notify() {
  // @@protoc_insertion_point(destructor:d2g_player_token_notify)
  SharedDtor();
}

void d2g_player_token_notify::SharedDtor() {
  token_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

void d2g_player_token_notify::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* d2g_player_token_notify::descriptor() {
  protobuf_d2g_5fplayer_5ftoken_5fnotify_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_d2g_5fplayer_5ftoken_5fnotify_2eproto::file_level_metadata[kIndexInFileMessages].descriptor;
}

const d2g_player_token_notify& d2g_player_token_notify::default_instance() {
  protobuf_d2g_5fplayer_5ftoken_5fnotify_2eproto::InitDefaults();
  return *internal_default_instance();
}

d2g_player_token_notify* d2g_player_token_notify::New(::google::protobuf::Arena* arena) const {
  d2g_player_token_notify* n = new d2g_player_token_notify;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void d2g_player_token_notify::Clear() {
// @@protoc_insertion_point(message_clear_start:d2g_player_token_notify)
  if (has_token()) {
    GOOGLE_DCHECK(!token_.IsDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited()));
    (*token_.UnsafeRawStringPointer())->clear();
  }
  if (_has_bits_[0 / 32] & 6u) {
    ::memset(&player_id_, 0, reinterpret_cast<char*>(&gas_id_) -
      reinterpret_cast<char*>(&player_id_) + sizeof(gas_id_));
  }
  _has_bits_.Clear();
  _internal_metadata_.Clear();
}

bool d2g_player_token_notify::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:d2g_player_token_notify)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required uint64 player_id = 1;
      case 1: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(8u)) {
          set_has_player_id();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint64, ::google::protobuf::internal::WireFormatLite::TYPE_UINT64>(
                 input, &player_id_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // required uint32 gas_id = 2;
      case 2: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(16u)) {
          set_has_gas_id();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &gas_id_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // required string token = 3;
      case 3: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(26u)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_token()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->token().data(), this->token().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "d2g_player_token_notify.token");
        } else {
          goto handle_unusual;
        }
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:d2g_player_token_notify)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:d2g_player_token_notify)
  return false;
#undef DO_
}

void d2g_player_token_notify::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:d2g_player_token_notify)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // required uint64 player_id = 1;
  if (cached_has_bits & 0x00000002u) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt64(1, this->player_id(), output);
  }

  // required uint32 gas_id = 2;
  if (cached_has_bits & 0x00000004u) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(2, this->gas_id(), output);
  }

  // required string token = 3;
  if (cached_has_bits & 0x00000001u) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->token().data(), this->token().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "d2g_player_token_notify.token");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      3, this->token(), output);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:d2g_player_token_notify)
}

::google::protobuf::uint8* d2g_player_token_notify::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  (void)deterministic; // Unused
  // @@protoc_insertion_point(serialize_to_array_start:d2g_player_token_notify)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // required uint64 player_id = 1;
  if (cached_has_bits & 0x00000002u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt64ToArray(1, this->player_id(), target);
  }

  // required uint32 gas_id = 2;
  if (cached_has_bits & 0x00000004u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(2, this->gas_id(), target);
  }

  // required string token = 3;
  if (cached_has_bits & 0x00000001u) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->token().data(), this->token().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "d2g_player_token_notify.token");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        3, this->token(), target);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:d2g_player_token_notify)
  return target;
}

size_t d2g_player_token_notify::RequiredFieldsByteSizeFallback() const {
// @@protoc_insertion_point(required_fields_byte_size_fallback_start:d2g_player_token_notify)
  size_t total_size = 0;

  if (has_token()) {
    // required string token = 3;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->token());
  }

  if (has_player_id()) {
    // required uint64 player_id = 1;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt64Size(
        this->player_id());
  }

  if (has_gas_id()) {
    // required uint32 gas_id = 2;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->gas_id());
  }

  return total_size;
}
size_t d2g_player_token_notify::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:d2g_player_token_notify)
  size_t total_size = 0;

  if (_internal_metadata_.have_unknown_fields()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  if (((_has_bits_[0] & 0x00000007) ^ 0x00000007) == 0) {  // All required fields are present.
    // required string token = 3;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->token());

    // required uint64 player_id = 1;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt64Size(
        this->player_id());

    // required uint32 gas_id = 2;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->gas_id());

  } else {
    total_size += RequiredFieldsByteSizeFallback();
  }
  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = cached_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void d2g_player_token_notify::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:d2g_player_token_notify)
  GOOGLE_DCHECK_NE(&from, this);
  const d2g_player_token_notify* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const d2g_player_token_notify>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:d2g_player_token_notify)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:d2g_player_token_notify)
    MergeFrom(*source);
  }
}

void d2g_player_token_notify::MergeFrom(const d2g_player_token_notify& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:d2g_player_token_notify)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = from._has_bits_[0];
  if (cached_has_bits & 7u) {
    if (cached_has_bits & 0x00000001u) {
      set_has_token();
      token_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.token_);
    }
    if (cached_has_bits & 0x00000002u) {
      player_id_ = from.player_id_;
    }
    if (cached_has_bits & 0x00000004u) {
      gas_id_ = from.gas_id_;
    }
    _has_bits_[0] |= cached_has_bits;
  }
}

void d2g_player_token_notify::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:d2g_player_token_notify)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void d2g_player_token_notify::CopyFrom(const d2g_player_token_notify& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:d2g_player_token_notify)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool d2g_player_token_notify::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000007) != 0x00000007) return false;
  return true;
}

void d2g_player_token_notify::Swap(d2g_player_token_notify* other) {
  if (other == this) return;
  InternalSwap(other);
}
void d2g_player_token_notify::InternalSwap(d2g_player_token_notify* other) {
  token_.Swap(&other->token_);
  std::swap(player_id_, other->player_id_);
  std::swap(gas_id_, other->gas_id_);
  std::swap(_has_bits_[0], other->_has_bits_[0]);
  _internal_metadata_.Swap(&other->_internal_metadata_);
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata d2g_player_token_notify::GetMetadata() const {
  protobuf_d2g_5fplayer_5ftoken_5fnotify_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_d2g_5fplayer_5ftoken_5fnotify_2eproto::file_level_metadata[kIndexInFileMessages];
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// d2g_player_token_notify

// required uint64 player_id = 1;
bool d2g_player_token_notify::has_player_id() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
void d2g_player_token_notify::set_has_player_id() {
  _has_bits_[0] |= 0x00000002u;
}
void d2g_player_token_notify::clear_has_player_id() {
  _has_bits_[0] &= ~0x00000002u;
}
void d2g_player_token_notify::clear_player_id() {
  player_id_ = GOOGLE_ULONGLONG(0);
  clear_has_player_id();
}
::google::protobuf::uint64 d2g_player_token_notify::player_id() const {
  // @@protoc_insertion_point(field_get:d2g_player_token_notify.player_id)
  return player_id_;
}
void d2g_player_token_notify::set_player_id(::google::protobuf::uint64 value) {
  set_has_player_id();
  player_id_ = value;
  // @@protoc_insertion_point(field_set:d2g_player_token_notify.player_id)
}

// required uint32 gas_id = 2;
bool d2g_player_token_notify::has_gas_id() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
void d2g_player_token_notify::set_has_gas_id() {
  _has_bits_[0] |= 0x00000004u;
}
void d2g_player_token_notify::clear_has_gas_id() {
  _has_bits_[0] &= ~0x00000004u;
}
void d2g_player_token_notify::clear_gas_id() {
  gas_id_ = 0u;
  clear_has_gas_id();
}
::google::protobuf::uint32 d2g_player_token_notify::gas_id() const {
  // @@protoc_insertion_point(field_get:d2g_player_token_notify.gas_id)
  return gas_id_;
}
void d2g_player_token_notify::set_gas_id(::google::protobuf::uint32 value) {
  set_has_gas_id();
  gas_id_ = value;
  // @@protoc_insertion_point(field_set:d2g_player_token_notify.gas_id)
}

// required string token = 3;
bool d2g_player_token_notify::has_token() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
void d2g_player_token_notify::set_has_token() {
  _has_bits_[0] |= 0x00000001u;
}
void d2g_player_token_notify::clear_has_token() {
  _has_bits_[0] &= ~0x00000001u;
}
void d2g_player_token_notify::clear_token() {
  token_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_token();
}
const ::std::string& d2g_player_token_notify::token() const {
  // @@protoc_insertion_point(field_get:d2g_player_token_notify.token)
  return token_.GetNoArena();
}
void d2g_player_token_notify::set_token(const ::std::string& value) {
  set_has_token();
  token_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:d2g_player_token_notify.token)
}
#if LANG_CXX11
void d2g_player_token_notify::set_token(::std::string&& value) {
  set_has_token();
  token_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:d2g_player_token_notify.token)
}
#endif
void d2g_player_token_notify::set_token(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  set_has_token();
  token_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:d2g_player_token_notify.token)
}
void d2g_player_token_notify::set_token(const char* value, size_t size) {
  set_has_token();
  token_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:d2g_player_token_notify.token)
}
::std::string* d2g_player_token_notify::mutable_token() {
  set_has_token();
  // @@protoc_insertion_point(field_mutable:d2g_player_token_notify.token)
  return token_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
::std::string* d2g_player_token_notify::release_token() {
  // @@protoc_insertion_point(field_release:d2g_player_token_notify.token)
  clear_has_token();
  return token_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
void d2g_player_token_notify::set_allocated_token(::std::string* token) {
  if (token != NULL) {
    set_has_token();
  } else {
    clear_has_token();
  }
  token_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), token);
  // @@protoc_insertion_point(field_set_allocated:d2g_player_token_notify.token)
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)

// @@protoc_insertion_point(global_scope)
