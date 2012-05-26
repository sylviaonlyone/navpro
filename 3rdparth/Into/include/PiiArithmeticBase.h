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

#ifndef _PIIARITHMETICBASE_H
#define _PIIARITHMETICBASE_H

#include <PiiFunctional.h>
#include <functional>

/**
 * A template that adds support for arithmetic operators for any data
 * structure that can be iterated over. This class is intended to be
 * used with the CRTP (curiously recurring template pattern). The
 * derived class must have have stl-style @p begin() and @p end()
 * functions for iterators. Information about the element type itself
 * is provided through the helper type @p Traits. The required type
 * definitions are shown in the example below.
 *
 * @code
 * // Forward declaration for the rebinder in MyTraits.
 * template <class T> class MyClass;
 *
 * template <class T> struct MyTraits
 * {
 *   // The element type
 *   typedef T Type;
 *   // Iterator type
 *   typedef T* Iterator;
 *   // Const iterator type
 *   typedef const T* ConstIterator;
 *   // Rebinder creates a MyClass types with varying content types.
 *   template <class U> struct Rebind { typedef MyClass<U> Type; };
 * };
 *
 * template <class T> class MyClass : public PiiArithmeticBase<MyClass<T>, MyTraits<T> >
 * {
 * public:
 *   typedef PiiArithmeticBase<MyClass<T>, MyTraits<T> > BaseType;
 *
 *   // Required iterator functions
 *   typename BaseType::ConstIterator begin() const { return _pData; }   // const iterator to the beginning
 *   typename BaseType::Iterator begin() { return _pData; }              // iterator to the beginning
 *   typename BaseType::ConstIterator end() const { return _pData + 5; } // const iterator to the end
 *   typename BaseType::Iterator end() { return _pData + 5; }            // const iterator to the end
 *
 *   // The following aren't necessarily required if your class is a POD type
 *
 *   // Standard onstructors (implementations may vary)
 *   MyClass() { BaseType::operator=(0); }
 *   MyClass(const MyClass& other) { *this = other; }
 *
 *   // Copy assignment isn't inherited...
 *   MyClass& operator= (const MyClass& other) { return BaseType::operator=(other); }
 *
 * private:
 *   T _pData[5];
 * };
 *
 * // Now, you can do this:
 * MyClass a, b;
 * MyClass c = a + b;
 * @endcode
 *
 * @note The length of the vector/array is assumed to be fixed. That
 * is, all instances of the derived class should hold the same amount
 * of elements. If this is not the case, your program will (in the
 * best case) crash.
 *
 * @ingroup Core
 */
