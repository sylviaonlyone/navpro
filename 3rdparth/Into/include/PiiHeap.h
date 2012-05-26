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

#ifndef _PIIHEAP_H
#define _PIIHEAP_H

#include <QVarLengthArray>
#include <functional>

/// @internal
#define PII_HEAP_PARENT_INDEX(i)    ((i-1)>>1)
/// @internal
#define PII_HEAP_CHILD_INDEX(i)     ((i<<1)+1)

/**
 * A heap data structure. Heap is a complete binary tree with the
 * following properties:
 *
 * @li All leaf nodes reside in at most two adjacent levels.
 * @li It is filled from left.
 * @li No child node in the tree is greater than its parent. (Normal
 * heap)
 * @li No child node in the three is smaller than its parent. (Inverse
 * heap)
 *
 * As a consequence of the third property, the first element in a heap
 * (the one at index 0) is the largest one (normal heap) or the
 * smallest one (inverse heap). The other elements are not in any
 * particular order. Use #sort() to sort the values. Sorting a heap in
 * place inverts it.
 *
 * Heap quarantees <tt>O(log(N))</tt> complexity for insertion and
 * deletion. The complexity for sorting N items is <tt>O(N
 * log(N))</tt>. It is most commonly used as a priority queue to
 * maintain the N most "important" entries during algorithm execution.
 *
 * %PiiHeap uses QVarLengthArray as a storage. Please refer to its
 * documentation for details on memory arrangement.
 *
 * @note If you modify the heap data in place, it may not have the
 * heap property any more.
 *
 * %PiiHeap can be used to maintain partial sort order. The following
 * example finds the five smallest numbers in a random number set:
 *
 * @code
 * // Preallocate 5 elements on the stack.
 * PiiHeap<double, 5> heap;
 * // Initialize with the largest possible value.
 * heap.fill(5, 1.0);
 * for (int i=1000; i--; )
 *   heap.put(Pii::uniform());
 * // Heap now contains the 5 smallest random numbers generated
 * heap.sort();
 * // The numbers are now in ascending order.
 * @endcode
 *
 * @ingroup Core
 */
template <class T, int prealloc = 16> class PiiHeap
{
public:
  /**
   * Constructs a heap with an initial size of @p size elements. The
   * elements will not be initialized unless @p T has a default
   * constructor.
   *
   * @param size initial size for the heap
   *
   * @param inverse inversion flag. Normal heap (@p inverse = @p
   * false) always keeps the largest element at the top of the binary
   * tree. Inverse heap keeps the smallest element.
   */
  PiiHeap(int size = 0, bool inverse = false) : _array(size), _bInverse(inverse) {}

  /**
   * Constructs a heap with an initial size of @p size elements. @p
   * fillValue will be used to initialize the elements.
   *
   * @param size initial size for the heap
   *
   * @param fillValue the initial value of the elements
   *
   * @param inverse inversion flag. Normal heap (@p inverse = @p
   * false) always keeps the largest element at the top of the binary
   * tree. Inverse heap keeps the smallest element.
   */
  PiiHeap(int size, const T& fillValue, bool inverse) : _array(size), _bInverse(inverse) { fill(size, fillValue); }

  /**
   * Copies @a other.
   */
  PiiHeap(const PiiHeap& other) : _array(other._array), _bInverse(other._bInverse) {}

  /**
   * Assigns @a other to @p this and returns @p *this.
   */
  PiiHeap& operator= (const PiiHeap& other)
  {
    _array = other._array;
    _bInverse = other._bInverse;
    return *this;
  }
  
  /**
   * Returns a pointer to the beginning of the heap data.
   */
  T* _d() { return _array._d(); }

  /**
   * Returns const a pointer to the beginning of the heap data.
   */
  const T* _d() const { return _array._d(); }
  
  /**
   * Returns a reference to the item at @a index.
   */
  T& operator[] (int index) { return _array[index]; }
  /**
   * Returns a @p const reference to the item at @a index.
   */
  const T& operator[] (int index) const { return _array[index]; }

  /**
   * Adds @p element to the end of the heap and rearranges elements so
   * that the heap property is retained.
   */
  void append(const T& element);

  /**
   * Puts an element to the heap so that #size() smallest/largest
   * elements are always retained. If the heap is a normal heap,
   * @a element will be only placed into the list if it is smaller than
   * the first element (the topmost parent), which will be removed. If
   * the heap is inverted, the @a element will only be accepted if it
   * is larger than the first one, which will be removed. This
   * technique always retains the N smallest elements for a normal
   * heap and N largest elements for an inverse heap. This function
   * will not change the number of elements in the list. Use #fill()
   * to initialize the heap.
   */
  void put(const T& element);

  /**
   * Reserves space for @a n elements and initializes each to @a value.
   */
  void fill(int n, const T& value);
  
  /**
   * Removes the element at @a index and rearranges the heap to retain
   * the heap property.
   */
  void remove(int index);

  /**
   * Removes the element at @a index and returns it. The heap will be
   * rearranged to retain the heap property.
   */
  T take(int index);
  
  /**
   * Returns the number of elements in the heap.
   */
  int size() const { return _array.size(); }

  /**
   * Sorts the heap in place. Sorting inverts the heap property. If the
   * parents are larger than their children before sorting, they will
   * be smaller after sorting and vice versa. After sorting, elements
   * are either in ascending order (normal heap) or descending order
   * (inverse heap).
   */
  void sort();

  /**
   * Returns a copy of this heap with its element in a sorted order. 
   * Normal heap will be turned into an inverse one and vice versa.
   */
  PiiHeap sorted() const;

  /**
   * Returns @p true if the heap is inversed (smallest value at top)
   * and @p false otherwise.
   */
  bool isInverse() const { return _bInverse; }

private:
  template <class GreaterThan> void append(const T& element, GreaterThan greaterThan);
  template <class GreaterThan> void remove(int index, GreaterThan greaterThan);
  template <class GreaterThan> inline void updateDownwards(int index, int currentItems, GreaterThan greaterThan);
  template <class GreaterThan> inline void sort(GreaterThan greaterThan);
  template <class GreaterThan> inline void put(const T& element, GreaterThan greaterThan);

  QVarLengthArray<T,prealloc> _array;
  bool _bInverse;
};


