﻿#pragma once

#include <complex>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>

#include "clcxx_config.hpp"

namespace clcxx {

// supported types are  Primitive CFFI Types with
// void + complex  + struct + class
// POD structs are passed through libffi
// structs,class are passed as pointers with *new*
// (e.g. (:pointer :char))
// (array :int 10)
// for nonPOD struct, class, pointer

std::string class_name(std::type_index i);

extern "C" typedef union {
  float Float;
  double Double;
  // long double LongDouble;
} ComplexType;

extern "C" typedef struct {
  ComplexType real;
  ComplexType imag;
} LispComplex;

template <typename T>
struct IsPOD {
  static constexpr bool value =
      (std::is_trivial<T>::value && std::is_standard_layout<T>::value) &&
      (std::is_class<T>::value);
};

template <typename T>
struct IsFundamental {
  static constexpr bool value = std::is_fundamental<T>::value ||
                                (std::is_array<T>::value && IsPOD<T>::value);
};

template <typename T>
struct IsComplex {
  static constexpr bool value = std::is_same<T, std::complex<float>>::value ||
                                std::is_same<T, std::complex<double>>::value;
};

template <typename T>
struct IsString {
  static constexpr bool value = std::is_same<T, std::string>::value;
};

template <typename T>
struct IsClass {
  static constexpr bool value =
      !(std::is_same<T, std::string>::value ||
        std::is_same<T, std::complex<float>>::value ||
        std::is_same<T, std::complex<double>>::value) &&
      std::is_class<T>::value;
};

namespace detail {

template <bool, typename T1, typename T2>
struct StaticIf;

// non-bits
template <typename T1, typename T2>
struct StaticIf<false, T1, T2> {
  typedef T2 type;
};

// bits type
template <typename T1, typename T2>
struct StaticIf<true, T1, T2> {
  typedef T1 type;
};

template <typename T>
struct unused_type {};

template <typename T1, typename T2>
struct DefineIfDifferent {
  typedef T1 type;
};

template <typename T>
struct DefineIfDifferent<T, T> {
  typedef unused_type<T> type;
};

template <typename T1, typename T2>
using define_if_different = typename DefineIfDifferent<T1, T2>::type;

}  // namespace detail

/// Remove reference and const from a type
template <typename T>
using remove_const_ref =
    typename std::remove_const<typename std::remove_reference<T>::type>::type;

////
/// Convenience function to get the lisp data type associated with T
template <typename T>
struct static_type_mapping {
  typedef typename std::conditional<IsPOD<T>::value, T, void *>::type type;
  static std::string lisp_type() {
    static_assert(std::is_class<T>::value, "Unkown type");
    // if (!std::is_class<T>::value && !(std::is_same<T,std::string>::value)) {
    //   throw std::runtime_error("Type " + std::string(typeid(T).name()) +
    //                            " has no lisp wrapper");
    // }
    if (IsPOD<T>::value) {
      return std::string("(:struct " + class_name(std::type_index(typeid(T))) +
                         ")");
    }
    return std::string("(:class " + class_name(std::type_index(typeid(T))) +
                       ")");
  }
};

template <typename SourceT>
using dereferenced_type_mapping = static_type_mapping<SourceT>;
template <typename SourceT>
using mapped_lisp_type = typename dereferenced_type_mapping<SourceT>::type;

namespace detail {
template <typename T>
struct MappedReferenceType {
  // typedef typename std::remove_const<T>::type type;
  typedef T type;
};

// template <typename T>
// struct MappedReferenceType<T &> {
//   typedef T *type;
// };

// template <typename T>
// struct MappedReferenceType<const T &> {
//   typedef const T *type;
// };
}  // namespace detail

/// Remove reference and const from value types only, pass-through otherwise
template <typename T>
using mapped_reference_type = typename detail::MappedReferenceType<T>::type;

template <>
struct static_type_mapping<char> {
  typedef char type;
  static std::string lisp_type() { return ":char"; }
};
template <>
struct static_type_mapping<
    detail::define_if_different<unsigned char, uint8_t>> {
  typedef unsigned char type;
  static std::string lisp_type() { return ":uchar"; }
};
template <>
struct static_type_mapping<detail::define_if_different<short, int16_t>> {
  typedef short type;
  static std::string lisp_type() { return ":short"; }
};
template <>
struct static_type_mapping<
    detail::define_if_different<unsigned short, uint16_t>> {
  typedef unsigned short type;
  static std::string lisp_type() { return ":ushort"; }
};
template <>
struct static_type_mapping<detail::define_if_different<int, int32_t>> {
  typedef int type;
  static std::string lisp_type() { return ":int"; }
};
template <>
struct static_type_mapping<
    detail::define_if_different<unsigned int, uint32_t>> {
  typedef unsigned int type;
  static std::string lisp_type() { return ":uint"; }
};
template <>
struct static_type_mapping<detail::define_if_different<long, int64_t>> {
  typedef long type;
  static std::string lisp_type() { return ":long"; }
};
template <>
struct static_type_mapping<
    detail::define_if_different<unsigned long, uint64_t>> {
  typedef unsigned long type;
  static std::string lisp_type() { return ":ulong"; }
};
template <>
struct static_type_mapping<detail::define_if_different<long long, int64_t>> {
  typedef long long type;
  static std::string lisp_type() { return ":llong"; }
};
template <>
struct static_type_mapping<
    detail::define_if_different<unsigned long long, uint64_t>> {
  typedef unsigned long long type;
  static std::string lisp_type() { return ":ullong"; }
};
template <>
struct static_type_mapping<int8_t> {
  typedef int8_t type;
  static std::string lisp_type() { return ":int8"; }
};
template <>
struct static_type_mapping<uint8_t> {
  typedef uint8_t type;
  static std::string lisp_type() { return ":uint8"; }
};
template <>
struct static_type_mapping<int16_t> {
  typedef int16_t type;
  static std::string lisp_type() { return ":int16"; }
};
template <>
struct static_type_mapping<uint16_t> {
  typedef uint16_t type;
  static std::string lisp_type() { return ":uint16"; }
};
template <>
struct static_type_mapping<int32_t> {
  typedef int32_t type;
  static std::string lisp_type() { return ":int32"; }
};
template <>
struct static_type_mapping<uint32_t> {
  typedef uint32_t type;
  static std::string lisp_type() { return ":uint32"; }
};
template <>
struct static_type_mapping<int64_t> {
  typedef int64_t type;
  static std::string lisp_type() { return ":int64"; }
};
template <>
struct static_type_mapping<uint64_t> {
  typedef uint64_t type;
  static std::string lisp_type() { return ":uint64"; }
};
template <>
struct static_type_mapping<float> {
  typedef float type;
  static std::string lisp_type() { return ":float"; }
};
template <>
struct static_type_mapping<double> {
  typedef double type;
  static std::string lisp_type() { return ":double"; }
};
// template <> struct static_type_mapping<long double> {
//   typedef long double type;
//   static std::string lisp_type() { return ":long-double"; }
// };

template <typename T>
struct static_type_mapping<T &> {
  typedef void *type;
  static std::string lisp_type() {
    return std::string("(:reference " + static_type_mapping<T>::lisp_type() +
                       ")");
  }
};

template <typename T>
struct static_type_mapping<T *> {
  typedef void *type;
  static std::string lisp_type() {
    return std::string("(:pointer " + static_type_mapping<T>::lisp_type() +
                       ")");
  }
};
template <typename T, std::size_t N>
struct static_type_mapping<T (&)[N]> {
  typedef T type[N];
  static std::string lisp_type() {
    return std::string("(:array " + static_type_mapping<T>::lisp_type() + " " +
                       N + ")");
  }
};
template <typename T, std::size_t N>
struct static_type_mapping<T (*)[N]> {
  typedef T type[N];
  static std::string lisp_type() {
    return std::string("(:array " + static_type_mapping<T>::lisp_type() + " " +
                       N + ")");
  }
};

template <>
struct static_type_mapping<bool> {
  typedef bool type;
  static std::string lisp_type() { return ":bool"; }
};
template <>
struct static_type_mapping<const char *> {
  typedef const char *type;
  static std::string lisp_type() { return ":string"; }
};
template <>
struct static_type_mapping<std::string> {
  typedef const char *type;
  static std::string lisp_type() { return ":string"; }
};
template <>
struct static_type_mapping<void> {
  typedef void type;
  static std::string lisp_type() { return ":void"; }
};

template <>
struct static_type_mapping<std::complex<float>> {
  typedef LispComplex type;
  static std::string lisp_type() {
    return std::string(std::string("(:complex ") +
                       static_type_mapping<float>::lisp_type() + ")");
  }
};

template <>
struct static_type_mapping<std::complex<double>> {
  typedef LispComplex type;
  static std::string lisp_type() {
    return std::string(std::string("(:complex ") +
                       static_type_mapping<double>::lisp_type() + ")");
  }
};

template <typename T>
inline std::string lisp_type() {
  return static_type_mapping<T>::lisp_type();
}
// ------------------------------------------------------------------//
// Box an automatically converted value
/// Wrap a C++ pointer in a lisp type that contains a single void pointer field,
template <typename CppT, typename LispType = void *>
inline LispType box(CppT cpp_val) {
  return reinterpret_cast<void *>(cpp_val);
}

template <>
inline const char *box(const char *str) {
  return str;
}

template <>
inline LispComplex box(std::complex<float> x) {
  LispComplex c;
  c.real.Float = std::real(x);
  c.imag.Float = std::imag(x);
  return c;
}

template <>
inline LispComplex box(std::complex<double> x) {
  LispComplex c;
  c.real.Double = std::real(x);
  c.imag.Double = std::imag(x);
  return c;
}

// template <> inline LispComplex box(std::complex<long double> x) {
//   LispComplex c;
//   c.real.LongDouble = std::real(x);
//   c.imag.LongDouble = std::imag(x);
//   return c;
// }

// unbox -----------------------------------------------------------------//
/// pointers
template <typename CppT, typename LispT = void *>
CppT unbox(LispT lisp_val) {
  return reinterpret_cast<CppT>(lisp_val);
}

template <>
inline const char *unbox(const char *v) {
  return v;
}

template <>
inline std::complex<float> unbox(LispComplex v) {
  return std::complex<float>(v.real.Float, v.imag.Float);
}

template <>
inline std::complex<double> unbox(LispComplex v) {
  return std::complex<double>(v.real.Double, v.imag.Double);
}

// template <> inline std::complex<long double> unbox(LispComplex v) {
//   return std::complex<long double>(v.real.LongDouble, v.imag.LongDouble);
// }

/////////////////////////////////////

// Base template for converting to CPP
template <typename CppT, typename Enable = void>
struct ConvertToCpp {
  template <typename lispT>
  CppT *operator()(lispT &&) {
    static_assert(sizeof(CppT) == 0,
                  "No appropriate specialization for ConvertToCpp");
    return nullptr;
  }
};

// Fundamental type conversion
template <typename CppT>
struct ConvertToCpp<CppT,
                    typename std::enable_if<IsFundamental<CppT>::value>::type> {
  CppT operator()(CppT lisp_val) const { return lisp_val; }
};

// reference conversion
template <typename CppT>
struct ConvertToCpp<
    CppT, typename std::enable_if<std::is_reference<CppT>::value>::type> {
  using LispT = typename static_type_mapping<CppT>::type;
  CppT operator()(LispT lisp_val) const {
    auto obj_ptr =
        reinterpret_cast<typename std::remove_reference<CppT>::type *>(
            lisp_val);
    return *obj_ptr;
  }
};

// pointers conversion
template <typename CppT>
struct ConvertToCpp<
    CppT, typename std::enable_if<std::is_pointer<CppT>::value>::type> {
  using LispT = typename static_type_mapping<CppT>::type;
  CppT operator()(LispT lisp_val) const { return unbox<CppT, LispT>(lisp_val); }
};

// complex numbers types
template <typename CppT>
struct ConvertToCpp<CppT,
                    typename std::enable_if<IsComplex<CppT>::value>::type> {
  using LispT = typename static_type_mapping<CppT>::type;
  CppT operator()(LispT lisp_val) const { return unbox<CppT, LispT>(lisp_val); }
};

// strings
template <typename CppT>
struct ConvertToCpp<CppT,
                    typename std::enable_if<IsString<CppT>::value>::type> {
  std::string operator()(const char *str) const {
    return std::string(ConvertToCpp<const char *>()(str));
  }
};

// struct
template <typename CppT>
struct ConvertToCpp<CppT, typename std::enable_if<IsPOD<CppT>::value>::type> {
  CppT operator()(CppT pod_struct) const { return pod_struct; }
};

// class
template <typename CppT>
struct ConvertToCpp<CppT, typename std::enable_if<IsClass<CppT>::value>::type> {
  CppT operator()(void *class_ptr) const {
    auto cpp_class_ptr = reinterpret_cast<CppT *>(class_ptr);
    return *cpp_class_ptr;
  }
};

// Base template for converting To lisp
template <typename CppT, typename Enable = void>
struct ConvertToLisp {
  template <typename LispT>
  LispT *operator()(CppT &&) {
    static_assert(sizeof(CppT) == 0,
                  "No appropriate specialization for ConvertToCpp");
    return nullptr;
  }
};

// Fundamental type conversion
template <typename CppT>
struct ConvertToLisp<
    CppT, typename std::enable_if<IsFundamental<CppT>::value>::type> {
  CppT operator()(CppT cpp_val) const { return cpp_val; }
};

// Reference conversion
template <typename CppT>
struct ConvertToLisp<
    CppT, typename std::enable_if<std::is_reference<CppT>::value>::type> {
  using LispT = typename static_type_mapping<CppT>::type;
  LispT operator()(CppT cpp_val) const {
    return reinterpret_cast<LispT>(&cpp_val);
  }
};

// pointers conversion
template <typename CppT>
struct ConvertToLisp<
    CppT, typename std::enable_if<std::is_pointer<CppT>::value>::type> {
  using LispT = typename static_type_mapping<CppT>::type;
  LispT operator()(CppT cpp_val) const { return box<CppT, LispT>(cpp_val); }
};

// complex numbers types
template <typename CppT>
struct ConvertToLisp<CppT,
                     typename std::enable_if<IsComplex<CppT>::value>::type> {
  using LispT = typename static_type_mapping<CppT>::type;
  LispT operator()(CppT cpp_val) const { return box<CppT, LispT>(cpp_val); }
};

// Srings
template <typename CppT>
struct ConvertToLisp<CppT,
                     typename std::enable_if<IsString<CppT>::value>::type> {
  // struct ConvertToLisp<CppT, typename std::enable_if<
  //                                std::is_same<CppT,
  //                                std::string>::value>::type> {
  const char *operator()(const std::string &str) const {
    return ConvertToLisp<const char *>()(str.c_str());
  }
};

// template <typename CppT>
// struct ConvertToLisp<
//     CppT,
//     typename std::enable_if<std::is_same<CppT, std::string *>::value>::type>
//     {
//   const char *operator()(const std::string *str) const {
//     return ConvertToLisp<std::string>()(*str);
//   }
// };

// template <typename CppT>
// struct ConvertToLisp<CppT, typename std::enable_if<std::is_same<
//                                CppT, const std::string *>::value>::type> {
//   const char *operator()(const std::string *str) const {
//     return ConvertToLisp<std::string>()(*str);
//   }
// };

// struct
template <typename CppT>
struct ConvertToLisp<CppT, typename std::enable_if<IsPOD<CppT>::value>::type> {
  CppT operator()(CppT pod_struct) const { return pod_struct; }
};

// class exclude std::string
template <typename CppT>
struct ConvertToLisp<CppT,
                     typename std::enable_if<IsClass<CppT>::value>::type> {
  void *operator()(CppT cpp_class) const {
    auto class_ptr = new CppT(cpp_class);
    return reinterpret_cast<void *>(class_ptr);
  }
};

namespace detail {
template <typename T>
struct StrippedConversionType {
  typedef mapped_reference_type<T> type;
};

// template <typename T>
// struct StrippedConversionType<T *&> {
//   typedef T **type;
// };

// template <typename T>
// struct StrippedConversionType<T *const &> {
//   typedef const T **type;
// };
}  // namespace detail

template <typename T>
using lisp_converter_type =
    ConvertToLisp<typename detail::StrippedConversionType<T>::type>;

/// Conversion to the statically mapped target type.
template <typename T>
inline auto convert_to_lisp(T &&cpp_val)
    -> decltype(lisp_converter_type<T>()(std::forward<T>(cpp_val))) {
  return lisp_converter_type<T>()(std::forward<T>(cpp_val));
}

template <typename T>
using cpp_converter_type = ConvertToCpp<T>;

/// Conversion to C++
template <typename CppT, typename LispT>
inline CppT convert_to_cpp(const LispT &lisp_val) {
  return cpp_converter_type<CppT>()(lisp_val);
}

}  // namespace clcxx
