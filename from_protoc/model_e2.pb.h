// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: model_e2.proto

#ifndef PROTOBUF_model_5fe2_2eproto__INCLUDED
#define PROTOBUF_model_5fe2_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2006000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
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
class E2Model;

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

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  double a_;
  double delta_;
  double f_;
  ::google::protobuf::uint32 n_;
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

  // required double ksi = 3;
  inline bool has_ksi() const;
  inline void clear_ksi();
  static const int kKsiFieldNumber = 3;
  inline double ksi() const;
  inline void set_ksi(double value);

  // required double v = 4;
  inline bool has_v() const;
  inline void clear_v();
  static const int kVFieldNumber = 4;
  inline double v() const;
  inline void set_v(double value);

  // @@protoc_insertion_point(class_scope:pb.E2State.Particles)
 private:
  inline void set_has_ksi();
  inline void clear_has_ksi();
  inline void set_has_v();
  inline void clear_has_v();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  double ksi_;
  double v_;
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

  // optional bool simulated = 1;
  inline bool has_simulated() const;
  inline void clear_simulated();
  static const int kSimulatedFieldNumber = 1;
  inline bool simulated() const;
  inline void set_simulated(bool value);

  // repeated group Particles = 2 {
  inline int particles_size() const;
  inline void clear_particles();
  static const int kParticlesFieldNumber = 2;
  inline const ::pb::E2State_Particles& particles(int index) const;
  inline ::pb::E2State_Particles* mutable_particles(int index);
  inline ::pb::E2State_Particles* add_particles();
  inline const ::google::protobuf::RepeatedPtrField< ::pb::E2State_Particles >&
      particles() const;
  inline ::google::protobuf::RepeatedPtrField< ::pb::E2State_Particles >*
      mutable_particles();

  // @@protoc_insertion_point(class_scope:pb.E2State)
 private:
  inline void set_has_simulated();
  inline void clear_has_simulated();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::google::protobuf::RepeatedPtrField< ::pb::E2State_Particles > particles_;
  bool simulated_;
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

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  double tolerance_;
  double init_step_;
  friend void  protobuf_AddDesc_model_5fe2_2eproto();
  friend void protobuf_AssignDesc_model_5fe2_2eproto();
  friend void protobuf_ShutdownFile_model_5fe2_2eproto();

  void InitAsDefaultInstance();
  static E2PetscSolverConfig* default_instance_;
};
// -------------------------------------------------------------------

class E2Model : public ::google::protobuf::Message {
 public:
  E2Model();
  virtual ~E2Model();

  E2Model(const E2Model& from);

  inline E2Model& operator=(const E2Model& from) {
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
  static const E2Model& default_instance();

  void Swap(E2Model* other);

  // implements Message ----------------------------------------------

  E2Model* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const E2Model& from);
  void MergeFrom(const E2Model& from);
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

  // required double time = 4;
  inline bool has_time() const;
  inline void clear_time();
  static const int kTimeFieldNumber = 4;
  inline double time() const;
  inline void set_time(double value);

  // required uint32 steps = 5;
  inline bool has_steps() const;
  inline void clear_steps();
  static const int kStepsFieldNumber = 5;
  inline ::google::protobuf::uint32 steps() const;
  inline void set_steps(::google::protobuf::uint32 value);

  // required .pb.E2Config pconfig = 1;
  inline bool has_pconfig() const;
  inline void clear_pconfig();
  static const int kPconfigFieldNumber = 1;
  inline const ::pb::E2Config& pconfig() const;
  inline ::pb::E2Config* mutable_pconfig();
  inline ::pb::E2Config* release_pconfig();
  inline void set_allocated_pconfig(::pb::E2Config* pconfig);

  // required .pb.E2State state = 2;
  inline bool has_state() const;
  inline void clear_state();
  static const int kStateFieldNumber = 2;
  inline const ::pb::E2State& state() const;
  inline ::pb::E2State* mutable_state();
  inline ::pb::E2State* release_state();
  inline void set_allocated_state(::pb::E2State* state);

  // required .pb.E2PetscSolverConfig sconfig = 3;
  inline bool has_sconfig() const;
  inline void clear_sconfig();
  static const int kSconfigFieldNumber = 3;
  inline const ::pb::E2PetscSolverConfig& sconfig() const;
  inline ::pb::E2PetscSolverConfig* mutable_sconfig();
  inline ::pb::E2PetscSolverConfig* release_sconfig();
  inline void set_allocated_sconfig(::pb::E2PetscSolverConfig* sconfig);

  // @@protoc_insertion_point(class_scope:pb.E2Model)
 private:
  inline void set_has_time();
  inline void clear_has_time();
  inline void set_has_steps();
  inline void clear_has_steps();
  inline void set_has_pconfig();
  inline void clear_has_pconfig();
  inline void set_has_state();
  inline void clear_has_state();
  inline void set_has_sconfig();
  inline void clear_has_sconfig();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  double time_;
  ::pb::E2Config* pconfig_;
  ::pb::E2State* state_;
  ::pb::E2PetscSolverConfig* sconfig_;
  ::google::protobuf::uint32 steps_;
  friend void  protobuf_AddDesc_model_5fe2_2eproto();
  friend void protobuf_AssignDesc_model_5fe2_2eproto();
  friend void protobuf_ShutdownFile_model_5fe2_2eproto();

