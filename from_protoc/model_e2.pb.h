// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: model_e2.proto

#ifndef PROTOBUF_model_5fe2_2eproto__INCLUDED
#define PROTOBUF_model_5fe2_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2004000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2004001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_message_reflection.h>
// @@protoc_insertion_point(includes)

namespace pb {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_model_5fe2_2eproto();
void protobuf_AssignDesc_model_5fe2_2eproto();
void protobuf_ShutdownFile_model_5fe2_2eproto();

class E2Config;
class E2State;
class E2State_Particles;
class E2PetscSolverConfig;

// ===================================================================

class E2Config : public ::google::protobuf::Message {
 public:
  E2Config();
  virtual ~E2Config();
  
  E2Config(const E2Config& from);
  
  inline E2Config& operator=(const E2Config& from) {
    CopyFrom(from);
    return *this;
  }
  
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }
  
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }
  
  static const ::google::protobuf::Descriptor* descriptor();
  static const E2Config& default_instance();
  
  void Swap(E2Config* other);
  
  // implements Message ----------------------------------------------
  
  E2Config* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const E2Config& from);
  void MergeFrom(const E2Config& from);
  void Clear();
  bool IsInitialized() const;
  
  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  
  ::google::protobuf::Metadata GetMetadata() const;
  
  // nested types ----------------------------------------------------
  
  // accessors -------------------------------------------------------
  
  // required uint32 N = 1;
  inline bool has_n() const;
  inline void clear_n();
  static const int kNFieldNumber = 1;
  inline ::google::protobuf::uint32 n() const;
  inline void set_n(::google::protobuf::uint32 value);
  
  // optional double A = 2;
  inline bool has_a() const;
  inline void clear_a();
  static const int kAFieldNumber = 2;
  inline double a() const;
  inline void set_a(double value);
  
  // optional double delta = 3;
  inline bool has_delta() const;
  inline void clear_delta();
  static const int kDeltaFieldNumber = 3;
  inline double delta() const;
  inline void set_delta(double value);
  
  // optional double f = 4;
  inline bool has_f() const;
  inline void clear_f();
  static const int kFFieldNumber = 4;
  inline double f() const;
  inline void set_f(double value);
  
  // @@protoc_insertion_point(class_scope:pb.E2Config)
 private:
  inline void set_has_n();
  inline void clear_has_n();
  inline void set_has_a();
  inline void clear_has_a();
  inline void set_has_delta();
  inline void clear_has_delta();
  inline void set_has_f();
  inline void clear_has_f();
  
  ::google::protobuf::UnknownFieldSet _unknown_fields_;
  
  double a_;
  double delta_;
  double f_;
  ::google::protobuf::uint32 n_;
  
  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(4 + 31) / 32];
  
  friend void  protobuf_AddDesc_model_5fe2_2eproto();
  friend void protobuf_AssignDesc_model_5fe2_2eproto();
  friend void protobuf_ShutdownFile_model_5fe2_2eproto();
  
  void InitAsDefaultInstance();
  static E2Config* default_instance_;
};
// -------------------------------------------------------------------

class E2State_Particles : public ::google::protobuf::Message {
 public:
  E2State_Particles();
  virtual ~E2State_Particles();
  
  E2State_Particles(const E2State_Particles& from);
  
  inline E2State_Particles& operator=(const E2State_Particles& from) {
    CopyFrom(from);
    return *this;
  }
  
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }
  
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }
  
  static const ::google::protobuf::Descriptor* descriptor();
  static const E2State_Particles& default_instance();
  
  void Swap(E2State_Particles* other);
  
  // implements Message ----------------------------------------------
  
  E2State_Particles* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const E2State_Particles& from);
  void MergeFrom(const E2State_Particles& from);
  void Clear();
  bool IsInitialized() const;
  
  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  
  ::google::protobuf::Metadata GetMetadata() const;
  
  // nested types ----------------------------------------------------
  
  // accessors -------------------------------------------------------
  
  // required double ksi = 2;
  inline bool has_ksi() const;
  inline void clear_ksi();
  static const int kKsiFieldNumber = 2;
  inline double ksi() const;
  inline void set_ksi(double value);
  
  // required double v = 3;
  inline bool has_v() const;
  inline void clear_v();
  static const int kVFieldNumber = 3;
  inline double v() const;
  inline void set_v(double value);
  
  // @@protoc_insertion_point(class_scope:pb.E2State.Particles)
 private:
  inline void set_has_ksi();
  inline void clear_has_ksi();
  inline void set_has_v();
  inline void clear_has_v();
  
  ::google::protobuf::UnknownFieldSet _unknown_fields_;
  
  double ksi_;
  double v_;
  
  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(2 + 31) / 32];
  
  friend void  protobuf_AddDesc_model_5fe2_2eproto();
  friend void protobuf_AssignDesc_model_5fe2_2eproto();
  friend void protobuf_ShutdownFile_model_5fe2_2eproto();
  
  void InitAsDefaultInstance();
  static E2State_Particles* default_instance_;
};
// -------------------------------------------------------------------

