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

#error "Documentation.h is not intended for inclusion. Its sole purpose is to provide documentation for a package."

/**
 * @defgroup Serialization Serialization library
 *
 * The task of the serialization library is to marshal and demarshal
 * complex data structures. It uses a rather complex template
 * mechanism to generate (de)serialization code at compile time. The
 * result is efficient, compact and almost unreadable code. The
 * documentation links below try to cast some light into the
 * philosophy.
 *
 * @li @ref serialization_design_principles
 * @li @ref serialization_basic_concepts
 * @li @ref serialization_making_serializable
 * @li @ref serialization_wrappers
 * @li @ref serialization_archives
 * @li @ref serialization_tracking
 * @li @ref serialization_errors
 * @li @ref serialization_archive_format
 * @li @ref serialization_gotchas
 *
 *
 * @page serialization_mainpage Serialization Library
 *
 * @li @subpage serialization_design_principles
 * @li @subpage serialization_basic_concepts
 * @li @subpage serialization_making_serializable
 * @li @subpage serialization_wrappers
 * @li @subpage serialization_archives
 * @li @subpage serialization_tracking
 * @li @subpage serialization_errors
 * @li @subpage serialization_archive_format
 * @li @subpage serialization_gotchas
 */
 

/**
 * @page serialization_design_principles Design Principles and Features
 *
 * @section serialization_principles Design Principles
 *
 * Although the Serialization library may seem rather complex and hard
 * to understand at first sight, most of its obscurity has good
 * reasons.
 *
 * @li @b Performance. Extensive use of templates makes it unnecessary
 * to use virtual functions. Most serialization calls can be optimized
 * to direct stream operations by modern compilers.
 *
 * @li @b Flexibility. The use of templates makes it possible to
 * override any behavior without touching existing code and with no
 * run-time overhead.
 *
 * @li <b>Ease of use</b>. Although the library itself is complex,
 * its use is not. Serializable objects can be stored and restored
 * with a single function. There is often no need to write separate
 * save/load functions. All primitive data types and most Qt types can
 * be serialized with a single line of code.
 *
 * @section Features
 *
 * This section shortly summarizes the main features of the library.
 * Detailed documentation is provided in other places.
 *
 * @li @b Format-independend @b serialization. Serializable objects
 * just write and read @ref serialization_concepts_archives that
 * define the format of data. The library comes with support for
 * binary and plain text formats.
 *
 * @li @b Versioning. Objects and archives are independently
 * versioned. The version number is automatically inspected for
 * incompabilities, and serializable objects have an option to be
 * backwards-compatible when needed.
 *
 * @li @b Non-intrusive. The library makes it possible to write
 * serializers for types whose code is inaccessible.
 *
 * @li @b Graph @b reconstruction. The process of solving memory
 * references (via pointers or references) is often called graph
 * reconstruction. The library ensures that a memory location is only
 * serialized once and that all references remain valid when read back
 * (even with reference-counted pointers). The @ref
 * serialization_tracking "tracking" of memory addresses can be
 * selectively turned off for any object type.
 *
 * @li @b DLL @b support. The library supports serialization of
 * classes defined in dynamically loaded libraries.
 *
 * The Serialization library owes much to boost::serialization, but
 * has some notable differences. While boost::serialization builds on
 * stl, we use Qt. The Serialization library supports DLLs, which the
 * boost counterpart does not (at least it did not at the time of
 * writing this). Since we are not supporting obsolete compilers (like
 * MSVC 6.0), the code also has fewer hacks.
 */


