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

#ifndef _PIIIMAGETRAITS_H
#define _PIIIMAGETRAITS_H

namespace PiiImage
{
  /**
   * Traits for integer-valued images.
   */
  template <class T> struct IntegerTraits
  {
    /**
     * Returns the maximum value of a color channel. The maximum
     * channel value is 255 for all integer types and 1.0 for
     * floating-point types.
     */
    static T max() { return 255; }
    template <class U> static U toFloat(T value) { return U(value)/max(); }
    template <class U> static U toInt(T value) { return U(value); }
    template <class U> static T fromFloat(U value) { return T(value * max()); }
    template <class U> static T fromInt(U value) { return T(value); }
  };
  
  /**
   * Traits for floating-point images.
   */
  template <class T> struct FloatTraits
  {
    static T max() { return 1.0; }
    template <class U> static U toFloat(T value) { return U(value); }
    template <class U> static U toInt(T value) { return U(value * IntegerTraits<U>::max()); }
    template <class U> static T fromFloat(U value) { return T(value); }
    template <class U> static T fromInt(U value) { return T(value) / IntegerTraits<U>::max(); }
  };
  
  /**
   * The Traits structure specifies default properties of different
   * image types.
   */
  template <class T> struct Traits : Pii::IfClass<Pii::IsInteger<T>, IntegerTraits<T>, FloatTraits<T> >::Type
  {
  };

  /**
   * Specialization for three-channel colors. Just maps the color type
   * to the channel type.
   */
  template <class T> struct Traits<PiiColor<T> > : Traits<T> {};
  /**
   * Specialization for four-channel colors. Just maps the color type
   * to the channel type.
   */
  template <class T> struct Traits<PiiColor4<T> > : Traits<T> {};
};

#endif //_PIIIMAGETRAITS_H
