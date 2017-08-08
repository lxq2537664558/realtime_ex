// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: player_leave_gas_notify.proto

#ifndef PROTOBUF_player_5fleave_5fgas_5fnotify_2eproto__INCLUDED
#define PROTOBUF_player_5fleave_5fgas_5fnotify_2eproto__INCLUDED

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
class player_leave_gas_notify;
class player_leave_gas_notifyDefaultTypeInternal;
extern player_leave_gas_notifyDefaultTypeInternal _player_leave_gas_notify_default_instance_;

namespace protobuf_player_5fleave_5fgas_5fnotify_2eproto {
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
}  // namespace protobuf_player_5fleave_5fgas_5fnotify_2eproto

// ===================================================================

class player_leave_gas_notify : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:player_leave_gas_notify) */ {
 public:
  player_leave_gas_notify();
  virtual ~player_leave_gas_notify();

  player_leave_gas_notify(const player_leave_gas_notify& from);

  inline player_leave_gas_notify& operator=(const player_leave_gas_notify& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const player_leave_gas_notify& default_instance();

  static inline const player_leave_gas_notify* internal_default_instance() {
    return reinterpret_cast<const player_leave_gas_notify*>(
               &_player_leave_gas_notify_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(player_leave_gas_notify* other);

  // implements Message ----------------------------------------------

  inline player_leave_gas_notify* New() const PROTOBUF_FINAL { return New(NULL); }

  player_leave_gas_notify* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const player_leave_gas_notify& from);
  void MergeFrom(const player_leave_gas_notify& from);
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
  void InternalSwap(player_leave_gas_notify* other);
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

  // uint64 player_id = 1;
  void clear_player_id();
  static const int kPlayerIdFieldNumber = 1;
  ::google::protobuf::uint64 player_id() const;
  void set_player_id(::google::protobuf::uint64 value);

  // @@protoc_insertion_point(class_scope:player_leave_gas_notify)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::uint64 player_id_;
  mutable int _cached_size_;
  friend struct protobuf_player_5fleave_5fgas_5fnotify_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// player_leave_gas_notify

// uint64 player_id = 1;
inline void player_leave_gas_notify::clear_player_id() {
  player_id_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 player_leave_gas_notify::player_id() const {
  // @@protoc_insertion_point(field_get:player_leave_gas_notify.player_id)
  return player_id_;
}
inline void player_leave_gas_notify::set_player_id(::google::protobuf::uint64 value) {
  
  player_id_ = value;
  // @@protoc_insertion_point(field_set:player_leave_gas_notify.player_id)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)


// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_player_5fleave_5fgas_5fnotify_2eproto__INCLUDED