class E2State : public ::google::protobuf::Message {
 public:
  E2State();
  virtual ~E2State();
  
  E2State(const E2State& from);
  
  inline E2State& operator=(const E2State& from) {
    CopyFrom(from);
    return *this;
  }
  
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }
  
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }
  
  static const ::google::protobuf::Descriptor* descriptor();
  static const E2State& default_instance();
  
  void Swap(E2State* other);
  
  // implements Message ----------------------------------------------
  
  E2State* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const E2State& from);
  void MergeFrom(const E2State& from);
  void Clear();
  bool IsInitialized() const;
  
  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  
  ::google::protobuf::Metadata GetMetadata() const;
  
  // nested types ----------------------------------------------------
  
  typedef E2State_Particles Particles;
  
  // accessors -------------------------------------------------------
  
  // repeated group Particles = 1 {
  inline int particles_size() const;
  inline void clear_particles();
  static const int kParticlesFieldNumber = 1;
  inline const ::pb::E2State_Particles& particles(int index) const;
  inline ::pb::E2State_Particles* mutable_particles(int index);
  inline ::pb::E2State_Particles* add_particles();
  inline const ::google::protobuf::RepeatedPtrField< ::pb::E2State_Particles >&
      particles() const;
  inline ::google::protobuf::RepeatedPtrField< ::pb::E2State_Particles >*
      mutable_particles();
  
  // @@protoc_insertion_point(class_scope:pb.E2State)
 private:
  
  ::google::protobuf::UnknownFieldSet _unknown_fields_;
  
  ::google::protobuf::RepeatedPtrField< ::pb::E2State_Particles > particles_;
  
  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(1 + 31) / 32];
  
  friend void  protobuf_AddDesc_model_5fe2_2eproto();
  friend void protobuf_AssignDesc_model_5fe2_2eproto();
  friend void protobuf_ShutdownFile_model_5fe2_2eproto();
  
  void InitAsDefaultInstance();
  static E2State* default_instance_;
};
// -------------------------------------------------------------------

class E2PetscSolverConfig : public ::google::protobuf::Message {
 public:
  E2PetscSolverConfig();
  virtual ~E2PetscSolverConfig();
  
  E2PetscSolverConfig(const E2PetscSolverConfig& from);
  
  inline E2PetscSolverConfig& operator=(const E2PetscSolverConfig& from) {
    CopyFrom(from);
    return *this;
  }
  
  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }
  
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }
  
  static const ::google::protobuf::Descriptor* descriptor();
  static const E2PetscSolverConfig& default_instance();
  
  void Swap(E2PetscSolverConfig* other);
  
  // implements Message ----------------------------------------------
  
  E2PetscSolverConfig* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const E2PetscSolverConfig& from);
  void MergeFrom(const E2PetscSolverConfig& from);
  void Clear();
  bool IsInitialized() const;
  
  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  
  ::google::protobuf::Metadata GetMetadata() const;
  
  // nested types ----------------------------------------------------
  
  // accessors -------------------------------------------------------
  
  // optional double tolerance = 1;
  inline bool has_tolerance() const;
  inline void clear_tolerance();
  static const int kToleranceFieldNumber = 1;
  inline double tolerance() const;
  inline void set_tolerance(double value);
  
  // optional double init_step = 2;
  inline bool has_init_step() const;
  inline void clear_init_step();
  static const int kInitStepFieldNumber = 2;
  inline double init_step() const;
  inline void set_init_step(double value);
  
  // @@protoc_insertion_point(class_scope:pb.E2PetscSolverConfig)
 private:
  inline void set_has_tolerance();
  inline void clear_has_tolerance();
  inline void set_has_init_step();
  inline void clear_has_init_step();
  
  ::google::protobuf::UnknownFieldSet _unknown_fields_;
  
  double tolerance_;
  double init_step_;
  
  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(2 + 31) / 32];
  
  friend void  protobuf_AddDesc_model_5fe2_2eproto();
  friend void protobuf_AssignDesc_model_5fe2_2eproto();
  friend void protobuf_ShutdownFile_model_5fe2_2eproto();
  
  void InitAsDefaultInstance();
  static E2PetscSolverConfig* default_instance_;
};
// ===================================================================


// ===================================================================

// E2Config

