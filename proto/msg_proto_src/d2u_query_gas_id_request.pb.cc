// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: d2u_query_gas_id_request.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "d2u_query_gas_id_request.pb.h"

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
class d2u_query_gas_id_requestDefaultTypeInternal : public ::google::protobuf::internal::ExplicitlyConstructed<d2u_query_gas_id_request> {
} _d2u_query_gas_id_request_default_instance_;

namespace protobuf_d2u_5fquery_5fgas_5fid_5frequest_2eproto {


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
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(d2u_query_gas_id_request, _has_bits_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(d2u_query_gas_id_request, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(d2u_query_gas_id_request, player_id_),
  0,
};

static const ::google::protobuf::internal::MigrationSchema schemas[] = {
  { 0, 6, sizeof(d2u_query_gas_id_request)},
};

static ::google::protobuf::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::google::protobuf::Message*>(&_d2u_query_gas_id_request_default_instance_),
};

namespace {

void protobuf_AssignDescriptors() {
  AddDescriptors();
  ::google::protobuf::MessageFactory* factory = NULL;
  AssignDescriptors(
      "d2u_query_gas_id_request.proto", schemas, file_default_instances, TableStruct::offsets, factory,
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
  _d2u_query_gas_id_request_default_instance_.Shutdown();
  delete file_level_metadata[0].reflection;
}

void TableStruct::InitDefaultsImpl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::internal::InitProtobufDefaults();
  _d2u_query_gas_id_request_default_instance_.DefaultConstruct();
}

void InitDefaults() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &TableStruct::InitDefaultsImpl);
}
void AddDescriptorsImpl() {
  InitDefaults();
  static const char descriptor[] = {
      "\n\036d2u_query_gas_id_request.proto\"-\n\030d2u_"
      "query_gas_id_request\022\021\n\tplayer_id\030\001 \002(\004"
  };
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
      descriptor, 79);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "d2u_query_gas_id_request.proto", &protobuf_RegisterTypes);
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

}  // namespace protobuf_d2u_5fquery_5fgas_5fid_5frequest_2eproto


// ===================================================================

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int d2u_query_gas_id_request::kPlayerIdFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

d2u_query_gas_id_request::d2u_query_gas_id_request()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  if (GOOGLE_PREDICT_TRUE(this != internal_default_instance())) {
    protobuf_d2u_5fquery_5fgas_5fid_5frequest_2eproto::InitDefaults();
  }
  SharedCtor();
  // @@protoc_insertion_point(constructor:d2u_query_gas_id_request)
}
d2u_query_gas_id_request::d2u_query_gas_id_request(const d2u_query_gas_id_request& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      _has_bits_(from._has_bits_),
      _cached_size_(0) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  player_id_ = from.player_id_;
  // @@protoc_insertion_point(copy_constructor:d2u_query_gas_id_request)
}

void d2u_query_gas_id_request::SharedCtor() {
  _cached_size_ = 0;
  player_id_ = GOOGLE_ULONGLONG(0);
}

d2u_query_gas_id_request::~d2u_query_gas_id_request() {
  // @@protoc_insertion_point(destructor:d2u_query_gas_id_request)
  SharedDtor();
}

void d2u_query_gas_id_request::SharedDtor() {
}

void d2u_query_gas_id_request::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* d2u_query_gas_id_request::descriptor() {
  protobuf_d2u_5fquery_5fgas_5fid_5frequest_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_d2u_5fquery_5fgas_5fid_5frequest_2eproto::file_level_metadata[kIndexInFileMessages].descriptor;
}

const d2u_query_gas_id_request& d2u_query_gas_id_request::default_instance() {
  protobuf_d2u_5fquery_5fgas_5fid_5frequest_2eproto::InitDefaults();
  return *internal_default_instance();
}

d2u_query_gas_id_request* d2u_query_gas_id_request::New(::google::protobuf::Arena* arena) const {
  d2u_query_gas_id_request* n = new d2u_query_gas_id_request;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void d2u_query_gas_id_request::Clear() {
// @@protoc_insertion_point(message_clear_start:d2u_query_gas_id_request)
  player_id_ = GOOGLE_ULONGLONG(0);
  _has_bits_.Clear();
  _internal_metadata_.Clear();
}

bool d2u_query_gas_id_request::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!GOOGLE_PREDICT_TRUE(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:d2u_query_gas_id_request)
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
  // @@protoc_insertion_point(parse_success:d2u_query_gas_id_request)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:d2u_query_gas_id_request)
  return false;
#undef DO_
}