  void InitAsDefaultInstance();
  static E2Model* default_instance_;
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
  // @@protoc_insertion_point(field_get:pb.E2Config.N)
  return n_;
}
inline void E2Config::set_n(::google::protobuf::uint32 value) {
  set_has_n();
  n_ = value;
  // @@protoc_insertion_point(field_set:pb.E2Config.N)
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
  // @@protoc_insertion_point(field_get:pb.E2Config.A)
  return a_;
}
inline void E2Config::set_a(double value) {
  set_has_a();
  a_ = value;
  // @@protoc_insertion_point(field_set:pb.E2Config.A)
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
  // @@protoc_insertion_point(field_get:pb.E2Config.delta)
  return delta_;
}
inline void E2Config::set_delta(double value) {
  set_has_delta();
  delta_ = value;
  // @@protoc_insertion_point(field_set:pb.E2Config.delta)
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
  // @@protoc_insertion_point(field_get:pb.E2Config.f)
  return f_;
}
inline void E2Config::set_f(double value) {
  set_has_f();
  f_ = value;
  // @@protoc_insertion_point(field_set:pb.E2Config.f)
}

// -------------------------------------------------------------------

// E2State_Particles

// required double ksi = 3;
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
  // @@protoc_insertion_point(field_get:pb.E2State.Particles.ksi)
  return ksi_;
}
inline void E2State_Particles::set_ksi(double value) {
  set_has_ksi();
  ksi_ = value;
  // @@protoc_insertion_point(field_set:pb.E2State.Particles.ksi)
}

// required double v = 4;
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
  // @@protoc_insertion_point(field_get:pb.E2State.Particles.v)
  return v_;
}
inline void E2State_Particles::set_v(double value) {
  set_has_v();
  v_ = value;
  // @@protoc_insertion_point(field_set:pb.E2State.Particles.v)
}

// -------------------------------------------------------------------

// E2State

// optional bool simulated = 1;
inline bool E2State::has_simulated() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void E2State::set_has_simulated() {
  _has_bits_[0] |= 0x00000001u;
}
inline void E2State::clear_has_simulated() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void E2State::clear_simulated() {
  simulated_ = false;
  clear_has_simulated();
}
inline bool E2State::simulated() const {
  // @@protoc_insertion_point(field_get:pb.E2State.simulated)
  return simulated_;
}
inline void E2State::set_simulated(bool value) {
  set_has_simulated();
  simulated_ = value;
  // @@protoc_insertion_point(field_set:pb.E2State.simulated)
}