/**
 * @page serialization_basic_concepts Basic Concepts
 *
 * @section serialization_concepts_archives Archives
 *
 * Archive is an abstract concept for an object that defines the
 * format of data and is able to read and write data in that format. 
 * The serialization library is completely data format agnostic. The
 * storage format may be changed simply by providing a new archive
 * class. Currently, binary and plain text archives are supported.
 *
 * Although an archive can be specialized for a specific storage
 * format and storage device only, a better design pattern is to
 * separate archive and the actual I/O device. This way it is possible
 * to use any storage format with any I/O device. This allows one to
 * separate data format from storage format.
 *
 * Archives are separated into two categories: input and output
 * archives. Since they always come in pairs, the obvious question is
 * why they haven't been combined into one? The reason is that
 * separate input and output archives allow us to create template
 * serialization functions that work in both directions. Both archive
 * types define an @p operator& function that works as a '>>' for
 * input archives and as a '<<' for output archives.
 *
 * The class hierarchy for archives is rather complex, but it makes it
 * possible to easily customize archives at any level. Here, we'll
 * focus on input archives. The class hierarchy for output archives is
 * equivalent.
 *
 * All input archives are derived from the PiiInputArchive template. 
 * The purpose of this archive is to overload the & operator so that
 * it uses the >> operator of the archive implementation. An archive
 * implementation must provide a >> operator for all primitive types
 * and pointers. PiiInputArchive also provides support for handling
 * pointer serialization. It provides a template implementation for
 * the >> operator that works as a fallback when the derived archive
 * implementation does not provide a specialized one. The
 * implementation takes care of handling references and pointers so
 * that pointers and references to the same memory address are
 * correctly restored.
 *
 * The actual archive implementations must only provide specialized
 * implementations of the >> operator for reading primitive types and
 * QStrings. Whenever the operator & is invoked in source code,
 * PiiInputArchive converts it to @p operator>>. If the archive
 * implementation has a specialized implementation for the data type
 * in question, it will be invoked. If not, the fallback function will
 * be called.
 *
 * @section serialization_serializers Serializers and Factory Objects
 *
 * All non-primitive types are stored and restored through special
 * objects called serializers. Since serialization functions are
 * templates that are bound to a certain archive type at compile
 * time, serializers must be registered to a specific archive type.
 *
 * The Serialization library uses two different types of serializers,
 * one for serializing class instances whose type is not known at
 * compile time and the other for all others. If the type of the class
 * is known at compile time (it is not serialized through a base class
 * pointer/reference), the PiiSerialization::serialize() template
 * function will be invoked. If a template specialization for the type
 * to be serialized is available, it will be called. Otherwise, the
 * fallback function calls a @p serialize() member function of the
 * serializable object through the PiiSerialization::Accessor struct. 
 * Declaring this stuct as a friend of a serializable class makes it
 * possible to make the serialization function(s) private.
 *
 * The only portable way of requesting the name of a type at run time
 * is to use a virtual function. The serialization library uses meta
 * objects for this (See @ref PiiVirtualMetaObject.h). Therefore, the
 * base class of virtually serialized objects must declare a virtual
 * piiMetaObject() function. However, if you have an alternative way
 * of doing this, just override the
 * PiiSerialization::metaObjectPointer() function.
 *
 * If the type of a class cannot be known at compile time, the type is
 * said to be @e dynamic (see PiiSerialization::isDynamicType()). In
 * this case a serializer with a virtual serialization function needs
 * to be registered to the serializer map of an archive type. The name
 * of the type (obtained from the meta object) is used as a key in
 * look-ups. Once the serializer is found, its @e virtual @p
 * serialize() member function is invoked.
 *
 * Another requirement for dynamic types is to register a factory
 * object for creating instances of the class when deserializing (see
 * PiiSerializationFactory). Again, the class name (read from the
 * archive) is used as a key in look-ups.
 *
 * The library needs to be specifically told to use the serializer and
 * factory maps. The way to do this is to override the
 * PiiSerialization::isDynamicType() function (See @ref
 * PII_SERIALIZATION_DYNAMIC).
 *
 *
 * @section serialization_serializables Serializable Objects
 *
 * An object can be made serializable in a few different ways. All
 * primitive types are serializable without a serializer because the
 * archive implementations are required to provide the << and >>
 * operators for them. Class and struct types need a custom
 * serialization mechanism.
 *
 * In @e internal serialization, the serializable object itself has a
 * member function called @p serialize():
 *
 * @code
 * class MyClass
 * {
 *   friend struct PiiSerialization::Accessor;
 *   template <class Archive> void serialize(Archive& archive, const unsigned int version);
 * };
 * @endcode
 *
 * The function can be public, but since it should never be used
 * directly, it is wise to leave it private and declare
 * PiiSerialization::Accessor as a friend.
 *
 * In @e external serialization, there is no need to modify the class
 * itself. One only needs to provide a specialization of
 * PiiSerialization::serialize() for the serializable type. The
 * downside is that one can only work through the public interface of
 * the serializable object.
 *
 * @code
 * template <class Archive> inline void serialize(Archive& archive, MyType& value, const unsigned int version)
 * {
 *   // do serialization stuff here
 * }
 * @endcode
 *
 * In @e virtual serialization, the actual serialization mechanism may
 * be either internal or external, but it is bound at run time.
 * Virtual serializers must be bound to a class name by registering it
 * to the serialization map of an archive type.
 *
 *
 * @section serialization_traits Serialization Traits
 *
 * Traits are a C++ programming concept in which properties are bound
 * to types at compile time. Traits are implemented as template
 * classes with static constants which can be evaluated by the
 * compiler. The PiiSerializationTraits namespace holds traits that
 * control the serialization of objects. The controllable aspects of
 * serialization are:
 *
 * @li @ref PiiSerializationTraits::Tracking "Tracking". Whether or
 * not memory addresses to this type of objects will be tracked. See
 * @ref serialization_tracking for details. Primitive types will never
 * be tracked. For all other types, tracking is enabled by default. If
 * you need to store a pointer graph to, say, an @p int, create a @ref
 * serialization_wrappers "wrapper" or use a typedef:
 *
 * @code
 * typedef int MyInt;
 * PII_SERIALIZATION_TRACKING(MyInt, true)
 * template <class Archive> void serialize(Archive& ar, MyInt& i, const unsigned int)
 *   {
 *     archive & (int&)i;
 *   }
 * @endcode
 *
 * @li @ref PiiSerializationTraits::ClassInfo "ClassInfo". Will class
 * information be stored? Currently, the class information only
 * includes the version number.
 *
 * @li @ref PiiSerializationTraits::Version "Version". The version
 * number of a type. The default version number is 0. See @ref
 * serialization_making_version for details.
 *
 * @li @ref PiiSerializationTraits::ClassName "ClassName". The class
 * name. Class name must be defined for all dynamic types because it
 * is needed to a) instantiate the object through a factor b) look up
 * a serializer for serializing the object.
 *
 * @li @ref PiiSerializationTraits::IsAbstract "IsAbstract". Is it
 * possible to instantiate the class? Abstract superclasses of
 * serializable types need to turn this trait to @p true. It is also
 * needed for non-abstract superclasses that provide no default
 * constructor.
 *
 * The easiest way of controlling the traits is through macros in @ref
 * core/serialization/PiiSerializationTraits.h
 * "PiiSerializationTraits.h".
 *
 * Note that the traits affect archive format. Changing them may
 * invalidate previously stored data. Thus, one should change the
 * defaults with caution.
 *
 */