void d2u_query_gas_id_request::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:d2u_query_gas_id_request)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // required uint64 player_id = 1;
  if (cached_has_bits & 0x00000001u) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt64(1, this->player_id(), output);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:d2u_query_gas_id_request)
}

::google::protobuf::uint8* d2u_query_gas_id_request::InternalSerializeWithCachedSizesToArray(
    bool deterministic, ::google::protobuf::uint8* target) const {
  (void)deterministic; // Unused
  // @@protoc_insertion_point(serialize_to_array_start:d2u_query_gas_id_request)
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  cached_has_bits = _has_bits_[0];
  // required uint64 player_id = 1;
  if (cached_has_bits & 0x00000001u) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt64ToArray(1, this->player_id(), target);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:d2u_query_gas_id_request)
  return target;
}

size_t d2u_query_gas_id_request::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:d2u_query_gas_id_request)
  size_t total_size = 0;

  if (_internal_metadata_.have_unknown_fields()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  // required uint64 player_id = 1;
  if (has_player_id()) {
    total_size += 1 +
      ::google::protobuf::internal::WireFormatLite::UInt64Size(
        this->player_id());
  }
  int cached_size = ::google::protobuf::internal::ToCachedSize(total_size);
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = cached_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void d2u_query_gas_id_request::MergeFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_merge_from_start:d2u_query_gas_id_request)
  GOOGLE_DCHECK_NE(&from, this);
  const d2u_query_gas_id_request* source =
      ::google::protobuf::internal::DynamicCastToGenerated<const d2u_query_gas_id_request>(
          &from);
  if (source == NULL) {
  // @@protoc_insertion_point(generalized_merge_from_cast_fail:d2u_query_gas_id_request)
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
  // @@protoc_insertion_point(generalized_merge_from_cast_success:d2u_query_gas_id_request)
    MergeFrom(*source);
  }
}

void d2u_query_gas_id_request::MergeFrom(const d2u_query_gas_id_request& from) {
// @@protoc_insertion_point(class_specific_merge_from_start:d2u_query_gas_id_request)
  GOOGLE_DCHECK_NE(&from, this);
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  ::google::protobuf::uint32 cached_has_bits = 0;
  (void) cached_has_bits;

  if (from.has_player_id()) {
    set_player_id(from.player_id());
  }
}

void d2u_query_gas_id_request::CopyFrom(const ::google::protobuf::Message& from) {
// @@protoc_insertion_point(generalized_copy_from_start:d2u_query_gas_id_request)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void d2u_query_gas_id_request::CopyFrom(const d2u_query_gas_id_request& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:d2u_query_gas_id_request)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool d2u_query_gas_id_request::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000001) != 0x00000001) return false;
  return true;
}

void d2u_query_gas_id_request::Swap(d2u_query_gas_id_request* other) {
  if (other == this) return;
  InternalSwap(other);
}
void d2u_query_gas_id_request::InternalSwap(d2u_query_gas_id_request* other) {
  std::swap(player_id_, other->player_id_);
  std::swap(_has_bits_[0], other->_has_bits_[0]);
  _internal_metadata_.Swap(&other->_internal_metadata_);
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata d2u_query_gas_id_request::GetMetadata() const {
  protobuf_d2u_5fquery_5fgas_5fid_5frequest_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_d2u_5fquery_5fgas_5fid_5frequest_2eproto::file_level_metadata[kIndexInFileMessages];
}

#if PROTOBUF_INLINE_NOT_IN_HEADERS
// d2u_query_gas_id_request

// required uint64 player_id = 1;
bool d2u_query_gas_id_request::has_player_id() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
void d2u_query_gas_id_request::set_has_player_id() {
  _has_bits_[0] |= 0x00000001u;
}
void d2u_query_gas_id_request::clear_has_player_id() {
  _has_bits_[0] &= ~0x00000001u;
}
void d2u_query_gas_id_request::clear_player_id() {
  player_id_ = GOOGLE_ULONGLONG(0);
  clear_has_player_id();
}
::google::protobuf::uint64 d2u_query_gas_id_request::player_id() const {
  // @@protoc_insertion_point(field_get:d2u_query_gas_id_request.player_id)
  return player_id_;
}
void d2u_query_gas_id_request::set_player_id(::google::protobuf::uint64 value) {
  set_has_player_id();
  player_id_ = value;
  // @@protoc_insertion_point(field_set:d2u_query_gas_id_request.player_id)
}

#endif  // PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)

// @@protoc_insertion_point(global_scope)