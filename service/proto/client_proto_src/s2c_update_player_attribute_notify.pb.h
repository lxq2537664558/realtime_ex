// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: s2c_update_player_attribute_notify.proto

#ifndef PROTOBUF_s2c_5fupdate_5fplayer_5fattribute_5fnotify_2eproto__INCLUDED
#define PROTOBUF_s2c_5fupdate_5fplayer_5fattribute_5fnotify_2eproto__INCLUDED

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
class s2c_update_player_attribute_notify;
class s2c_update_player_attribute_notifyDefaultTypeInternal;
extern s2c_update_player_attribute_notifyDefaultTypeInternal _s2c_update_player_attribute_notify_default_instance_;

namespace protobuf_s2c_5fupdate_5fplayer_5fattribute_5fnotify_2eproto {
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
}  // namespace protobuf_s2c_5fupdate_5fplayer_5fattribute_5fnotify_2eproto

// ===================================================================

class s2c_update_player_attribute_notify : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:s2c_update_player_attribute_notify) */ {
 public:
  s2c_update_player_attribute_notify();
  virtual ~s2c_update_player_attribute_notify();

  s2c_update_player_attribute_notify(const s2c_update_player_attribute_notify& from);

  inline s2c_update_player_attribute_notify& operator=(const s2c_update_player_attribute_notify& from) {
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
  static const s2c_update_player_attribute_notify& default_instance();

  static inline const s2c_update_player_attribute_notify* internal_default_instance() {
    return reinterpret_cast<const s2c_update_player_attribute_notify*>(
               &_s2c_update_player_attribute_notify_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(s2c_update_player_attribute_notify* other);

  // implements Message ----------------------------------------------

  inline s2c_update_player_attribute_notify* New() const PROTOBUF_FINAL { return New(NULL); }

  s2c_update_player_attribute_notify* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const s2c_update_player_attribute_notify& from);
  void MergeFrom(const s2c_update_player_attribute_notify& from);
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
  void InternalSwap(s2c_update_player_attribute_notify* other);
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

  // required uint64 value = 2;
  bool has_value() const;
  void clear_value();
  static const int kValueFieldNumber = 2;
  ::google::protobuf::uint64 value() const;
  void set_value(::google::protobuf::uint64 value);

  // required uint32 type = 1;
  bool has_type() const;
  void clear_type();
  static const int kTypeFieldNumber = 1;
  ::google::protobuf::uint32 type() const;
  void set_type(::google::protobuf::uint32 value);

  // @@protoc_insertion_point(class_scope:s2c_update_player_attribute_notify)
 private:
  void set_has_type();
  void clear_has_type();
  void set_has_value();
  void clear_has_value();

  // helper for ByteSizeLong()
  size_t RequiredFieldsByteSizeFallback() const;

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::HasBits<1> _has_bits_;
  mutable int _cached_size_;
  ::google::protobuf::uint64 value_;
  ::google::protobuf::uint32 type_;
  friend struct protobuf_s2c_5fupdate_5fplayer_5fattribute_5fnotify_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// s2c_update_player_attribute_notify

// required uint32 type = 1;
inline bool s2c_update_player_attribute_notify::has_type() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void s2c_update_player_attribute_notify::set_has_type() {
  _has_bits_[0] |= 0x00000002u;
}
inline void s2c_update_player_attribute_notify::clear_has_type() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void s2c_update_player_attribute_notify::clear_type() {
  type_ = 0u;
  clear_has_type();
}
inline ::google::protobuf::uint32 s2c_update_player_attribute_notify::type() const {
  // @@protoc_insertion_point(field_get:s2c_update_player_attribute_notify.type)
  return type_;
}
inline void s2c_update_player_attribute_notify::set_type(::google::protobuf::uint32 value) {
  set_has_type();
  type_ = value;
  // @@protoc_insertion_point(field_set:s2c_update_player_attribute_notify.type)
}

// required uint64 value = 2;
inline bool s2c_update_player_attribute_notify::has_value() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void s2c_update_player_attribute_notify::set_has_value() {
  _has_bits_[0] |= 0x00000001u;
}
inline void s2c_update_player_attribute_notify::clear_has_value() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void s2c_update_player_attribute_notify::clear_value() {
  value_ = GOOGLE_ULONGLONG(0);
  clear_has_value();
}
inline ::google::protobuf::uint64 s2c_update_player_attribute_notify::value() const {
  // @@protoc_insertion_point(field_get:s2c_update_player_attribute_notify.value)
  return value_;
}
inline void s2c_update_player_attribute_notify::set_value(::google::protobuf::uint64 value) {
  set_has_value();
  value_ = value;
  // @@protoc_insertion_point(field_set:s2c_update_player_attribute_notify.value)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)


// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_s2c_5fupdate_5fplayer_5fattribute_5fnotify_2eproto__INCLUDED