template <class T, int prealloc> PiiHeap<T,prealloc> PiiHeap<T,prealloc>::sorted() const
{
  PiiHeap<T,prealloc> sorted(*this);
  sorted.sort();
  return sorted;
}


template <class T, int prealloc> void PiiHeap<T,prealloc>::sort()
{
  if (_bInverse)
    {
      sort(std::less<T>());
      _bInverse = false;
    }
  else
    {
      sort(std::greater<T>());
      _bInverse = true;
    }
}

template <class T, int prealloc>
template <class GreaterThan> void PiiHeap<T,prealloc>::sort(GreaterThan greaterThan)
{
  int size = _array.size();

  while (size > 1)
    {
      // Move the first (largest) element to the end of the list
      qSwap(_array[0], _array[--size]);
      // Update the (one element smaller) heap
      updateDownwards(0, size, greaterThan);
    } 
}

template <class T, int prealloc> void PiiHeap<T,prealloc>::put(const T& element)
{
  if (_array.size() > 0)
    {
      if (_bInverse)
        {
          if (element > _array[0])
            put(element, std::less<T>());
        }
      else if (element < _array[0])
        put(element, std::greater<T>());
    }
}

template <class T, int prealloc>
template <class GreaterThan> void PiiHeap<T,prealloc>::put(const T& element, GreaterThan greaterThan)
{
  _array[0] = element;
  updateDownwards(0, _array.size(), greaterThan);
}

template <class T, int prealloc> void PiiHeap<T,prealloc>::fill(int n, const T& value)
{
  _array.resize(n);
  for (int i=0; i<n; ++i)
    _array[i] = value;
}

template <class T, int prealloc> void PiiHeap<T,prealloc>::append(const T& element)
{
  if (_bInverse)
    append(element, std::less<T>());
  else
    append(element, std::greater<T>());
}


template <class T, int prealloc>
template <class GreaterThan> void PiiHeap<T,prealloc>::append(const T& element, GreaterThan greaterThan)
{
  int childIndex = _array.size();
  int parentIndex = PII_HEAP_PARENT_INDEX(childIndex);

  _array.append(element);

  // Bubble the just appended element upwards until its parent is
  // greater.
  while (parentIndex >=0 && greaterThan(_array[childIndex], _array[parentIndex]))
    {
      qSwap(_array[parentIndex], _array[childIndex]);
      childIndex = parentIndex;
      parentIndex = PII_HEAP_PARENT_INDEX(childIndex);
    }
}

template <class T, int prealloc> T PiiHeap<T,prealloc>::take(int index)
{
  // Store the removed element.
  T tmp(_array[index]);
  remove(index);
  return tmp;
}

template <class T, int prealloc> void PiiHeap<T,prealloc>::remove(int index)
{
  if (_bInverse)
    remove(index, std::less<T>());
  else
    remove(index, std::greater<T>());
}

template <class T, int prealloc>
template <class GreaterThan> void PiiHeap<T,prealloc>::remove(int index, GreaterThan greaterThan)
{
  int iCurrentItems = _array.size()-1;

  // Check if we need to heapize the list
  if (index < iCurrentItems)
    {
      // Move the last element to the free place.
      _array[index] = _array[iCurrentItems];

      // Update the heap downwards from the replaced element.
      updateDownwards(index, iCurrentItems, greaterThan);
      
      // Update upwards from the replaced element. If a child node is
      // larger than its parent, swap the two.
      int childIndex = index;
      int parentIndex = PII_HEAP_PARENT_INDEX(childIndex);
      while (parentIndex >=0 && greaterThan(_array[childIndex], _array[parentIndex]))
        {
          qSwap(_array[parentIndex], _array[childIndex]);
          childIndex = parentIndex;
          parentIndex = PII_HEAP_PARENT_INDEX(childIndex);
        }
    }
  
  _array.resize(iCurrentItems);
}

template <class T, int prealloc>
template <class GreaterThan> void PiiHeap<T,prealloc>::updateDownwards(int index,
                                                                       int currentItems,
                                                                       GreaterThan greaterThan)
{
  // If the parent of a child node is smaller than a child, then swap
  // the parent with the larger of its children. The list is
  // considered only up to currentItems.
  int parentIndex = index;
  int childIndex = PII_HEAP_CHILD_INDEX(index);
  while ((currentItems > childIndex && greaterThan(_array[childIndex], _array[parentIndex])) ||
         (currentItems > childIndex+1 && greaterThan(_array[childIndex+1], _array[parentIndex])))
    {
      if (currentItems > childIndex+1 && greaterThan(_array[childIndex+1], _array[childIndex]))
        ++childIndex;
      qSwap(_array[parentIndex], _array[childIndex]);
      parentIndex = childIndex;
      childIndex = PII_HEAP_CHILD_INDEX(parentIndex);
    }
}
  
#endif //_PIIHEAP_H
