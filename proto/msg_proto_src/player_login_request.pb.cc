// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: player_login_request.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "player_login_request.pb.h"

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
class player_login_requestDefaultTypeInternal : public ::google::protobuf::internal::ExplicitlyConstructed<player_login_request> {
} _player_login_request_default_instance_;

namespace protobuf_player_5flogin_5frequest_2eproto {


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
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(player_login_request, _has_bits_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(player_login_request, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(player_login_request, account_name_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(player_login_request, server_id_),
  0,
  1,
};

static const ::google::protobuf::internal::MigrationSchema schemas[] = {
  { 0, 7, sizeof(player_login_request)},
};

static ::google::protobuf::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::google::protobuf::Message*>(&_player_login_request_default_instance_),
};

namespace {

void protobuf_AssignDescriptors() {
  AddDescriptors();
  ::google::protobuf::MessageFactory* factory = NULL;
  AssignDescriptors(
      "player_login_request.proto", schemas, file_default_instances, TableStruct::offsets, factory,
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
  _player_login_request_default_instance_.Shutdown();
  delete file_level_metadata[0].reflection;
}

void TableStruct::InitDefaultsImpl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::internal::InitProtobufDefaults();
  _player_login_request_default_instance_.DefaultConstruct();
}

void InitDefaults() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &TableStruct::InitDefaultsImpl);
}
void AddDescriptorsImpl() {
  InitDefaults();
  static const char descriptor[] = {
      "\n\032player_login_request.proto\"\?\n\024player_l"
      "ogin_request\022\024\n\014account_name\030\001 \002(\t\022\021\n\tse"
      "rver_id\030\002 \002(\r"
  };
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
      descriptor, 93);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "player_login_request.proto", &protobuf_RegisterTypes);
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

}  // namespace protobuf_player_5flogin_5frequest_2eproto


// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int player_login_request::kAccountNameFieldNumber;
const int player_login_request::kServerIdFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

player_login_request::player_login_request()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  if (GOOGLE_PREDICT_TRUE(this != internal_default_instance())) {
    protobuf_player_5flogin_5frequest_2eproto::InitDefaults();
  }
  SharedCtor();
  // @@protoc_insertion_point(constructor:player_login_request)
}
player_login_request::player_login_request(const player_login_request& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      _has_bits_(from._has_bits_),
      _cached_size_(0) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  account_name_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (from.has_account_name()) {
    account_name_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.account_name_);
  }
  server_id_ = from.server_id_;
  // @@protoc_insertion_point(copy_constructor:player_login_request)
}

void player_login_request::SharedCtor() {
  _cached_size_ = 0;
  account_name_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  server_id_ = 0u;
}

player_login_request::~player_login_request() {
  // @@protoc_insertion_point(destructor:player_login_request)
  SharedDtor();
}

void player_login_request::SharedDtor() {
  account_name_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}

void player_login_request::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* player_login_request::descriptor() {
  protobuf_player_5flogin_5frequest_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_player_5flogin_5frequest_2eproto::file_level_metadata[kIndexInFileMessages].descriptor;
}

const player_login_request& player_login_request::default_instance() {
  protobuf_player_5flogin_5frequest_2eproto::InitDefaults();
  return *internal_default_instance();
}

player_login_request* player_login_request::New(::google::protobuf::Arena* arena) const {
  player_login_request* n = new player_login_request;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void player_login_request::Clear() {
// @@protoc_insertion_point(message_clear_start:player_login_request)
  if (has_account_name()) {
    GOOGLE_DCHECK(!account_name_.IsDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited()));
    (*account_name_.UnsafeRawStringPointer())->clear();
  }
  server_id_ = 0u;
  _has_bits_.Clear();
  _internal_metadata_.Clear();
}

bool player_login_request::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:player_login_request)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required string account_name = 1;
      case 1: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(10u)) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_account_name()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->account_name().data(), this->account_name().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "player_login_request.account_name");
        } else {
          goto handle_unusual;
        }
        break;
      }

      // required uint32 server_id = 2;
      case 2: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(16u)) {
          set_has_server_id();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &server_id_)));
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
  // @@protoc_insertion_point(parse_success:player_login_request)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:player_login_request)
  return false;
#undef DO_
}

void player_login_request::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:player_login_request)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // required string account_name = 1;
  if (cached_has_bits & 0x00000001u) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->account_name().data(), this->account_name().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "player_login_request.account_name");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      1, this->account_name(), output);
  }

  // required uint32 server_id = 2;
  if (cached_has_bits & 0x00000002u) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(2, this->server_id(), output);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:player_login_request)
}

::google::protobuf::uint8* player_login_request::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  (void)deterministic; // Unused
  // @@protoc_insertion_point(serialize_to_array_start:player_login_request)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // required string account_name = 1;
  if (cached_has_bits & 0x00000001u) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->account_name().data(), this->account_name().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "player_login_request.account_name");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        1, this->account_name(), target);
  }

  // required uint32 server_id = 2;
  if (cached_has_bits & 0x00000002u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(2, this->server_id(), target);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:player_login_request)
  return target;
}

