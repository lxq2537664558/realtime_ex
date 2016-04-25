// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: service_request_msg.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "service_request_msg.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace test {

namespace {

const ::google::protobuf::Descriptor* service_request_msg_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  service_request_msg_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_service_5frequest_5fmsg_2eproto() {
  protobuf_AddDesc_service_5frequest_5fmsg_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "service_request_msg.proto");
  GOOGLE_CHECK(file != NULL);
  service_request_msg_descriptor_ = file->message_type(0);
  static const int service_request_msg_offsets_[1] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(service_request_msg, name_),
  };
  service_request_msg_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      service_request_msg_descriptor_,
      service_request_msg::default_instance_,
      service_request_msg_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(service_request_msg, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(service_request_msg, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(service_request_msg));
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_service_5frequest_5fmsg_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    service_request_msg_descriptor_, &service_request_msg::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_service_5frequest_5fmsg_2eproto() {
  delete service_request_msg::default_instance_;
  delete service_request_msg_reflection_;
}

void protobuf_AddDesc_service_5frequest_5fmsg_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\031service_request_msg.proto\022\004test\"#\n\023ser"
    "vice_request_msg\022\014\n\004name\030\001 \001(\t", 70);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "service_request_msg.proto", &protobuf_RegisterTypes);
  service_request_msg::default_instance_ = new service_request_msg();
  service_request_msg::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_service_5frequest_5fmsg_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_service_5frequest_5fmsg_2eproto {
  StaticDescriptorInitializer_service_5frequest_5fmsg_2eproto() {
    protobuf_AddDesc_service_5frequest_5fmsg_2eproto();
  }
} static_descriptor_initializer_service_5frequest_5fmsg_2eproto_;

// ===================================================================

#ifndef _MSC_VER
const int service_request_msg::kNameFieldNumber;
#endif  // !_MSC_VER

service_request_msg::service_request_msg()
  : ::google::protobuf::Message() {
  SharedCtor();
  // @@protoc_insertion_point(constructor:test.service_request_msg)
}

void service_request_msg::InitAsDefaultInstance() {
}

service_request_msg::service_request_msg(const service_request_msg& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:test.service_request_msg)
}

void service_request_msg::SharedCtor() {
  ::google::protobuf::internal::GetEmptyString();
  _cached_size_ = 0;
  name_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

service_request_msg::~service_request_msg() {
  // @@protoc_insertion_point(destructor:test.service_request_msg)
  SharedDtor();
}

void service_request_msg::SharedDtor() {
  if (name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete name_;
  }
  if (this != default_instance_) {
  }
}

void service_request_msg::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* service_request_msg::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return service_request_msg_descriptor_;
}

const service_request_msg& service_request_msg::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_service_5frequest_5fmsg_2eproto();
  return *default_instance_;
}

service_request_msg* service_request_msg::default_instance_ = NULL;

service_request_msg* service_request_msg::New() const {
  return new service_request_msg;
}

void service_request_msg::Clear() {
  if (has_name()) {
    if (name_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
      name_->clear();
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool service_request_msg::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:test.service_request_msg)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional string name = 1;
      case 1: {
        if (tag == 10) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_name()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->name().data(), this->name().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "name");
        } else {
          goto handle_unusual;
        }
        if (input->ExpectAtEnd()) goto success;
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
  // @@protoc_insertion_point(parse_success:test.service_request_msg)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:test.service_request_msg)
  return false;
#undef DO_
}

void service_request_msg::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:test.service_request_msg)
  // optional string name = 1;
  if (has_name()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->name().data(), this->name().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "name");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      1, this->name(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:test.service_request_msg)
}

::google::protobuf::uint8* service_request_msg::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:test.service_request_msg)
  // optional string name = 1;
  if (has_name()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->name().data(), this->name().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "name");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        1, this->name(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:test.service_request_msg)
  return target;
}

int service_request_msg::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // optional string name = 1;
    if (has_name()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->name());
    }

  }
  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void service_request_msg::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const service_request_msg* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const service_request_msg*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void service_request_msg::MergeFrom(const service_request_msg& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_name()) {
      set_name(from.name());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void service_request_msg::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void service_request_msg::CopyFrom(const service_request_msg& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool service_request_msg::IsInitialized() const {

  return true;
}

void service_request_msg::Swap(service_request_msg* other) {
  if (other != this) {
    std::swap(name_, other->name_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata service_request_msg::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = service_request_msg_descriptor_;
  metadata.reflection = service_request_msg_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace test

// @@protoc_insertion_point(global_scope)