/**
 * @page serialization_making_serializable Making a Class Serializable
 *
 * @section serialization_making_basic Basic Serialization
 *
 * The simplest way of making a class serializable is to implement a
 * @p serialize() member function:
 *
 * @code
 * class MyClass
 * {
 * private:
 *   int _i;
 *   double _d;
 *
 *   // Serialization declarations
 *   friend struct PiiSerialization::Accessor;
 *   template <class Archive> void serialize(Archive& archive, const unsigned int version)
 *     {
 *        archive & _i;
 *        archive & _d;
 *     }
 * };
 * @endcode
 *
 * The @p friend declaration isn't actually necessary, but makes it
 * possible to hide the serialization function from the cruel world. 
 * The Serialization library invokes the same serialization function
 * for both input and output archives. Since they both have the @p
 * operator& defined, the same function works both in saving and
 * loading data. Note that serialization depends on the order of
 * reading / writing. If you write @p _i first, you @e must read it
 * back first.
 *
 * Now, let's assume you need to serialize a class whose code you
 * cannot or don't want to alter. Its public interface looks like
 * this:
 *
 * @code
 * class ProprietaryClass
 * {
 * public:
 *   int value() const;
 *   void setValue(int value);
 * };
 * @endcode
 *
 * Since modification of the class declaration is not feasible, one
 * needs to implement an external serialization function. The problem
 * is that the member variables cannot be directly accessed. In
 * saving, the @p value() member function must be used. In loading,
 * one needs its @p setValue() counterpart. The library provides a
 * macro, @ref PII_SEPARATE_SAVE_LOAD_FUNCTIONS, to conveniently solve
 * this problem. The macro creates a @p serialize() function that
 * invokes either @p save() or @p load(), depending on the archive
 * type.
 *
 * @code
 * PII_SEPARATE_SAVE_LOAD_FUNCTIONS(ProprietaryClass)
 *
 * namespace PiiSerialization
 * {
 *   template <class Archive> inline void save(Archive& ar, ProprietaryClass& obj, const unsigned int version)
 *     {
 *        int value = obj.value();
 *        ar << value;
 *     }
 *
 *   template <class Archive> inline void load(Archive& ar, ProprietaryClass& obj, const unsigned int version)
 *     {
 *        int value;
 *        ar >> value;
 *        obj.setValue(value);
 *     }
 * }
 * @endcode
 *
 * Save and load can be separated also for internal serialization, if
 * needed:
 *
 * @code
 * class MyClass
 * {
 * private:
 *   friend struct PiiSerialization::Accessor;
 *   PII_SEPARATE_SAVE_LOAD_MEMBERS
 *   template <class Archive> void save(Archive& archive, const unsigned int version)
 *     {
 *        // ...
 *     }
 *   template <class Archive> void load(Archive& archive, const unsigned int version)
 *     {
 *        // ...
 *     }
 * };
 * @endcode
 *
 * @section serialization_making_base Serializing Base Class
 *
 * When derived objects are serialized, their base class also needs to
 * be serialized. Since serialization of an object must always be
 * controlled by the Serialization library, one should @b never call
 * the @p serialize() member function of a base class directly. 
 * Keeping the serialization function private as suggested helps in
 * this.
 *
 * @ref core/serialization/PiiBaseClass.h "PiiBaseClass.h" provides
 * means of serializing base classes. The easiest way of serializing
 * the base class is the @ref PII_SERIALIZE_BASE macro:
 *
 * @code
 * class MyDerivedClass : public MyClass
 * {
 * private:
 *   float _f;
 *
 *   friend struct PiiSerialization::Accessor;
 *   template <class Archive> void serialize(Archive& archive, const unsigned int version)
 *     {
 *       PII_SERIALIZE_BASE(archive, MyClass);
 *       archive & _f;
 *     }
 * };
 * @endcode
 *
 * This approach has the advantage that the subclass does not need to
 * know how the superclass has serialization implemented. It may be
 * either internal or external, but we don't need to care.
 *
 * The serialization of the base class goes through a @ref
 * serialization_wrappers "wrapper" for which the @ref
 * serialization_tracking "tracking" and classinfo @ref
 * serialization_traits "traits" are turned off. Thus, the wrapper
 * itself leaves no marks into the archive. The version number of the
 * base class will be stored, which makes it possible to change the
 * storage format on all levels of the inheritance hierarchy.
 *
 * @section serialization_making_virtual Serializing through Base Class Pointer
 *
 * So far we have been dealing with types we know at compile time. 
 * Serialization of such types can be resolved by the compiler,
 * provided that proper declarations for the serialization function
 * templates are available.
 *
 * Imagine a class called @p Garage whose purpose is to store a
 * collection of @p Cars. @p Car is an (abstract) superclass for @p
 * Truck, @p Mazda, and @p Volvo. @p Truck is inherited by @p Sisu
 * and a number of other makes that will be available upon loading of
 * dynamic libraries. The best @p Garage can do is to keep a list of
 * pointers to @p Cars, because it has no way of knowing all possible
 * types at compile time. The class declarations look like this:
 *
 * @code
 * class Garage
 * {
 * public:
 *   // I'm a collector; there is no removeCar() function.
 *   void addCar(Car* car) { _lstCars.apppend(car); }
 * private:
 *   QList<Car*> _lstCars;
 * };
 *
 * // We omit the public interface. Just private stuff is shown.
 * class Car
 * {
 *   int _iWheels;
 * };
 *
 * class Truck : public Car
 * {
 *   bool _bHasTrailer;
 * };
 *
 * class Sisu : public Truck
 * {
 *   double _dExtraWidthForEURoads;
 * };
 *
 * class Mazda : public Car
 * {
 *   bool _bIsRusty;
 * };
 *
 * class Volvo : public Car
 * {
 *   enum Sound { TractorSound, BulldozerSound, TruckSound };
 *   Sound _sound;
 * };
 * @endcode
 *
 * Now, how do we ensure that the members of the derived classes are
 * saved when a @p Garage is stored?
 *
 * The Serialization library uses PiiMetaObject to store information
 * related to a type. Currently, such information include class name
 * and version number. Since requesting the meta object for @p Car
 * would not help in our situation, there must be a way of finding the
 * "most derived" class. The only way this can be performed across
 * dynamic library boundaries is to implement a virtual
 * piiMetaObject() function. (Yes, we have seen the vtable tricks in
 * boost::serialization.)
 *
 * The downside of the virtual meta object function is that the
 * declaration of the base class must be accessible. If this is not
 * the case, one needs to create an empty subclass just for
 * serialization purposes. The empty subclass declares the virtual
 * metaobject function. To make @p Cars serializable through the base
 * class, we make the following change:
 *
 * @code
 * class Car
 * {
 *   friend struct PiiSerialization::Accessor;
 *   PII_VIRTUAL_METAOBJECT_FUNCTION
 *
 *   // The serialization function
 *   template <class Archive> void serialize(Archive& archive, const unsigned int)
 *     {
 *       PII_SERIALIZE(archive, _iWheels);
 *     }
 *
 *   int _iWheels;
 * };
 *
 * // ...
 *
 * class Volvo : public Car
 * {
 *   friend struct PiiSerialization::Accessor;
 *   PII_VIRTUAL_METAOBJECT_FUNCTION
 *
 *   template <class Archive> void serialize(Archive& archive, const unsigned int)
 *     {
 *       PII_SERIALIZE_BASE(archive, Car);
 *       PII_SERIALIZE(archive, _sound);
 *     }
 *
 *   enum Sound { TractorSound, BulldozerSound, TruckSound };
 *   Sound _sound;
 * };
 *
 * // Specialize meta object for this type to call the virtual function
 * PII_SERIALIZATION_VIRTUAL_METAOBJECT(Car)
 * @endcode
 *
 * The @ref PII_VIRTUAL_METAOBJECT_FUNCTION macro must be present in
 * all class declarations that are serialized through a base class
 * pointer, and the base class itself. The base class must be
 * annotated with the @ref PII_SERIALIZATION_VIRTUAL_METAOBJECT macro. 
 * In our example, only @p Car needs it. If the base class is
 * abstract, one needs to use the @ref PII_SERIALIZATION_ABSTRACT
 * macro.
 *
 * Now that the Serialization library knows the meta object it must
 * find a serializer and a factory (an object that can create a class
 * instance given a key) for the corresponding class. The class name
 * is used as a look-up key for this. Every archive type has its own
 * serializer map to which serializers must be registered by the the
 * class name. The @ref PII_INSTANTIATE_SERIALIZER macro registers a
 * serializer for an archive type. The @ref PII_INSTANTIATE_FACTORY
 * creates the factory, which is not specific to any archive type. But
 * before the factory and the serializers can be created we need to
 * name the type. This is done with the @ref PII_SERIALIZATION_NAME
 * macro.
 *
 * We need to instantiate the serializers and factories for all new
 * types derived from @p Car:
 *
 * @code
 * PII_SERIALIZATION_NAME(Car);
 * PII_INSTANTIATE_SERIALIZER(PiiTextInputArchive, Car);
 * PII_INSTANTIATE_SERIALIZER(PiiTextOutputArchive, Car);
 * PII_INSTANTIATE_FACTORY(Car);
 *
 * // Repeat for other types
 * @endcode
 *
 *
 * @section serialization_making_easy Making a Class Serializable the Easy Way
 *
 * Now that we are familiar with the concepts or registering
 * serializable types we can just forget the manual way of doing
 * things. All one really needs to do is to define some configuration
 * values and include a single file called @ref
 * core/serialization/PiiSerializableRegistration.h
 * "PiiSerializableRegistration.h". @p Car could be registered simply
 * this way:
 *
 * @code
 * #define PII_SERIALIZABLE_CLASS Car
 * #define PII_VIRTUAL_METAOBJECT
 * #include <PiiSerializableRegistration.h>
 * @endcode
 *
 * @ref core/serialization/PiiSerializableRegistration.h
 * "PiiSerializableRegistration.h" can be included as many times as
 * necessary.
 *
 * @section serialization_making_version Versioning
 *
 * It sometimes happens that a class changes format while one still
 * has to support previously stored archives. This is possible if a
 * class has the PiiSerializationTraits::ClassInfo trait turned on (it
 * is by default). In deserialization, the serialize() function will
 * be given the version number of the stored object. The Serialization
 * library checks that the version number of valid and will fail if it
 * is larger than the current version number of the object.
 *
 * Let us assume you add a @p _iDoors member to the @p Car class. The
 * new declaration and registration for the class is as follows:
 *
 * @code
 * class Car
 * {
 *   friend struct PiiSerialization::Accessor;
 *   PII_VIRTUAL_METAOBJECT_FUNCTION
 *
 *   // The serialization function
 *   template <class Archive> void serialize(Archive& archive, const unsigned int version)
 *     {
 *       PII_SERIALIZE(archive, _iWheels);
 *       // Serialize _iDoors only if the version number is not zero
 *       if (version > 0)
 *         PII_SERIALIZE(archive, _iDoors);
 *     }
 *
 *   int _iWheels;
 *   int _iDoors;
 * };
 *
 * #define PII_SERIALIZABLE_CLASS Car
 * #define PII_VIRTUAL_METAOBJECT
 * // The current version number of Car is 1
 * #define PII_SERIALIZABLE_CLASS_VERSION 1
 * #include <PiiSerializableRegistration.h>
 * @endcode
 *
 * Whenever you make changes that affect the archive format and still
 * want to support old archives, increase the version number. Note
 * that the Serialization uses only eight bits (256 different values)
 * to store the version number to save space. If you need more, store
 * the version number as an integer in the beginning of your
 * serialization function.
 *
 * @section non_default_constructors Non-default Constructors
 *
 * Although it is usually bad practice not to provide a default
 * constructor for a class, there may be situations where a
 * serializable class must be created with a non-default constructor.
 *
 * The serialization library uses the PiiSerialization::Constructor
 * struct to create new class instances. The default implementation
 * returns <tt>new Type</tt> for a non-abstract @p Type, and a null
 * pointer for abstract types. Specializing the
 * PiiSerialization::Constructor structure either fully or partially
 * makes it possible to change the way object instances are created.
 * For convenience, the @ref PII_SERIALIZATION_CONSTRUCTOR and @ref
 * PII_SERIALIZATION_NORMAL_CONSTRUCTOR macros are provided.
 */



