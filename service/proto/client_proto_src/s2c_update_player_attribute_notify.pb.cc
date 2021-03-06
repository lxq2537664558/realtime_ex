// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: s2c_update_player_attribute_notify.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "s2c_update_player_attribute_notify.pb.h"

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
class s2c_update_player_attribute_notifyDefaultTypeInternal : public ::google::protobuf::internal::ExplicitlyConstructed<s2c_update_player_attribute_notify> {
} _s2c_update_player_attribute_notify_default_instance_;

namespace protobuf_s2c_5fupdate_5fplayer_5fattribute_5fnotify_2eproto {


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
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(s2c_update_player_attribute_notify, _has_bits_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(s2c_update_player_attribute_notify, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(s2c_update_player_attribute_notify, type_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(s2c_update_player_attribute_notify, value_),
  1,
  0,
};

static const ::google::protobuf::internal::MigrationSchema schemas[] = {
  { 0, 7, sizeof(s2c_update_player_attribute_notify)},
};

static ::google::protobuf::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::google::protobuf::Message*>(&_s2c_update_player_attribute_notify_default_instance_),
};

namespace {

void protobuf_AssignDescriptors() {
  AddDescriptors();
  ::google::protobuf::MessageFactory* factory = NULL;
  AssignDescriptors(
      "s2c_update_player_attribute_notify.proto", schemas, file_default_instances, TableStruct::offsets, factory,
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
  _s2c_update_player_attribute_notify_default_instance_.Shutdown();
  delete file_level_metadata[0].reflection;
}

void TableStruct::InitDefaultsImpl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::internal::InitProtobufDefaults();
  _s2c_update_player_attribute_notify_default_instance_.DefaultConstruct();
}

void InitDefaults() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &TableStruct::InitDefaultsImpl);
}
void AddDescriptorsImpl() {
  InitDefaults();
  static const char descriptor[] = {
      "\n(s2c_update_player_attribute_notify.pro"
      "to\"A\n\"s2c_update_player_attribute_notify"
      "\022\014\n\004type\030\001 \002(\r\022\r\n\005value\030\002 \002(\004"
  };
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
      descriptor, 109);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "s2c_update_player_attribute_notify.proto", &protobuf_RegisterTypes);
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

}  // namespace protobuf_s2c_5fupdate_5fplayer_5fattribute_5fnotify_2eproto


// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int s2c_update_player_attribute_notify::kTypeFieldNumber;
const int s2c_update_player_attribute_notify::kValueFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

s2c_update_player_attribute_notify::s2c_update_player_attribute_notify()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  if (GOOGLE_PREDICT_TRUE(this != internal_default_instance())) {
    protobuf_s2c_5fupdate_5fplayer_5fattribute_5fnotify_2eproto::InitDefaults();
  }
  SharedCtor();
  // @@protoc_insertion_point(constructor:s2c_update_player_attribute_notify)
}
s2c_update_player_attribute_notify::s2c_update_player_attribute_notify(const s2c_update_player_attribute_notify& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      _has_bits_(from._has_bits_),
      _cached_size_(0) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::memcpy(&value_, &from.value_,
    reinterpret_cast<char*>(&type_) -
    reinterpret_cast<char*>(&value_) + sizeof(type_));
  // @@protoc_insertion_point(copy_constructor:s2c_update_player_attribute_notify)
}

void s2c_update_player_attribute_notify::SharedCtor() {
  _cached_size_ = 0;
  ::memset(&value_, 0, reinterpret_cast<char*>(&type_) -
    reinterpret_cast<char*>(&value_) + sizeof(type_));
}

s2c_update_player_attribute_notify::~s2c_update_player_attribute_notify() {
  // @@protoc_insertion_point(destructor:s2c_update_player_attribute_notify)
  SharedDtor();
}

void s2c_update_player_attribute_notify::SharedDtor() {
}

void s2c_update_player_attribute_notify::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* s2c_update_player_attribute_notify::descriptor() {
  protobuf_s2c_5fupdate_5fplayer_5fattribute_5fnotify_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_s2c_5fupdate_5fplayer_5fattribute_5fnotify_2eproto::file_level_metadata[kIndexInFileMessages].descriptor;
}

const s2c_update_player_attribute_notify& s2c_update_player_attribute_notify::default_instance() {
  protobuf_s2c_5fupdate_5fplayer_5fattribute_5fnotify_2eproto::InitDefaults();
  return *internal_default_instance();
}

s2c_update_player_attribute_notify* s2c_update_player_attribute_notify::New(::google::protobuf::Arena* arena) const {
  s2c_update_player_attribute_notify* n = new s2c_update_player_attribute_notify;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void s2c_update_player_attribute_notify::Clear() {
// @@protoc_insertion_point(message_clear_start:s2c_update_player_attribute_notify)
  if (_has_bits_[0 / 32] & 3u) {
    ::memset(&value_, 0, reinterpret_cast<char*>(&type_) -
      reinterpret_cast<char*>(&value_) + sizeof(type_));
  }
  _has_bits_.Clear();
  _internal_metadata_.Clear();
}

bool s2c_update_player_attribute_notify::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:s2c_update_player_attribute_notify)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoffNoLastTag(127u);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required uint32 type = 1;
      case 1: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(8u)) {
          set_has_type();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint32, ::google::protobuf::internal::WireFormatLite::TYPE_UINT32>(
                 input, &type_)));
        } else {
          goto handle_unusual;
        }
        break;
      }

      // required uint64 value = 2;
      case 2: {
        if (static_cast< ::google::protobuf::uint8>(tag) ==
            static_cast< ::google::protobuf::uint8>(16u)) {
          set_has_value();
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint64, ::google::protobuf::internal::WireFormatLite::TYPE_UINT64>(
                 input, &value_)));
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
  // @@protoc_insertion_point(parse_success:s2c_update_player_attribute_notify)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:s2c_update_player_attribute_notify)
  return false;