template <class Derived, class Traits> class PiiArithmeticBase
{
public:
  /**
   * A typedef for the traits template parameter. Can be used to
   * access the rebinder and other parts of the traits directly.
   *
   * @code
   * PiiColor<> clr;
   * PiiColor<double> clr2 = PiiColor<>::TraitsType::Rebind<double>::Type();
   * @endcode
   */
  typedef Traits TraitsType;
  /**
   * A shorthand for the content type of the derived class.
   */
  typedef typename Traits::Type T;
  /**
   * Qt-style iterator.
   */
  typedef typename Traits::Iterator Iterator;
  /**
   * Qt-style const iterator.
   */
  typedef typename Traits::ConstIterator ConstIterator;
  /**
   * Stl-style iterator.
   */
  typedef typename Traits::Iterator iterator;
  /**
   * Stl-style const iterator.
   */
  typedef typename Traits::ConstIterator const_iterator;

  /**
   * Returns an stl-style const iterator to the beginning of data.
   */
  inline const_iterator constBegin() const { return self()->begin(); }
  /**
   * Returns an stl-style const iterator to the end of data.
   */
  inline const_iterator constEnd() const { return self()->end(); }

  /**
   * Sum corresponding elements.
   */
  Derived operator+ (const Derived& other) const { return binaryOp(std::plus<T>(), other); }
  /**
   * Subtract corresponding elements from each other.
   */
  Derived operator- (const Derived& other) const { return binaryOp(std::minus<T>(), other); }
  /**
   * Divide corresponding elements by each other.
   */
  Derived operator/ (const Derived& other) const { return binaryOp(std::divides<T>(), other); }
  /**
   * Multiply corresponding elements by each other.
   */
  Derived operator* (const Derived& other) const { return binaryOp(std::multiplies<T>(), other); }
  /**
   * Add corresponding elements in @p other to this.
   */
  void operator+= (const Derived& other) { return binaryOp(std::plus<T>(), other); }
  /**
   * Subtract corresponding elements in @p other from this.
   */
  void operator-= (const Derived& other) { return binaryOp(std::minus<T>(), other); }
  /**
   * Divide corresponding elements in this by @p other.
   */
  void operator/= (const Derived& other) { return binaryOp(std::divides<T>(), other); }
  /**
   * Multiply corresponding elements in this by @p other.
   */
  void operator*= (const Derived& other) { return binaryOp(std::multiplies<T>(), other); }
  /**
   * Add @p value to all elements.
   */
  Derived operator+ (T value) const { return binaryOp(std::plus<T>(), value); }
  /**
   * Subtract @p value from all elements.
   */
  Derived operator- (T value) const { return binaryOp(std::minus<T>(), value); }
  /**
   * Divide all elements by @p value.
   */
  Derived operator/ (T value) const { return binaryOp(std::divides<T>(), value); }
  /**
   * Multiply all elements by @p value.
   */
  Derived operator* (T value) const { return binaryOp(std::multiplies<T>(), value); }
  /**
   * Add @p value to all elements.
   */
  void operator+= (T value) { return binaryOp(std::plus<T>(), value); }
  /**
   * Subtract @p value from all elements.
   */
  void operator-= (T value) { return binaryOp(std::minus<T>(), value); }
  /**
   * Divide all elements by @p value.
   */
  void operator/= (T value) { return binaryOp(std::divides<T>(), value); }
  /**
   * Multiply all elements by @p value.
   */
  void operator*= (T value) { binaryOp(std::multiplies<T>(), value); }
  /**
   * Create a negation of all elements.
   */
  Derived operator- () const { return unaryOp(std::negate<T>()); }
  /**
   * Assign the values in @p other to this.
   */
  Derived& operator= (const Derived& other) { binaryOp(Pii::SelectSecond<typename Traits::Type>(), other); return *self(); }
  /**
   * Set all elements to @p value.
   */
  Derived& operator= (T value) { binaryOp(Pii::SelectSecond<typename Traits::Type>(), value); return *self(); }

  /**
   * Returns @p true if all elements in @p this and @p other are
   * equal, @p false otherwise.
   */
  bool operator==(const Derived& other) const
  {
    typename Traits::ConstIterator myCurrent = self()->begin();
    typename Traits::ConstIterator otherCurrent = other.begin();
    while (myCurrent != self()->end())
      if (*(myCurrent++) != *(otherCurrent++))
        return false;
    return true;
  }
  /**
   * Returns @p false if all elements in @p this and @p other are
   * equal, @p true otherwise.
   */
  bool operator!=(const Derived& other) const
  {
    typename Traits::ConstIterator myCurrent = self()->begin();
    typename Traits::ConstIterator otherCurrent = other.begin();
    while (myCurrent != self()->end())
      if (*(myCurrent++) != *(otherCurrent++))
        return true;
    return false;
  }

  /**
   * Apply a unary function to all elements. Return a new object.
   */
  template <class Operation>
  typename Traits::template Rebind<typename Operation::result_type>::Type unaryOp(Operation op) const
  {
    typedef typename Traits::template Rebind<typename Operation::result_type>::Type ResultType;
    ResultType result;
    
    // Take iterators to the beginning of both elements
    typename Traits::ConstIterator myCurrent = self()->begin();
    typename ResultType::Iterator resultCurrent = result.begin();
    while (myCurrent != self()->end())
      {
        *resultCurrent = op(*myCurrent);
        ++myCurrent;
        ++resultCurrent;
      }
    return result;
  }
  
  /**
   * Apply a unary function to all elements. Modify the elements in
   * place.
   */
  template <class Operation> void unaryOp(Operation op)
  {
    // Take iterators to the beginning my elements
    typename Traits::Iterator myCurrent = self()->begin();
    // Apply unary function to all
    while (myCurrent != self()->end())
      {
        *myCurrent = op(*myCurrent);
        ++myCurrent;
      }
  }

  /**
   * Apply a binary function to all elements using the corresponding
   * elements in @p this and @p other as function parameters. Return a
   * new object.
   */
  template <class Operation> typename Traits::template Rebind<typename Operation::result_type>::Type
  binaryOp(Operation op,
           const typename Traits::template Rebind<typename Operation::second_argument_type>::Type& other) const
  {
    typedef typename Traits::template Rebind<typename Operation::result_type>::Type ResultType;
    typedef typename Traits::template Rebind<typename Operation::second_argument_type>::Type SecondType;
    ResultType result;
    // Take iterators to the beginning of all elements
    typename Traits::ConstIterator myCurrent = self()->begin();
    typename SecondType::ConstIterator otherCurrent = other.begin();
    typename ResultType::Iterator resultCurrent = result.begin();
    while (myCurrent != self()->end())
      {
        *resultCurrent = op(*myCurrent, *otherCurrent);
        ++myCurrent;
        ++otherCurrent;
        ++resultCurrent;
      }
    return result;
  }
  
  /**
   * Apply a binary function to all elements using the corresponding
   * elements in @p this and @p other as function parameters. Modify
   * elements in place.
   */
  template <class Operation>
  void binaryOp(Operation op,
                const typename Traits::template Rebind<typename Operation::second_argument_type>::Type& other)

  {
    typedef typename Traits::template Rebind<typename Operation::second_argument_type>::Type SecondType;

    // Take iterators to the beginning of both elements
    typename Traits::Iterator myCurrent = self()->begin();
    typename SecondType::ConstIterator otherCurrent = other.begin();
    while (myCurrent != self()->end())
      {
        *myCurrent = op(*myCurrent, *otherCurrent);
        ++myCurrent;
        ++otherCurrent;
      }
  }

  /**
   * Apply a binary function to all elements using @p value as the
   * second function parameter. Return a new object.
   */
  template <class Operation> typename Traits::template Rebind<typename Operation::result_type>::Type
  binaryOp(Operation op, typename Operation::second_argument_type value) const
  {
    typedef typename Traits::template Rebind<typename Operation::result_type>::Type ResultType;
    ResultType result(*self());
    result.binaryOp(op, value);
    return result;
  }
  
  /**
   * Apply a binary function to all elements using @p value as the
   * second function parameter. Modify elements in place.
   */
  template <class Operation> void binaryOp(Operation op, typename Operation::second_argument_type value)
  {
    // Take iterator to the beginning of my elements
    typename Traits::Iterator myCurrent = self()->begin();
    while (myCurrent != self()->end())
      {
        *myCurrent = op(*myCurrent, value);
        ++myCurrent;
      }
  }

private:
  Derived* self() { return static_cast<Derived*>(this); }
  const Derived* self() const { return static_cast<const Derived*>(this); }
};

#endif //_PIIARITHMETICBASE_H