/**
 * @page serialization_wrappers Serialization Wrappers
 *
 * If a type needs special handling in serialization, it is often
 * convenient to create a wrapper and write the serialization
 * functions for it. The Serialization library provides three built-in
 * wrappers: one for serializing binary data, another for storing
 * name-value pairs and the last one for serializing a base class. 
 * @ref core/serialization/PiiSerializationUtil.h
 * "PiiSerializationUtil.h" defines some handy macros for dealing with
 * serialization wrappers.
 *
 * @section serialization_wrappers_binary Binary Objects
 *
 * It is often necessary to store binary data. Since there is no way
 * to find the size of a data array given just a pointer, one needs to
 * use the PiiBinaryObject wrapper for serialization. PiiBinaryObject
 * needs a pointer to an array and the number of elements in it. Since
 * the wrapper does not know whether you are actually writing or
 * reading the data, you need to provide both values as references. In
 * reading, the values will be filled by the serialization library.
 *
 * @code
 * // Somewhere else
 * MyClass::MyClass() : _array(new float[5]) {}
 *
 * template <class Archive> void MyClass::serialize(Archive& ar, const unsigned int)
 *   {
 *     int bytes = 5*sizeof(float);
 *     ar & Pii::binaryObject(_array, bytes);
 *   }
 * @endcode
 *
 *
 * @section serialization_wrappers_nvp Name-Value Pairs
 *
 * Some archive formats such as XML accompany stored values with
 * names. By default, data is stored as such without tagging it in any
 * way. To support named data one should, however, always use the
 * PiiNameValuePair wrapper. Archive types that don't support named
 * data just ignore the name attribute.
 *
 * A name-value pair can be created in a number of different ways. The
 * easiest one is to use the @ref PII_NVP macro:
 *
 * @code
 * template <class Archive> void MyClass::serialize(Archive& ar, const unsigned int)
 *   {
 *     // _iValue is an int member
 *     ar & PII_NVP("value", _iValue);
 *     int elements = 5;
 *     // wrappers can be chained
 *     ar & PII_NVP("array", Pii::binaryObject(array, elements));
 *   }
 * @endcode
 *
 * To make variable naming easy, the library defines the @ref
 * PII_SERIALIZE and @ref PII_SERIALIZE_NAMED macros. If you want to
 * store a variable with its name as it appears in the class
 * declaration, just do this:
 *
 * @code
 * template <class Archive> void MyClass::serialize(Archive& ar, const unsigned int)
 *   {
 *     PII_SERIALIZE(ar, _iValue);
 *     PII_SERIALIZE_NAMED(ar, Pii::binaryObject(array, elements), "_array");
 *   }
 * @endcode
 *
 *
 * @section serialization_wrappers_baseclass Base Class
 *
 * PiiBaseClass is the last built-in wrapper. Its purpose is to invoke
 * the serialization of a base class as explained in @ref
 * serialization_making_base. The PiiBaseClass wrapper makes it
 * possible to use different serialization mechanisms for different
 * levels of inheritance hierarchy. There is no need to make the
 * serialization functions virtual and directly call the superclass'
 * implementation.
 *
 * @b Note. It is possible to skip parents in the hierarchy if you
 * want to override their default serialization behavior. For example,
 * B derives from A and C derives from B (A -> B -> C), it is possible
 * to invoke the serialization of A from C. It is also possible to
 * totally omit the parents.
 *
 */