#undef DO_
}

void s2c_update_player_attribute_notify::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:s2c_update_player_attribute_notify)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // required uint32 type = 1;
  if (cached_has_bits & 0x00000002u) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt32(1, this->type(), output);
  }

  // required uint64 value = 2;
  if (cached_has_bits & 0x00000001u) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt64(2, this->value(), output);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:s2c_update_player_attribute_notify)
}

::google::protobuf::uint8* s2c_update_player_attribute_notify::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  (void)deterministic; // Unused
  // @@protoc_insertion_point(serialize_to_array_start:s2c_update_player_attribute_notify)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // required uint32 type = 1;
  if (cached_has_bits & 0x00000002u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt32ToArray(1, this->type(), target);
  }

  // required uint64 value = 2;
  if (cached_has_bits & 0x00000001u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt64ToArray(2, this->value(), target);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:s2c_update_player_attribute_notify)
  return target;
}

size_t s2c_update_player_attribute_notify::RequiredFieldsByteSizeFallback() const {
// @@protoc_insertion_point(required_fields_byte_size_fallback_start:s2c_update_player_attribute_notify)
  size_t total_size = 0;

  if (has_value()) {
    // required uint64 value = 2;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt64Size(
        this->value());
  }

  if (has_type()) {
    // required uint32 type = 1;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->type());
  }

  return total_size;
}
size_t s2c_update_player_attribute_notify::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:s2c_update_player_attribute_notify)
  size_t total_size = 0;

  if (_internal_metadata_.have_unknown_fields()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  if (((_has_bits_[0] & 0x00000003) ^ 0x00000003) == 0) {  // All required fields are present.
    // required uint64 value = 2;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt64Size(
        this->value());

    // required uint32 type = 1;
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt32Size(
        this->type());

  } else {
    total_size += RequiredFieldsByteSizeFallback();
  }
  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = cached_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void s2c_update_player_attribute_notify::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:s2c_update_player_attribute_notify)
  GOOGLE_DCHECK_NE(&from, this);
  const s2c_update_player_attribute_notify* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const s2c_update_player_attribute_notify>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:s2c_update_player_attribute_notify)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:s2c_update_player_attribute_notify)
    MergeFrom(*source);
  }
}

void s2c_update_player_attribute_notify::MergeFrom(const s2c_update_player_attribute_notify& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:s2c_update_player_attribute_notify)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = from._has_bits_[0];
  if (cached_has_bits & 3u) {
    if (cached_has_bits & 0x00000001u) {
      value_ = from.value_;
    }
    if (cached_has_bits & 0x00000002u) {
      type_ = from.type_;
    }
    _has_bits_[0] |= cached_has_bits;
  }
}

void s2c_update_player_attribute_notify::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:s2c_update_player_attribute_notify)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void s2c_update_player_attribute_notify::CopyFrom(const s2c_update_player_attribute_notify& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:s2c_update_player_attribute_notify)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool s2c_update_player_attribute_notify::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000003) != 0x00000003) return false;
  return true;
}

void s2c_update_player_attribute_notify::Swap(s2c_update_player_attribute_notify* other) {
  if (other == this) return;
  InternalSwap(other);
}
void s2c_update_player_attribute_notify::InternalSwap(s2c_update_player_attribute_notify* other) {
  std::swap(value_, other->value_);
  std::swap(type_, other->type_);
  std::swap(_has_bits_[0], other->_has_bits_[0]);
  _internal_metadata_.Swap(&other->_internal_metadata_);
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata s2c_update_player_attribute_notify::GetMetadata() const {
  protobuf_s2c_5fupdate_5fplayer_5fattribute_5fnotify_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_s2c_5fupdate_5fplayer_5fattribute_5fnotify_2eproto::file_level_metadata[kIndexInFileMessages];
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// s2c_update_player_attribute_notify

// required uint32 type = 1;
bool s2c_update_player_attribute_notify::has_type() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
void s2c_update_player_attribute_notify::set_has_type() {
  _has_bits_[0] |= 0x00000002u;
}
void s2c_update_player_attribute_notify::clear_has_type() {
  _has_bits_[0] &= ~0x00000002u;
}
void s2c_update_player_attribute_notify::clear_type() {
  type_ = 0u;
  clear_has_type();
}
::google::protobuf::uint32 s2c_update_player_attribute_notify::type() const {
  // @@protoc_insertion_point(field_get:s2c_update_player_attribute_notify.type)
  return type_;
}
void s2c_update_player_attribute_notify::set_type(::google::protobuf::uint32 value) {
  set_has_type();
  type_ = value;
  // @@protoc_insertion_point(field_set:s2c_update_player_attribute_notify.type)
}

// required uint64 value = 2;
bool s2c_update_player_attribute_notify::has_value() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
void s2c_update_player_attribute_notify::set_has_value() {
  _has_bits_[0] |= 0x00000001u;
}
void s2c_update_player_attribute_notify::clear_has_value() {
  _has_bits_[0] &= ~0x00000001u;
}
void s2c_update_player_attribute_notify::clear_value() {
  value_ = GOOGLE_ULONGLONG(0);
  clear_has_value();
}
::google::protobuf::uint64 s2c_update_player_attribute_notify::value() const {
  // @@protoc_insertion_point(field_get:s2c_update_player_attribute_notify.value)
  return value_;
}
void s2c_update_player_attribute_notify::set_value(::google::protobuf::uint64 value) {
  set_has_value();
  value_ = value;
  // @@protoc_insertion_point(field_set:s2c_update_player_attribute_notify.value)
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)

// @@protoc_insertion_point(global_scope)
