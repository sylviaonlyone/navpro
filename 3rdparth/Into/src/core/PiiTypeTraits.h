/* This file is part of Into.
 * Copyright (C) 2003-2011 Intopii.
 * All rights reserved.
 *
 * IMPORTANT LICENSING INFORMATION
 *
 * Into is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License version 3
 * as published by the Free Software Foundation.
 *
 * Into is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef _PIITYPETRAITS_H
#define _PIITYPETRAITS_H

#include "PiiMetaTemplate.h"

namespace std { template <class T> class complex; }

/// @file

namespace Pii
{
  /**
   * @name Static type checking
   *
   * This group contains structures that can be used for static type
   * checking in template meta-programming. The structures can be used
   * as checks for conditional structrures such as Pii::If to compile
   * different versions of code for different types.
   *
   * All @p IsXXXX structs implement a similar design pattern:
   *
   * @code
   * ASSERT(Pii::IsPointer<int>::boolValue == false);
   * ASSERT(Pii::IsPointer<int*>::boolValue == true);
   * ASSERT(Pii::IsArray<char[]>::boolValue == true);
   * // etc...
   * @endcode
   *
   * Static type checking is very useful in template meta-programs.
   *
   * @code
   * // Divide by two by shifting an integer one position to the right.
   * struct IntOp { static inline int divide(int value) { return value >> 1; } };
   * // Divide by two by multiplying by 0.5
   * struct FloatOp { static inline float divide(float value) { return value * 0.5; } };
   * // ...
   * template <class T> T calculate(T value)
   * {
   *   // This could be made much simpler with template specialization, but
   *   // it illustrates the idea anyway.
   *   return Pii::IfClass<Pii::IsInteger<T>,
   *                       IntOp, FloatOp>::Type::divide(value);
   * };
   * @endcode
   */
  //@{
  
  /**
   * An empty structure.
   */
  struct Empty {};

  /**
   * A structure whose nested type, @p Type, is @p T.
   */
  template <class T> struct Id { typedef T Type; };
  
  /**
   * A tester struct whose @p boolValue member evaluates statically to
   * @p true if the template parameter @p T is a pointer.
   */
  template <class T> struct IsPointer : False {};
  template <class T> struct IsPointer<T*> : True {};

  /**
   * A tester struct whose @p boolValue member evaluates statically to
   * @p true if the template parameter @p T is a reference.
   */
  template <class T> struct IsReference : False {};
  template <class T> struct IsReference<T&> : True {};

  /**
   * A tester struct whose @p boolValue member evaluates statically to
   * @p true if the template parameter @p T is an array.
   */
  template <class T> struct IsArray : False {};
  template <class T> struct IsArray<T[]> : True {};

  /**
   * A tester struct whose @p boolValue member evaluates statically to
   * @p true if the template parameter @p T is a signed integer type.
   */
  template <class T> struct IsSigned : False {};
  template <> struct IsSigned<char> : True {};
  template <> struct IsSigned<short> : True {};
  template <> struct IsSigned<int> : True {};
  template <> struct IsSigned<long> : True {};
  template <> struct IsSigned<long long> : True {};

  /**
   * A tester struct whose @p boolValue member evaluates statically to
   * @p true if the template parameter @p T is an unsigned integer
   * type.
   */
  template <class T> struct IsUnsigned : False {};
  template <> struct IsUnsigned<unsigned char> : True {};
  template <> struct IsUnsigned<unsigned short> : True {};
  template <> struct IsUnsigned<unsigned int> : True {};
  template <> struct IsUnsigned<unsigned long> : True {};
  template <> struct IsUnsigned<unsigned long long> : True {};

  /**
   * A tester struct whose @p boolValue member evaluates statically to
   * @p true if the template parameter @p T is an integer type (either
   * signed or unsigned).
   */
  template <class T> struct IsInteger : Or<IsSigned<T>::boolValue, IsUnsigned<T>::boolValue> {};

  /**
   * A tester struct whose @p boolValue member evaluates statically to
   * @p true if the template parameter @p T is a floating point number
   * type.
   */
  template <class T> struct IsFloatingPoint : False {};
  template <> struct IsFloatingPoint<float> : True {};
  template <> struct IsFloatingPoint<double> : True {};
  template <> struct IsFloatingPoint<long double> : True {};

  /**
   * A tester struct whose @p boolValue member evaluates statically to
   * @p true if the template parameter @p T is a numeric primitive
   * (non-bool) type.
   */
  template <class T> struct IsNumeric : Or<IsSigned<T>::boolValue,
                                           IsUnsigned<T>::boolValue,
                                           IsFloatingPoint<T>::boolValue>
  {};
  
  /**
   * A tester struct whose @p boolValue member evaluates statically to
   * @p true if the template parameter @p T is a primitive type.
   */
  template <class T> struct IsPrimitive : IsNumeric<T> {};
  template <> struct IsPrimitive<bool> : True {};

  /**
   * A structure that has a nested type @p Type if and only if @p T is
   * a primitive type. This can be exploited as a <em>substitution
   * failure is not an error</em> (SFINAE) trick to guide overload
   * resolution.
   *
   * @code
   * // I want this function to be called for primitive types only
   * template <class T> void func(T value, typename Pii::OnlyPrimitive<T>::Type = 0)
   * {}
   *
   * // This will be called for Base and all derived types.
   * void func(const Base& base)
   * {}
   *
   * // Derived inherits Base
   * Derived d;
   * func(d); // calls func(const Base&)
   * @endcode
   *
   * If you left the second dummy parameter out, the template function
   * would be called for types derived from Base, as the template type
   * @p T is always an exact match. Since
   * <tt>Pii::OnlyPrimitive<T>::Type</tt> only exists if @p T is a
   * primitive type, the first function causes a substitution failure
   * for all other types, and will be left out of oveload resolution.
   */
  template <class T> struct OnlyPrimitive : IfClass<IsPrimitive<T>, Id<T>, Empty>::Type {};

  /**
   * A structure that has a nested type @p Type if and only if @p T is
   * a floating-point type.
   *
   * @see OnlyPrimitive
   */
  template <class T> struct OnlyFloatingPoint : IfClass<IsFloatingPoint<T>, Id<T>, Empty>::Type {};

  /**
   * A structure that has a nested type @p Type if and only if @p T is
   * a numeric primitive type.
   *
   * @see OnlyPrimitive
   */
  template <class T> struct OnlyNumeric : IfClass<IsNumeric<T>, Id<T>, Empty>::Type {};

  /**
   * A tester struct whose @p boolValue member evaluates statically to
   * @p true if the template parameter @p T is a complex number
   * (std::complex).
   */
  template <class T> struct IsComplex : False {};
  template <class T> struct IsComplex<std::complex<T> > : True {};

  /**
   * A structure that has a nested type @p Type if and only if @p T is
   * a numeric primitive type.
   *
   * @see Primitive
   */
  template <class T> struct OnlyComplex : IfClass<IsComplex<T>, Id<T>, Empty>::Type {};

  /**
   * A tester struct whose @p boolValue member evaluates statically to
   * @p true if the template parameter @p T is a numeric primitive
   * (non-bool) type or a complex number (std::complex).
   */
  template <class T> struct IsNumericOrComplex : Or<IsNumeric<T>::boolValue,
                                                    IsComplex<T>::boolValue>
  {};
  
  /**
   * A structure that has a nested type @p Type if and only if @p T is
   * a numeric primitive type or a complex number.
   *
   * @see Primitive
   */
  template <class T> struct OnlyNumericOrComplex : IfClass<IsNumericOrComplex<T>, Id<T>, Empty>::Type {};

  /**
   * A tester struct whose @p boolValue member evaluates statically to
   * @p true if the template parameter @p T is a @p const type.
   */
  template <class T> struct IsConst : False {};
  template <class T> struct IsConst<const T> : True {};

  /**
   * A tester struct whose @p boolValue member evaluates statically to
   * @p true if the template parameters @p T and @p U are the same
   * type.
   */
  template <class T, class U> struct IsSame : False {};
  template <class T> struct IsSame<T,T> : True {};
  
  /**
   * A structure whose @p boolValue member evaluates statically to @p
   * true if (and only if) @p Base is a base class of @p Derived. Note
   * that instances of @p Derived are instances of @p Base also if @p
   * Base and @p Derived are the same type. Therefore, this trait will
   * be @p true if @p Base and @p Derived are the same type.
   */
  template <class Base, class Derived> struct IsBaseOf
  {
    /// @cond null
    typedef char (&Yes)[1];
    typedef char (&No)[2];
    
    template <class T> static Yes isBaseOf(Derived*, T);
    static No isBaseOf(Base*, int);

    struct Converter
    {
      operator Base*() const;
      operator Derived*();
    };

    // New GCC and MSVC versions provide __is_base_of() operator as an
    // extension, but it is unfortunately not available everywhere. 
    // Thus, we need to resort to this ugly hack. The explanation is
    // too long to be written here. See
    // http://stackoverflow.com/questions/2910979/how-is-base-of-works
    enum { boolValue = sizeof(isBaseOf(Converter(), int(0))) == sizeof(Yes) ||
           IsSame<Base,Derived>::boolValue };
    /// @endcond
  };

  /**
   * A structure whose member type @p Type is a non-const version of
   * the template parameter @p T.
   */
  template <class T> struct ToNonConst { typedef T Type; };    
  template <class T> struct ToNonConst<const T> { typedef T Type; };
  
  /**
   * A static functor that converts pointers to references and keeps
   * references as such.
   *
   * @code
   * int i = 1, j;
   * // Both of these set j to 1
   * j = ToReference<int>::apply(i);
   * j = ToReference<int*>::apply(&i);
   * @endcode
   *
   * The @p Type member of this class evaluates to the non-pointer
   * type. That is, ToReference<T>::Type is @p T&, and so is
   * ToReference<T*>::Type.
   */
  template <class T> struct ToReference
  {
    typedef T& Type;
    static T& apply(T& value) { return value; }
  };

  template <class T> struct ToReference<T*>
  {
    typedef T& Type;
    static T& apply(T* value) { return *value; }
  };

  /**
   * A static functor that converts references to pointers and keeps
   * pointers as such.
   *
   * @see ToReference
   */
  template <class T> struct ToPointer
  {
    typedef T* Type;
    static T* apply(T& value) { return &value; }
  };

  template <class T> struct ToPointer<T*>
  {
    typedef T* Type;
    static T* apply(T* value) { return value; }
  };

  /**
   * A static functor that converts references to values and keeps
   * other types as such.
   *
   * @see ToReference
   */
  template <class T> struct RefToValue
  {
    typedef T Type;
    static T apply(T& value) { return value; }
  };
  
  template <class T> struct RefToValue<T&>
  {
    typedef typename ToNonConst<T>::Type Type;
    static Type apply(T& value) { return value; }
  };

  /**
   * A static functor that converts pointers to values and keeps other
   * types as such.
   *
   * @see ToReference
   */
  template <class T> struct PtrToValue
  {
    typedef T Type;
    static T apply(T& value) { return value; }
  };

  template <class T> struct PtrToValue<T*>
  {
    typedef typename ToNonConst<T>::Type Type;
    static Type apply(T* value) { return *value; }
  };

  /**
   * Converts a type to a corresponding floating-point type. The @p
   * Type member of this structure is @p float for all primitive types
   * except @p double and @p long @p double, for which no conversion
   * will be made. Composed types such as PiiMatrix should specialize
   * this structure:
   *
   * @code
   * namespace Pii
   * {
   *   template <class T> struct ToFloatingPoint<PiiMatrix<T> >
   *   {
   *     typedef PiiMatrix<typename ToFloatingPoint<T>::Type> Type;
   *     typedef typename ToFloatingPoint<T>::Type PrimitiveType;
   *   };
   * }
   * @endcode
   */
  template <class T> struct ToFloatingPoint
  {
    /**
     * Primitive types are converted to @p floats by default.
     */
    typedef float Type;
    /**
     * Primitive types are primitive, so the @p PrimitiveType equals
     * @p Type.
     */
    typedef float PrimitiveType;
  };

  /**
   * Specialization for @p double.
   */
  template <> struct ToFloatingPoint<double>
  {
    typedef double Type;
    typedef double PrimitiveType;
  };

  /**
   * Specialization for @p long @p double.
   */
  template <> struct ToFloatingPoint<long double>
  {
    typedef long double Type;
    typedef long double PrimitiveType;
  };

  template <class T> struct ToFloatingPoint<std::complex<T> >
  {
    typedef std::complex<typename ToFloatingPoint<T>::Type> Type;
    typedef typename ToFloatingPoint<T>::Type PrimitiveType;
  };

  /**
   * Type for values passed through the va_arg mechanism ("..." in
   * function parameters). When passing through va_args, floats are
   * promoted to doubles, (unsigned) chars/shorts and bools to ints.
   */
  template <class T> struct VaArg { typedef T Type; };
  template <> struct VaArg<float> { typedef double Type; };
  template <> struct VaArg<char> { typedef int Type; };
  template <> struct VaArg<unsigned char> { typedef int Type; };
  template <> struct VaArg<bool> { typedef int Type; };
  template <> struct VaArg<short> { typedef int Type; };
  template <> struct VaArg<unsigned short> { typedef int Type; };

  // PENDING VaArg<enum> -> int
  // Requires an IsEnum trait.
  
  /**
   * The @p Type member typedef of this structure maps to the @p
   * value_type member type of the type given as the template
   * argument. The structure is specialized for pointers to return the
   * type pointed to.
   *
   * @code
   * using namespace Pii;
   * typedef ValueType<PiiMatrix<double>::column_iterator>::Type DoubleType1;
   * typedef ValueType<PiiMatrix<double>::row_iterator>::Type DoubleType2; 
   * @endcode
   */
  template <class T> struct ValueType { typedef typename T::value_type Type; };
  template <class T> struct ValueType<T*> { typedef T Type; };
  
  /**
   * A trait whose @p boolValue member is @p true if type @p T has
   * less bits than type @p U.
   */
  template <class T, class U> struct LessBits : public If<sizeof(T) < sizeof(U),
                                                               True,
                                                               False>::Type
  {};
  
  /**
   * A trait whose @p boolValue member is @p true if type @p T has
   * more bits than type @p U.
   */
  template <class T, class U> struct MoreBits : public If<sizeof(T) <= sizeof(U),
                                                               False,
                                                               True>::Type
  {};

  /**
   * A trait whose @p Type is @p U if @p sizeof(T) is less than @p
   * sizeof(U) and @p T otherwise.
   */
  template <class T, class U> struct Larger { typedef typename IfClass<LessBits<T,U>, U, T>::Type Type; };
  
  /**
   * A trait whose @p Type is @p T if @p sizeof(T) is larger than @p
   * sizeof(U) and @p T otherwise.
   */
  template <class T, class U> struct Smaller { typedef typename IfClass<MoreBits<T,U>, U, T>::Type Type; };

  /**
   * Convert any integer type to its signed counterpart. For example,
   * ToSigned<unsigned int>::Type is @p int.
   */
  template <class T> struct ToSigned { typedef T Type; };
  template <> struct ToSigned<unsigned char> { typedef char Type; };
  template <> struct ToSigned<unsigned short> { typedef short Type; };
  template <> struct ToSigned<unsigned int> { typedef int Type; };
  template <> struct ToSigned<unsigned long> { typedef long Type; };
  template <> struct ToSigned<unsigned long long> { typedef long long Type; };
  
  /**
   * Convert any integer type to its unsigned counterpart. For
   * example, ToSigned<char>::Type is @p unsigned @p char.
   */
  template <class T> struct ToUnsigned { typedef T Type; };
  template <> struct ToUnsigned<char> { typedef unsigned char Type; };
  template <> struct ToUnsigned<short> { typedef unsigned short Type; };
  template <> struct ToUnsigned<int> { typedef unsigned int Type; };
  template <> struct ToUnsigned<long> { typedef unsigned long Type; };
  template <> struct ToUnsigned<long long> { typedef unsigned long long Type; };

  /**
   * Combine two integer types.
   *
   * @see Combine
   */
  template <class T, class U> struct CombineInts
  {
    // Check if either is signed.
    typedef typename IfClass<Or<IsSigned<T>::boolValue,
                                IsSigned<U>::boolValue>,
                             // At least one signed -> use a signed version
                             // of the type with more bits
                             typename ToSigned<typename Larger<T,U>::Type>::Type,
                             // Both are unsigned -> use larger as such
                             typename Larger<T,U>::Type >::Type Type;
  };

  /**
   * Combine two types, at least one of which is a floating-point number.
   *
   * @see Combine
   */
  template <class T, class U> struct CombineFloats
  {
    // Check if both are floats
    typedef typename IfClass<And<IsFloatingPoint<T>::boolValue,
                                 IsFloatingPoint<U>::boolValue>,
                             // Yes, use the larger
                             typename Larger<T,U>::Type,
                             // No, only one is float -> use it
                             typename IfClass<IsFloatingPoint<T>, T, U>::Type >::Type Type;
  };

  /**
   * A trait that can be used to find a type that best combines the
   * properties of two types. This trait can be used when mixing
   * different types in calculations to ensure the result type will be
   * accurate enough.
   *
   * The logic is as follows:
   *
   * @li If both types are floating-point, the result is the type with
   * larger number of bits. For example (@p double, @p float) maps to
   * @p double.
   *
   * @li If one of the types is floating-point and the other is
   * integer, the floating-point type will be used. For example (@p
   * float, @p int) maps to @p float.
   *
   * @li If both types are integers, and either of them is signed, the
   * result is a signed version of the type with larger number of
   * bits. For example (@p unsigned @p long, @p char) maps to @p long.
   *
   * @li If both types are unsigned integers, the result is the type
   * with larger number of bits. For example (@p unsigned @p int, @p
   * unsigned @p char) maps to @p unsigned @p int.
   *
   * @code
   * template <class T, class U> void calculate(T a, U b)
   * {
   *   typedef typename Pii::Combine<T,U>::Type R;
   *   R result = R(a) * R(b);
   *   std::cout << result << std::endl;
   * }
   *
   * // ...
   *
   * char a = -5;
   * unsigned int b = 2;
   * calculate(a,b);
   * // Outputs -10.
   * // If you directly multiply a*b, you'll get 4294967286
   * // as an unsigned int
   * @endcode
   */
  template <class T, class U> struct Combine
  {
    // Check if both are integer types
    typedef typename IfClass<And<IsInteger<T>::boolValue,
                                 IsInteger<U>::boolValue>,
                             // "Call" CombineInts
                             typename CombineInts<T,U>::Type,
                             // "Call" CombineFloats
                             typename CombineFloats<T,U>::Type >::Type Type;
  };

  //@}
}

#endif //_PIITYPETRAITS_H