/**
 * @page serialization_archives Dealing with Archives
 *
 * @section serialization_archives_impl Archive Implementations
 *
 * The Serialization library provides two implementations of the
 * archive interface. (Note that the word @e interface here refers to
 * a template interface, not a declaration of an abstract class.) 
 * Since input and output archives are separated, the result is four
 * classes, named PiiBinaryInputArchive, PiiBinaryOutputArchive,
 * PiiTextInputArchive, and PiiTextOutputArchive.
 *
 * Depending on archive "direction", the implementation must provide
 * operators for either reading (>>) or writing (<<) all primitive
 * types plus a fallback mechanism for all other types. The fallback
 * mechanism is a template function that the compiler selects if it
 * cannot find an explicit specialization. The archive implementations
 * should just redirect calls to the template function to
 * PiiInputArchive and PiiOutputArchive.
 *
 * The usage of the built-in archive types is easy:
 *
 * @code
 * MyClass obj;
 *
 * // Writing
 * QFile textFile(fileName);
 * textFile.open(QIODevice::WriteOnly);
 * PiiTextOutputArchive oa(&textFile);
 * oa << obj; // replace << with & if you wish
 *
 * // Reading
 * QFile textFile(fileName);
 * textFile.open(QIODevice::ReadOnly);
 * PiiTextInputArchive ia(&textFile);
 * ia >> obj;
 * @endcode
 *
 * Any QIODevice works as the output or input device. This makes it
 * easy to (de)marshal data across network connections, for example.
 *
 * @b Note. Reading and writing must occur in the same order. 
 * Furthermore, if you store an object as a pointer, you must read it
 * back as a pointer.
 *
 * @section serialization_archives_generic Generic Archive Types
 *
 * Since serialization functions are templates, serializers must be
 * bound to an archive type. Although this approach results in
 * efficient code, it has a drawback: one must know all supported
 * archive types at compile time. As a solution, archive
 * implementations with virtual serialization functions are provided
 * (PiiGenericInputArchive and PiiGenericOutputArchive). If
 * serializers are registered to the generic archives, any archive
 * implementing the interface (interface in the traditional sense) can
 * be used at run time. The downside is that serialization must go
 * through virtual function call resolution, which leads in less
 * efficient code. (It is still fast, don't worry.)
 *
 * The PiiGenericInputArchive::Impl and PiiGenericOutputArchive::Impl
 * provide an easy way of converting any archive type to a generic
 * archive. They derive from the generic archive interfaces and also
 * from an implementation of the template interface. The classes just
 * work as proxies that forward virtual function calls to the
 * corresponding implementations in the template interface.
 *
 * Generic versions of the basic archive types are provided as
 * typedefs: @ref PiiGenericTextInputArchive, @ref
 * PiiGenericTextOutputArchive, @ref PiiGenericBinaryInputArchive, and
 * @ref PiiGenericBinaryOutputArchive.
 */


