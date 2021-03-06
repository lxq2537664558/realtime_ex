// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: select_command.proto

#ifndef PROTOBUF_select_5fcommand_2eproto__INCLUDED
#define PROTOBUF_select_5fcommand_2eproto__INCLUDED

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
namespace proto {
namespace db {
class select_command;
class select_commandDefaultTypeInternal;
extern select_commandDefaultTypeInternal _select_command_default_instance_;
}  // namespace db
}  // namespace proto

namespace proto {
namespace db {

namespace protobuf_select_5fcommand_2eproto {
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
}  // namespace protobuf_select_5fcommand_2eproto

// ===================================================================

class select_command : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:proto.db.select_command) */ {
 public:
  select_command();
  virtual ~select_command();

  select_command(const select_command& from);

  inline select_command& operator=(const select_command& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const select_command& default_instance();

  static inline const select_command* internal_default_instance() {
    return reinterpret_cast<const select_command*>(
               &_select_command_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(select_command* other);

  // implements Message ----------------------------------------------

  inline select_command* New() const PROTOBUF_FINAL { return New(NULL); }

  select_command* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const select_command& from);
  void MergeFrom(const select_command& from);
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
  void InternalSwap(select_command* other);
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

  // required string table_name = 2;
  bool has_table_name() const;
  void clear_table_name();
  static const int kTableNameFieldNumber = 2;
  const ::std::string& table_name() const;
  void set_table_name(const ::std::string& value);
  #if LANG_CXX11
  void set_table_name(::std::string&& value);
  #endif
  void set_table_name(const char* value);
  void set_table_name(const char* value, size_t size);
  ::std::string* mutable_table_name();
  ::std::string* release_table_name();
  void set_allocated_table_name(::std::string* table_name);

  // required uint64 id = 1;
  bool has_id() const;
  void clear_id();
  static const int kIdFieldNumber = 1;
  ::google::protobuf::uint64 id() const;
  void set_id(::google::protobuf::uint64 value);

  // @@protoc_insertion_point(class_scope:proto.db.select_command)
 private:
  void set_has_id();
  void clear_has_id();
  void set_has_table_name();
  void clear_has_table_name();

  // helper for ByteSizeLong()
  size_t RequiredFieldsByteSizeFallback() const;

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::HasBits<1> _has_bits_;
  mutable int _cached_size_;
  ::google::protobuf::internal::ArenaStringPtr table_name_;
  ::google::protobuf::uint64 id_;
  friend struct protobuf_select_5fcommand_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// select_command

// required uint64 id = 1;
inline bool select_command::has_id() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void select_command::set_has_id() {
  _has_bits_[0] |= 0x00000002u;
}
inline void select_command::clear_has_id() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void select_command::clear_id() {
  id_ = GOOGLE_ULONGLONG(0);
  clear_has_id();
}
inline ::google::protobuf::uint64 select_command::id() const {
  // @@protoc_insertion_point(field_get:proto.db.select_command.id)
  return id_;
}
inline void select_command::set_id(::google::protobuf::uint64 value) {
  set_has_id();
  id_ = value;
  // @@protoc_insertion_point(field_set:proto.db.select_command.id)
}

// required string table_name = 2;
inline bool select_command::has_table_name() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void select_command::set_has_table_name() {
  _has_bits_[0] |= 0x00000001u;
}
inline void select_command::clear_has_table_name() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void select_command::clear_table_name() {
  table_name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_table_name();
}
inline const ::std::string& select_command::table_name() const {
  // @@protoc_insertion_point(field_get:proto.db.select_command.table_name)
  return table_name_.GetNoArena();
}
inline void select_command::set_table_name(const ::std::string& value) {
  set_has_table_name();
  table_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:proto.db.select_command.table_name)
}
#if LANG_CXX11
inline void select_command::set_table_name(::std::string&& value) {
  set_has_table_name();
  table_name_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:proto.db.select_command.table_name)
}
#endif
inline void select_command::set_table_name(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  set_has_table_name();
  table_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:proto.db.select_command.table_name)
}
inline void select_command::set_table_name(const char* value, size_t size) {
  set_has_table_name();
  table_name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:proto.db.select_command.table_name)
}
inline ::std::string* select_command::mutable_table_name() {
  set_has_table_name();
  // @@protoc_insertion_point(field_mutable:proto.db.select_command.table_name)
  return table_name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* select_command::release_table_name() {
  // @@protoc_insertion_point(field_release:proto.db.select_command.table_name)
  clear_has_table_name();
  return table_name_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void select_command::set_allocated_table_name(::std::string* table_name) {
  if (table_name != NULL) {
    set_has_table_name();
  } else {
    clear_has_table_name();
  }
  table_name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), table_name);
  // @@protoc_insertion_point(field_set_allocated:proto.db.select_command.table_name)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)


}  // namespace db
}  // namespace proto

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_select_5fcommand_2eproto__INCLUDED