// repeated group Particles = 2 {
inline int E2State::particles_size() const {
  return particles_.size();
}
inline void E2State::clear_particles() {
  particles_.Clear();
}
inline const ::pb::E2State_Particles& E2State::particles(int index) const {
  // @@protoc_insertion_point(field_get:pb.E2State.particles)
  return particles_.Get(index);
}
inline ::pb::E2State_Particles* E2State::mutable_particles(int index) {
  // @@protoc_insertion_point(field_mutable:pb.E2State.particles)
  return particles_.Mutable(index);
}
inline ::pb::E2State_Particles* E2State::add_particles() {
  // @@protoc_insertion_point(field_add:pb.E2State.particles)
  return particles_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::pb::E2State_Particles >&
E2State::particles() const {
  // @@protoc_insertion_point(field_list:pb.E2State.particles)
  return particles_;
}
inline ::google::protobuf::RepeatedPtrField< ::pb::E2State_Particles >*
E2State::mutable_particles() {
  // @@protoc_insertion_point(field_mutable_list:pb.E2State.particles)
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
  // @@protoc_insertion_point(field_get:pb.E2PetscSolverConfig.tolerance)
  return tolerance_;
}
inline void E2PetscSolverConfig::set_tolerance(double value) {
  set_has_tolerance();
  tolerance_ = value;
  // @@protoc_insertion_point(field_set:pb.E2PetscSolverConfig.tolerance)
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
  // @@protoc_insertion_point(field_get:pb.E2PetscSolverConfig.init_step)
  return init_step_;
}
inline void E2PetscSolverConfig::set_init_step(double value) {
  set_has_init_step();
  init_step_ = value;
  // @@protoc_insertion_point(field_set:pb.E2PetscSolverConfig.init_step)
}

// -------------------------------------------------------------------

// E2Model

// required double time = 4;
inline bool E2Model::has_time() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void E2Model::set_has_time() {
  _has_bits_[0] |= 0x00000001u;
}
inline void E2Model::clear_has_time() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void E2Model::clear_time() {
  time_ = 0;
  clear_has_time();
}
inline double E2Model::time() const {
  // @@protoc_insertion_point(field_get:pb.E2Model.time)
  return time_;
}
inline void E2Model::set_time(double value) {
  set_has_time();
  time_ = value;
  // @@protoc_insertion_point(field_set:pb.E2Model.time)
}

// required uint32 steps = 5;
inline bool E2Model::has_steps() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void E2Model::set_has_steps() {
  _has_bits_[0] |= 0x00000002u;
}
inline void E2Model::clear_has_steps() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void E2Model::clear_steps() {
  steps_ = 0u;
  clear_has_steps();
}
inline ::google::protobuf::uint32 E2Model::steps() const {
  // @@protoc_insertion_point(field_get:pb.E2Model.steps)
  return steps_;
}
inline void E2Model::set_steps(::google::protobuf::uint32 value) {
  set_has_steps();
  steps_ = value;
  // @@protoc_insertion_point(field_set:pb.E2Model.steps)
}

// required .pb.E2Config pconfig = 1;
inline bool E2Model::has_pconfig() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void E2Model::set_has_pconfig() {
  _has_bits_[0] |= 0x00000004u;
}
inline void E2Model::clear_has_pconfig() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void E2Model::clear_pconfig() {
  if (pconfig_ != NULL) pconfig_->::pb::E2Config::Clear();
  clear_has_pconfig();
}
inline const ::pb::E2Config& E2Model::pconfig() const {
  // @@protoc_insertion_point(field_get:pb.E2Model.pconfig)
  return pconfig_ != NULL ? *pconfig_ : *default_instance_->pconfig_;
}
inline ::pb::E2Config* E2Model::mutable_pconfig() {
  set_has_pconfig();
  if (pconfig_ == NULL) pconfig_ = new ::pb::E2Config;
  // @@protoc_insertion_point(field_mutable:pb.E2Model.pconfig)
  return pconfig_;
}
inline ::pb::E2Config* E2Model::release_pconfig() {
  clear_has_pconfig();
  ::pb::E2Config* temp = pconfig_;
  pconfig_ = NULL;
  return temp;
}
inline void E2Model::set_allocated_pconfig(::pb::E2Config* pconfig) {
  delete pconfig_;
  pconfig_ = pconfig;
  if (pconfig) {
    set_has_pconfig();
  } else {
    clear_has_pconfig();
  }
  // @@protoc_insertion_point(field_set_allocated:pb.E2Model.pconfig)
}

// required .pb.E2State state = 2;
inline bool E2Model::has_state() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void E2Model::set_has_state() {
  _has_bits_[0] |= 0x00000008u;
}
inline void E2Model::clear_has_state() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void E2Model::clear_state() {
  if (state_ != NULL) state_->::pb::E2State::Clear();
  clear_has_state();
}
inline const ::pb::E2State& E2Model::state() const {
  // @@protoc_insertion_point(field_get:pb.E2Model.state)
  return state_ != NULL ? *state_ : *default_instance_->state_;
}
inline ::pb::E2State* E2Model::mutable_state() {
  set_has_state();
  if (state_ == NULL) state_ = new ::pb::E2State;
  // @@protoc_insertion_point(field_mutable:pb.E2Model.state)
  return state_;
}
inline ::pb::E2State* E2Model::release_state() {
  clear_has_state();
  ::pb::E2State* temp = state_;
  state_ = NULL;
  return temp;
}
inline void E2Model::set_allocated_state(::pb::E2State* state) {
  delete state_;
  state_ = state;
  if (state) {
    set_has_state();
  } else {
    clear_has_state();
  }
  // @@protoc_insertion_point(field_set_allocated:pb.E2Model.state)
}

// required .pb.E2PetscSolverConfig sconfig = 3;
inline bool E2Model::has_sconfig() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
inline void E2Model::set_has_sconfig() {
  _has_bits_[0] |= 0x00000010u;
}
inline void E2Model::clear_has_sconfig() {
  _has_bits_[0] &= ~0x00000010u;
}
inline void E2Model::clear_sconfig() {
  if (sconfig_ != NULL) sconfig_->::pb::E2PetscSolverConfig::Clear();
  clear_has_sconfig();
}
inline const ::pb::E2PetscSolverConfig& E2Model::sconfig() const {
  // @@protoc_insertion_point(field_get:pb.E2Model.sconfig)
  return sconfig_ != NULL ? *sconfig_ : *default_instance_->sconfig_;
}
inline ::pb::E2PetscSolverConfig* E2Model::mutable_sconfig() {
  set_has_sconfig();
  if (sconfig_ == NULL) sconfig_ = new ::pb::E2PetscSolverConfig;
  // @@protoc_insertion_point(field_mutable:pb.E2Model.sconfig)
  return sconfig_;
}
inline ::pb::E2PetscSolverConfig* E2Model::release_sconfig() {
  clear_has_sconfig();
  ::pb::E2PetscSolverConfig* temp = sconfig_;
  sconfig_ = NULL;
  return temp;
}
inline void E2Model::set_allocated_sconfig(::pb::E2PetscSolverConfig* sconfig) {
  delete sconfig_;
  sconfig_ = sconfig;
  if (sconfig) {
    set_has_sconfig();
  } else {
    clear_has_sconfig();
  }
  // @@protoc_insertion_point(field_set_allocated:pb.E2Model.sconfig)
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