/**
 * @page serialization_tracking Tracking
 *
 * The Serialization library tracks the memory address (a.k.a
 * pointers) of serialized objects to make sure objects are serialized
 * only once. Whenever saving of a tracked object is requested, the
 * library checks the memory address of the object. If it matches that
 * of a previously saved object, only a reference to the previous
 * object will be stored. When the objects are read back the library
 * makes sure all pointer and references remain valid independent of
 * the saving order. The programmer may however affect the performance
 * of deserialization by the saving order.
 *
 * If an object is first saved through a pointer, the Serialization
 * library has no way of knowing whether it will later be saved as an
 * object. Thus, if the object is later saved again as an object, it
 * will be saved again. When reading the archive, the object will be
 * first a memory location through the pointer. Later, the library
 * will notice that the object itself was stored. The contents of the
 * previously allocated object must be released. Better performance
 * would be achieved by first storing the object itself.
 *
 * To be able to preserve memory references, the Serialization library
 * must know the @e final location of deserialized objects. Therefore,
 * if a tracked object is deserialized into a temporary memory
 * location and subsequently moved to its final location, the move
 * must be told to the deserializer. The following example illustrates
 * this. Note that @ref core/serialization/PiiSerializationUtil.h
 * "PiiSerializationUtil.h" provides a function for saving QLists, so
 * you don't actually need to do this manually.
 *
 * @code
 * QList<MyObj> lst; // filled somehow
 *
 * // Read elements in sequence in your load() function:
 * for (int i=0; i<elementCount; i++)
 *   {
 *     MyObj obj;
 *     archive >> obj;
 *     lst << obj;
 *     // "archive" now thinks &obj is the address of the deserialized object.
 *     // If a pointer refers to this object, it must be updated
 *     archive.objectMoved(obj, lst.last());
 *   }
 * @endcode
 *
 * The PiiInputArchive::objectMoved() function updates all references
 * currently pointing to a certain memory address to a new address. If
 * you don't signal the move, any subsequently restored pointer to the
 * object will point to the local variable.
 *
 * Tracking can be selectively turned off with the
 * PiiSerializationTraits::Tracking trait.
 *
 * The tracking mechanism is implemented in PiiOutputArchive and
 * PiiInputArchive. The former uses PiiTrackedPointerHolder objects to
 * store pointers to tracked objects. Please refer to the
 * PiiSerialization::createTrackedPointerHolder() and
 * PiiSerialization::rereferencePointer() functions if you want to
 * serialize reference-counted objects or customize the tracking
 * behaviour in other ways.
 */