size_t player_login_request::RequiredFieldsByteSizeFallback() const {
// @@protoc_insertion_point(required_fields_byte_size_fallback_start:player_login_request)
  size_t total_size = 0;

  if (has_account_name()) {
    // required string account_name = 1;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->account_name());
  }

  if (has_server_id()) {
    // required uint32 server_id = 2;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->server_id());
  }

  return total_size;
}
size_t player_login_request::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:player_login_request)
  size_t total_size = 0;

  if (_internal_metadata_.have_unknown_fields()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  if (((_has_bits_[0] & 0x00000003) ^ 0x00000003) == 0) {  // All required fields are present.
    // required string account_name = 1;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::StringSize(
        this->account_name());

    // required uint32 server_id = 2;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->server_id());

  } else {
    total_size += RequiredFieldsByteSizeFallback();
  }
  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = cached_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void player_login_request::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:player_login_request)
  GOOGLE_DCHECK_NE(&from, this);
  const player_login_request* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const player_login_request>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:player_login_request)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:player_login_request)
    MergeFrom(*source);
  }
}

void player_login_request::MergeFrom(const player_login_request& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:player_login_request)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = from._has_bits_[0];
  if (cached_has_bits & 3u) {
    if (cached_has_bits & 0x00000001u) {
      set_has_account_name();
      account_name_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.account_name_);
    }
    if (cached_has_bits & 0x00000002u) {
      server_id_ = from.server_id_;
    }
    _has_bits_[0] |= cached_has_bits;
  }
}

void player_login_request::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:player_login_request)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void player_login_request::CopyFrom(const player_login_request& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:player_login_request)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool player_login_request::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000003) != 0x00000003) return false;
  return true;
}

void player_login_request::Swap(player_login_request* other) {
  if (other == this) return;
  InternalSwap(other);
}
void player_login_request::InternalSwap(player_login_request* other) {
  account_name_.Swap(&other->account_name_);
  std::swap(server_id_, other->server_id_);
  std::swap(_has_bits_[0], other->_has_bits_[0]);
  _internal_metadata_.Swap(&other->_internal_metadata_);
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata player_login_request::GetMetadata() const {
  protobuf_player_5flogin_5frequest_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_player_5flogin_5frequest_2eproto::file_level_metadata[kIndexInFileMessages];
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// player_login_request

// required string account_name = 1;
bool player_login_request::has_account_name() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
void player_login_request::set_has_account_name() {
  _has_bits_[0] |= 0x00000001u;
}
void player_login_request::clear_has_account_name() {
  _has_bits_[0] &= ~0x00000001u;
}
void player_login_request::clear_account_name() {
  account_name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_account_name();
}
const ::std::string& player_login_request::account_name() const {
  // @@protoc_insertion_point(field_get:player_login_request.account_name)
  return account_name_.GetNoArena();
}
void player_login_request::set_account_name(const ::std::string& value) {
  set_has_account_name();
  account_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:player_login_request.account_name)
}
#if LANG_CXX11
void player_login_request::set_account_name(::std::string&& value) {
  set_has_account_name();
  account_name_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:player_login_request.account_name)
}
#endif
void player_login_request::set_account_name(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  set_has_account_name();
  account_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:player_login_request.account_name)
}
void player_login_request::set_account_name(const char* value, size_t size) {
  set_has_account_name();
  account_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:player_login_request.account_name)
}
::std::string* player_login_request::mutable_account_name() {
  set_has_account_name();
  // @@protoc_insertion_point(field_mutable:player_login_request.account_name)
  return account_name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
::std::string* player_login_request::release_account_name() {
  // @@protoc_insertion_point(field_release:player_login_request.account_name)
  clear_has_account_name();
  return account_name_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
void player_login_request::set_allocated_account_name(::std::string* account_name) {
  if (account_name != NULL) {
    set_has_account_name();
  } else {
    clear_has_account_name();
  }
  account_name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), account_name);
  // @@protoc_insertion_point(field_set_allocated:player_login_request.account_name)
}

// required uint32 server_id = 2;
bool player_login_request::has_server_id() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
void player_login_request::set_has_server_id() {
  _has_bits_[0] |= 0x00000002u;
}
void player_login_request::clear_has_server_id() {
  _has_bits_[0] &= ~0x00000002u;
}
void player_login_request::clear_server_id() {
  server_id_ = 0u;
  clear_has_server_id();
}
::google::protobuf::uint32 player_login_request::server_id() const {
  // @@protoc_insertion_point(field_get:player_login_request.server_id)
  return server_id_;
}
void player_login_request::set_server_id(::google::protobuf::uint32 value) {
  set_has_server_id();
  server_id_ = value;
  // @@protoc_insertion_point(field_set:player_login_request.server_id)
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)

// @@protoc_insertion_point(global_scope)