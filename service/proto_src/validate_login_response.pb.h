// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: validate_login_response.proto

#ifndef PROTOBUF_validate_5flogin_5fresponse_2eproto__INCLUDED
#define PROTOBUF_validate_5flogin_5fresponse_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3003000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3003002 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
class validate_login_response;
class validate_login_responseDefaultTypeInternal;
extern validate_login_responseDefaultTypeInternal _validate_login_response_default_instance_;

namespace protobuf_validate_5flogin_5fresponse_2eproto {
// Internal implementation detail -- do not call these.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[];
  static const ::google::protobuf::uint32 offsets[];
  static void InitDefaultsImpl();
  static void Shutdown();
};
void AddDescriptors();
void InitDefaults();
}  // namespace protobuf_validate_5flogin_5fresponse_2eproto

// ===================================================================

class validate_login_response : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:validate_login_response) */ {
 public:
  validate_login_response();
  virtual ~validate_login_response();

  validate_login_response(const validate_login_response& from);

  inline validate_login_response& operator=(const validate_login_response& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const validate_login_response& default_instance();

  static inline const validate_login_response* internal_default_instance() {
    return reinterpret_cast<const validate_login_response*>(
               &_validate_login_response_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(validate_login_response* other);

  // implements Message ----------------------------------------------

  inline validate_login_response* New() const PROTOBUF_FINAL { return New(NULL); }

  validate_login_response* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const validate_login_response& from);
  void MergeFrom(const validate_login_response& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(validate_login_response* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string key = 1;
  void clear_key();
  static const int kKeyFieldNumber = 1;
  const ::std::string& key() const;
  void set_key(const ::std::string& value);
  #if LANG_CXX11
  void set_key(::std::string&& value);
  #endif
  void set_key(const char* value);
  void set_key(const char* value, size_t size);
  ::std::string* mutable_key();
  ::std::string* release_key();
  void set_allocated_key(::std::string* key);

  // string gate_addr = 3;
  void clear_gate_addr();
  static const int kGateAddrFieldNumber = 3;
  const ::std::string& gate_addr() const;
  void set_gate_addr(const ::std::string& value);
  #if LANG_CXX11
  void set_gate_addr(::std::string&& value);
  #endif
  void set_gate_addr(const char* value);
  void set_gate_addr(const char* value, size_t size);
  ::std::string* mutable_gate_addr();
  ::std::string* release_gate_addr();
  void set_allocated_gate_addr(::std::string* gate_addr);

  // uint32 result = 2;
  void clear_result();
  static const int kResultFieldNumber = 2;
  ::google::protobuf::uint32 result() const;
  void set_result(::google::protobuf::uint32 value);

  // @@protoc_insertion_point(class_scope:validate_login_response)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr key_;
  ::google::protobuf::internal::ArenaStringPtr gate_addr_;
  ::google::protobuf::uint32 result_;
  mutable int _cached_size_;
  friend struct protobuf_validate_5flogin_5fresponse_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// validate_login_response

// string key = 1;
inline void validate_login_response::clear_key() {
  key_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& validate_login_response::key() const {
  // @@protoc_insertion_point(field_get:validate_login_response.key)
  return key_.GetNoArena();
}
inline void validate_login_response::set_key(const ::std::string& value) {
  
  key_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:validate_login_response.key)
}
#if LANG_CXX11
inline void validate_login_response::set_key(::std::string&& value) {
  
  key_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:validate_login_response.key)
}
#endif
inline void validate_login_response::set_key(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  key_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:validate_login_response.key)
}
inline void validate_login_response::set_key(const char* value, size_t size) {
  
  key_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:validate_login_response.key)
}
inline ::std::string* validate_login_response::mutable_key() {
  
  // @@protoc_insertion_point(field_mutable:validate_login_response.key)
  return key_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* validate_login_response::release_key() {
  // @@protoc_insertion_point(field_release:validate_login_response.key)
  
  return key_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void validate_login_response::set_allocated_key(::std::string* key) {
  if (key != NULL) {
    
  } else {
    
  }
  key_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), key);
  // @@protoc_insertion_point(field_set_allocated:validate_login_response.key)
}

// uint32 result = 2;
inline void validate_login_response::clear_result() {
  result_ = 0u;
}
inline ::google::protobuf::uint32 validate_login_response::result() const {
  // @@protoc_insertion_point(field_get:validate_login_response.result)
  return result_;
}
inline void validate_login_response::set_result(::google::protobuf::uint32 value) {
  
  result_ = value;
  // @@protoc_insertion_point(field_set:validate_login_response.result)
}

// string gate_addr = 3;
inline void validate_login_response::clear_gate_addr() {
  gate_addr_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& validate_login_response::gate_addr() const {
  // @@protoc_insertion_point(field_get:validate_login_response.gate_addr)
  return gate_addr_.GetNoArena();
}
inline void validate_login_response::set_gate_addr(const ::std::string& value) {
  
  gate_addr_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:validate_login_response.gate_addr)
}
#if LANG_CXX11
inline void validate_login_response::set_gate_addr(::std::string&& value) {
  
  gate_addr_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:validate_login_response.gate_addr)
}
#endif
inline void validate_login_response::set_gate_addr(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  gate_addr_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:validate_login_response.gate_addr)
}
inline void validate_login_response::set_gate_addr(const char* value, size_t size) {
  
  gate_addr_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:validate_login_response.gate_addr)
}
inline ::std::string* validate_login_response::mutable_gate_addr() {
  
  // @@protoc_insertion_point(field_mutable:validate_login_response.gate_addr)
  return gate_addr_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* validate_login_response::release_gate_addr() {
  // @@protoc_insertion_point(field_release:validate_login_response.gate_addr)
  
  return gate_addr_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void validate_login_response::set_allocated_gate_addr(::std::string* gate_addr) {
  if (gate_addr != NULL) {
    
  } else {
    
  }
  gate_addr_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), gate_addr);
  // @@protoc_insertion_point(field_set_allocated:validate_login_response.gate_addr)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)


// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_validate_5flogin_5fresponse_2eproto__INCLUDED