/**
 * @page serialization_errors Handling Errors
 *
 * Errors in serialization are handled with exceptions. Typically, the
 * serialize() functions do not catch serialization exceptions but let
 * them flow to the caller. Most exceptions are thrown by the
 * archives, but it is possible to generate custom errors in
 * serialization functions. The type used for serialization errors is
 * PiiSerializationException.
 *
 * Custom serialization errors are most conveniently handled with the
 * @ref PII_SERIALIZATION_CUSTOM_ERROR macro declared in @ref
 * core/serialization/PiiSerializationException.h
 * "PiiSerializationException.h".
 *
 * @code
 * template <class Archive> void MyClass::serialize(Archive& ar, const unsigned int)
 *   {
 *     PII_SERIALIZATION_CUSTOM_ERROR("MyClass is not serializable!");
 *   }
 * @endcode
 *
 * Errors must be handled in the application that invokes
 * serialization code by catching PiiSerializationException. The error
 * code tells the reason of failure. If you want textual output, use
 * the @ref PiiSerializationException::message() "message()" function. 
 * PiiSerializationException also has a @ref
 * PiiSerializationException::info() "info()" function that returns
 * extra information for debugging purposes. It is filled with a class
 * name when no serializer or no factory is found for an object.
 *
 * @code
 * MyClass obj;
 * QFile textFile(fileName);
 * textFile.open(QIODevice::WriteOnly);
 * PiiTextOutputArchive ar(&textFile);
 *
 * try
 *   {
 *     PII_SERIALIZE(ar, obj);
 *   }
 * catch (PiiSerializationException& ex)
 *   {
 *     qDebug("Error code: %d\nMessage: %s",
 *            ex.code(), qPrintable(ex.message()));
 *   }
 * @endcode
 *
 * Note that the archive may not always be able to find out the real
 * reason of an error. If stored data is corrupted, it may in
 * principle result in any type of an error. @p InvalidDataFormat is
 * only thrown when the archive notices it is reading something it
 * isn't expecting.
 */