// required uint32 N = 1;
inline bool E2Config::has_n() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void E2Config::set_has_n() {
  _has_bits_[0] |= 0x00000001u;
}
inline void E2Config::clear_has_n() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void E2Config::clear_n() {
  n_ = 0u;
  clear_has_n();
}
inline ::google::protobuf::uint32 E2Config::n() const {
  return n_;
}
inline void E2Config::set_n(::google::protobuf::uint32 value) {
  set_has_n();
  n_ = value;
}

// optional double A = 2;
inline bool E2Config::has_a() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void E2Config::set_has_a() {
  _has_bits_[0] |= 0x00000002u;
}
inline void E2Config::clear_has_a() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void E2Config::clear_a() {
  a_ = 0;
  clear_has_a();
}
inline double E2Config::a() const {
  return a_;
}
inline void E2Config::set_a(double value) {
  set_has_a();
  a_ = value;
}

// optional double delta = 3;
inline bool E2Config::has_delta() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void E2Config::set_has_delta() {
  _has_bits_[0] |= 0x00000004u;
}
inline void E2Config::clear_has_delta() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void E2Config::clear_delta() {
  delta_ = 0;
  clear_has_delta();
}
inline double E2Config::delta() const {
  return delta_;
}
inline void E2Config::set_delta(double value) {
  set_has_delta();
  delta_ = value;
}

// optional double f = 4;
inline bool E2Config::has_f() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void E2Config::set_has_f() {
  _has_bits_[0] |= 0x00000008u;
}
inline void E2Config::clear_has_f() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void E2Config::clear_f() {
  f_ = 0;
  clear_has_f();
}
inline double E2Config::f() const {
  return f_;
}
inline void E2Config::set_f(double value) {
  set_has_f();
  f_ = value;
}

// -------------------------------------------------------------------

// E2State_Particles

// required double ksi = 2;
inline bool E2State_Particles::has_ksi() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void E2State_Particles::set_has_ksi() {
  _has_bits_[0] |= 0x00000001u;
}
inline void E2State_Particles::clear_has_ksi() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void E2State_Particles::clear_ksi() {
  ksi_ = 0;
  clear_has_ksi();
}
inline double E2State_Particles::ksi() const {
  return ksi_;
}
inline void E2State_Particles::set_ksi(double value) {
  set_has_ksi();
  ksi_ = value;
}

// required double v = 3;
inline bool E2State_Particles::has_v() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void E2State_Particles::set_has_v() {
  _has_bits_[0] |= 0x00000002u;
}
inline void E2State_Particles::clear_has_v() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void E2State_Particles::clear_v() {
  v_ = 0;
  clear_has_v();
}
inline double E2State_Particles::v() const {
  return v_;
}
inline void E2State_Particles::set_v(double value) {
  set_has_v();
  v_ = value;
}

// -------------------------------------------------------------------

// E2State

// repeated group Particles = 1 {
inline int E2State::particles_size() const {
  return particles_.size();
}
inline void E2State::clear_particles() {
  particles_.Clear();
}
inline const ::pb::E2State_Particles& E2State::particles(int index) const {
  return particles_.Get(index);
}
inline ::pb::E2State_Particles* E2State::mutable_particles(int index) {
  return particles_.Mutable(index);
}
inline ::pb::E2State_Particles* E2State::add_particles() {
  return particles_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::pb::E2State_Particles >&
E2State::particles() const {
  return particles_;
}
inline ::google::protobuf::RepeatedPtrField< ::pb::E2State_Particles >*
E2State::mutable_particles() {
  return &particles_;
}

// -------------------------------------------------------------------

// E2PetscSolverConfig

// optional double tolerance = 1;
inline bool E2PetscSolverConfig::has_tolerance() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void E2PetscSolverConfig::set_has_tolerance() {
  _has_bits_[0] |= 0x00000001u;
}
inline void E2PetscSolverConfig::clear_has_tolerance() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void E2PetscSolverConfig::clear_tolerance() {
  tolerance_ = 0;
  clear_has_tolerance();
}
inline double E2PetscSolverConfig::tolerance() const {
  return tolerance_;
}
inline void E2PetscSolverConfig::set_tolerance(double value) {
  set_has_tolerance();
  tolerance_ = value;
}

// optional double init_step = 2;
inline bool E2PetscSolverConfig::has_init_step() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void E2PetscSolverConfig::set_has_init_step() {
  _has_bits_[0] |= 0x00000002u;
}
inline void E2PetscSolverConfig::clear_has_init_step() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void E2PetscSolverConfig::clear_init_step() {
  init_step_ = 0;
  clear_has_init_step();
}
inline double E2PetscSolverConfig::init_step() const {
  return init_step_;
}
inline void E2PetscSolverConfig::set_init_step(double value) {
  set_has_init_step();
  init_step_ = value;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace pb

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_model_5fe2_2eproto__INCLUDED
