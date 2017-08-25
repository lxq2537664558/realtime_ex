// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: g2s_player_enter_request.proto

#ifndef PROTOBUF_g2s_5fplayer_5fenter_5frequest_2eproto__INCLUDED
#define PROTOBUF_g2s_5fplayer_5fenter_5frequest_2eproto__INCLUDED

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
class g2s_player_enter_request;
class g2s_player_enter_requestDefaultTypeInternal;
extern g2s_player_enter_requestDefaultTypeInternal _g2s_player_enter_request_default_instance_;

namespace protobuf_g2s_5fplayer_5fenter_5frequest_2eproto {
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
}  // namespace protobuf_g2s_5fplayer_5fenter_5frequest_2eproto

// ===================================================================

class g2s_player_enter_request : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:g2s_player_enter_request) */ {
 public:
  g2s_player_enter_request();
  virtual ~g2s_player_enter_request();

  g2s_player_enter_request(const g2s_player_enter_request& from);

  inline g2s_player_enter_request& operator=(const g2s_player_enter_request& from) {
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
  static const g2s_player_enter_request& default_instance();

  static inline const g2s_player_enter_request* internal_default_instance() {
    return reinterpret_cast<const g2s_player_enter_request*>(
               &_g2s_player_enter_request_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(g2s_player_enter_request* other);

  // implements Message ----------------------------------------------

  inline g2s_player_enter_request* New() const PROTOBUF_FINAL { return New(NULL); }

  g2s_player_enter_request* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const g2s_player_enter_request& from);
  void MergeFrom(const g2s_player_enter_request& from);
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
  void InternalSwap(g2s_player_enter_request* other);
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

  // required uint64 player_id = 1;
  bool has_player_id() const;
  void clear_player_id();
  static const int kPlayerIdFieldNumber = 1;
  ::google::protobuf::uint64 player_id() const;
  void set_player_id(::google::protobuf::uint64 value);

  // @@protoc_insertion_point(class_scope:g2s_player_enter_request)
 private:
  void set_has_player_id();
  void clear_has_player_id();

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::HasBits<1> _has_bits_;
  mutable int _cached_size_;
  ::google::protobuf::uint64 player_id_;
  friend struct protobuf_g2s_5fplayer_5fenter_5frequest_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// g2s_player_enter_request

// required uint64 player_id = 1;
inline bool g2s_player_enter_request::has_player_id() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void g2s_player_enter_request::set_has_player_id() {
  _has_bits_[0] |= 0x00000001u;
}
inline void g2s_player_enter_request::clear_has_player_id() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void g2s_player_enter_request::clear_player_id() {
  player_id_ = GOOGLE_ULONGLONG(0);
  clear_has_player_id();
}
inline ::google::protobuf::uint64 g2s_player_enter_request::player_id() const {
  // @@protoc_insertion_point(field_get:g2s_player_enter_request.player_id)
  return player_id_;
}
inline void g2s_player_enter_request::set_player_id(::google::protobuf::uint64 value) {
  set_has_player_id();
  player_id_ = value;
  // @@protoc_insertion_point(field_set:g2s_player_enter_request.player_id)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS

// @@protoc_insertion_point(namespace_scope)


// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_g2s_5fplayer_5fenter_5frequest_2eproto__INCLUDED