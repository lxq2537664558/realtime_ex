// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: gate_addr_notify.proto

#ifndef PROTOBUF_gate_5faddr_5fnotify_2eproto__INCLUDED
#define PROTOBUF_gate_5faddr_5fnotify_2eproto__INCLUDED

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
class gate_addr_notify;
class gate_addr_notifyDefaultTypeInternal;
extern gate_addr_notifyDefaultTypeInternal _gate_addr_notify_default_instance_;

namespace protobuf_gate_5faddr_5fnotify_2eproto {
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
}  // namespace protobuf_gate_5faddr_5fnotify_2eproto

// ===================================================================

class gate_addr_notify : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:gate_addr_notify) */ {
 public:
  gate_addr_notify();
  virtual ~gate_addr_notify();

  gate_addr_notify(const gate_addr_notify& from);

  inline gate_addr_notify& operator=(const gate_addr_notify& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const gate_addr_notify& default_instance();

  static inline const gate_addr_notify* internal_default_instance() {
    return reinterpret_cast<const gate_addr_notify*>(
               &_gate_addr_notify_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(gate_addr_notify* other);

  // implements Message ----------------------------------------------

  inline gate_addr_notify* New() const PROTOBUF_FINAL { return New(NULL); }

  gate_addr_notify* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const gate_addr_notify& from);
  void MergeFrom(const gate_addr_notify& from);
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
  void InternalSwap(gate_addr_notify* other);
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

  // string addr = 1;
  void clear_addr();
  static const int kAddrFieldNumber = 1;
  const ::std::string& addr() const;
  void set_addr(const ::std::string& value);
  #if LANG_CXX11
  void set_addr(::std::string&& value);
  #endif
  void set_addr(const char* value);
  void set_addr(const char* value, size_t size);
  ::std::string* mutable_addr();
  ::std::string* release_addr();
  void set_allocated_addr(::std::string* addr);

  // @@protoc_insertion_point(class_scope:gate_addr_notify)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr addr_;
  mutable int _cached_size_;
  friend struct protobuf_gate_5faddr_5fnotify_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// gate_addr_notify

// string addr = 1;
inline void gate_addr_notify::clear_addr() {
  addr_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& gate_addr_notify::addr() const {
  // @@protoc_insertion_point(field_get:gate_addr_notify.addr)
  return addr_.GetNoArena();
}
inline void gate_addr_notify::set_addr(const ::std::string& value) {
  
  addr_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:gate_addr_notify.addr)
}
#if LANG_CXX11
inline void gate_addr_notify::set_addr(::std::string&& value) {
  
  addr_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:gate_addr_notify.addr)
}
#endif
inline void gate_addr_notify::set_addr(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  addr_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:gate_addr_notify.addr)
}
inline void gate_addr_notify::set_addr(const char* value, size_t size) {
  
  addr_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:gate_addr_notify.addr)
}
inline ::std::string* gate_addr_notify::mutable_addr() {
  
  // @@protoc_insertion_point(field_mutable:gate_addr_notify.addr)
  return addr_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* gate_addr_notify::release_addr() {
  // @@protoc_insertion_point(field_release:gate_addr_notify.addr)
  
  return addr_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void gate_addr_notify::set_allocated_addr(::std::string* addr) {
  if (addr != NULL) {
    
  } else {
    
  }
  addr_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), addr);
  // @@protoc_insertion_point(field_set_allocated:gate_addr_notify.addr)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)


// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_gate_5faddr_5fnotify_2eproto__INCLUDED