/**
 * @page serialization_archive_format Interpreting Archived Data
 *
 * The format of an archive depends on the archive type being used,
 * but serializable objects do not need to care about the underlying
 * format. Sometimes, it is however useful to know a bit more about
 * generic structures present in all archive formats. This section
 * describes how PiiOutputArchive stores data, independent of archive
 * format. The following (loosely) describes the contents of an
 * archive in EBNF notation.
 *
@verbatim
archive           ::= header data*
data              ::= object | pointer
object            ::= trackedObject | nonTrackedObject
trackedObject     ::= pointerIndex classData
nonTrackedObject  ::= classData
pointer           ::= trackedPointer | nonTrackedPointer
trackedPointer    ::= pointerIndex className classData
nonTrackedPointer ::= className classData
classData         ::= version? data* parentClassData* data*
parentClassData   ::= classData
@endverbatim
 *
 * @lip header - each archive implementation has its own header,
 * which may include any data. The header should contain a magic
 * string/number to identify archive format, and major and minor
 * archive version numbers.
 *
 * @lip pointerIndex - the archive keeps track of addresses to saved
 * objects. Whenever a tracked object is saved, its address will be
 * inserted to a tracking list. If a reference to the same memory
 * location is found, the pointer object will not be saved again. 
 * Instead, an index to the list will be saved. -1 means a null
 * pointer.
 *
 * @lip className - the name of a class as set by
 * PiiSerializationTraits::ClassName. If a serializable type is not
 * named, an empty string will be stored. "0" means a null pointer. In
 * a text archive, string is preceded by its length. Thus, an empty
 * string is stored as "0". A null pointer is "1 0" (length 1,
 * contents "0").
 *
 * @lip version - the version number (see
 * PiiSerializationTraits::Version) always precedes class data. The
 * version number is stored only if the
 * PiiSerializationTraits::ClassInfo trait is set to @p true.
 *
 * @lip parentClassData - the parent class' data, usually saved with
 * @ref PII_SERIALIZE_BASE. It is up to the programmer to actually
 * save the parent class/classes, and their data can be preceded by
 * other data, although this is usually a bad practice.
 *
 * Now, consider the following example, taken from the beginning of a
 * real archive:
 *
@verbatim
Into Txt 1 0
0 9 PiiEngine 1 0 0 0
0 11 1 15 PiiImageCropper 0 0 0
5 10 objectName 10 7 cropper
@endverbatim
 *
 * The first line is the header, which identifies the archive as a
 * text archive. The major and minor version numbers are 1 and 0.
 *
 * The second line is a tracked pointer, whose tracking index is zero.
 * Its class name (PiiEngine) is nine characters long. The class name
 * is followed by its version number (1). The following three numbers
 * are base class version numbers (PiiOperationCompound, PiiOperation,
 * and QObject). The last two parent classes store no data.
 *
 * On the third line, PiiOperationCompound has serialized its internal
 * list of operations. The first integer (0) is the version number of
 * QList, and 11 is the number of operations on the list. The first
 * one is a tracked pointer (tracking index one) with a 15-character
 * class name (PiiImageCropper). Its version number is zero, and so
 * are the versions of its bases (PiiOperation and QObject). Note that
 * serialization of PiiImageCropper skips a couple of intermediate
 * classes.
 *
 * On the last line, PiiImageCropper has stored its properties. It has
 * five of them, and the first one is named "objectName" (10 chars).
 * The value of the property is a QVariant, whose type id is 10
 * (QString). The string is "cropper". The rest of the properties and
 * operations are stored in a similar manner.
 *
 * It is easy to see that interpreting the archive without knowledge
 * of the actual data types being stored is a tedious task. One may
 * need the source code to decrypt the storage format. In fact, text
 * archives aren't even as clear as shown here. They don't really have
 * linefeeds to aid interpretation.
 */

/**
 * @page serialization_gotchas Common Problems
 *
 * The most common problems with the serialization library are caused
 * by wrong declaration order. It is sometimes tricky to find correct
 * ordering. The following tips may help if you run into trouble.
 *
 * @li Include PiiSerializationUtil.h first. It contains serializers
 * for many %Qt types that you will need in many cases. If a
 * serialization code later refers to a %Qt type whose serializer
 * hasn't been declared, compilation will fail with error messages
 * such as "Class 'QObject' has no member 'serialize'".
 *
 * @li The same applies to any other type: the serializer must be
 * declared before the serialization headers are included. If you
 * can't serialize a PiiMatrix, for example, include
 * PiiMatrixSerialization.h first.
 *
 * @li The general rule of thumb to avoid declaration order problems:
 * always include archive headers
 * (<tt>Pii{Generic,Text,Binary},{Input,Output}Archive.h</tt>) and
 * files that include archive headers (@p
 * PiiSerializableRegistration.h and @p PiiSerializableExport.h) last. 
 * Otherwise, serialization functions may not find the declarations
 * for serializing custom objects and fall back to default
 * serialization. Symptom: the serialization library "cannot find
 * serializer".
 *
 * @li Do not serialize temporary tracked objects. If you serialize a
 * pointer to a tracked object and delete it while the archive still
 * exists, chances are that another tracked object will later get the
 * same memory address on the heap. This plays havoc with the tracking
 * mechanism. See PiiTrackedPointerHolder on how to work around
 * problems with automatic deletion of reference-counted objects.
 */
