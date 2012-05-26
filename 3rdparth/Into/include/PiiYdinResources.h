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

#ifndef _PIIYDINRESOURCES_H
#define _PIIYDINRESOURCES_H

#include <PiiSerializationTraits.h>
#include "PiiResourceConnector.h"
// Force the definitions of essential types always before
// PiiDynamicTypeFunctions.
#include "PiiEngine.h" 
#include <PiiDynamicTypeFunctions.h>

/**
 * @file
 *
 * Functions for creating resources described in the @ref
 * PiiYdin::resourceDatabase() "resource database". This header file
 * must be included after all the types involved in the function calls
 * have been declared.
 *
 * @code
 * #include <PiiYdinResources.h>
 * #include "MyInterface.h" // defines MyInterface
 *
 * //...
 * // This will NOT work because the functions in PiiYdinResources.h
 * // do not see the declarations related to MyInterface.
 * MyInterface* pIface = PiiYdin::createResource<MyInterface>("MyInterfaceImpl");
 * @endcode
 *
 * @code
 * #include "MyInterface.h" // defines MyInterface
 * #include <PiiYdinResources.h>
 *
 * //...
 * // This works because MyInterface is fully defined before createResource().
 * MyInterface* pIface = PiiYdin::createResource<MyInterface>("MyInterfaceImpl");
 * @endcode
 */
template <class T, class U> T resource_cast(U* resource);

namespace PiiYdin
{
  /**
   * Get the resource name of an object given a pointer to it. This
   * function uses the serialization meta object system to find the
   * class name of the most derived class, given a base class pointer.
   */
  template <class T> inline const char* resourceName(const T* resource)
  {
    return PII_GET_METAOBJECT(*resource).className();
  }

  /**
   * Get the resource name of a type. This function uses
   * PiiSerializationTraits to find the name for the type.
   *
   * @code
   * class MyClass {};
   * PII_SERIALIZATION_NAME(MyClass);
   *
   * // Returns "MyClass"
   * const char* name = PiiYdin::resourceName<MyClass>();
   * @endcode
   */
  template <class T> inline const char* resourceName()
  {
    return PiiSerializationTraits::ClassName<T>::get();
  }

  /// @internal
  PII_YDIN_EXPORT int pointerOffset(const char* superClass, const char* subClass);
  
  /**
   * Create an instance of the object identified by @a resourceName. 
   * This function uses PiiSerializationFactory to create the object,
   * and returns a pointer to the new instance. The @p ParentType
   * template parameter is used in checking that the resource really
   * is an instantiable entity. If @p ParentType is not a superclass
   * of the resource identified by @a name, the function will return a
   * null pointer. Consequently, @p ParentType must have the @ref
   * PiiSerializationTraits::ClassName "class name trait" defined. A
   * null pointer will be returned also if the named resource does not
   * exist.
   *
   * The code below creates an instance of PiiImageFileReader, but
   * only if such a resource exists in the resource database with a @p
   * pii:class attribute that equals "PiiOperation". It is not needed
   * to specify all possible superclasses for a class; superclasses of
   * superclasses will be recursively resolved.
   *
   * @code
   * PiiOperation* pOperation = PiiYdin::createResource<PiiOperation>("PiiImageFileReader");
   *
   * class MyInterface
   * {
   * public:
   *   virtual void virtualFunc() const = 0;
   * };
   *
   * // Cannot instantiate this class
   * PII_SERIALIZATION_ABSTRACT(MyInterface);
   * // Make the type name known to Into
   * PII_SERIALIZATION_NAME(MyInterface);
   *
   * class MyClass : public MyInterface
   * {
   * public:
   *   void virtualFunc() const;
   * };
   *
   * PII_IMPLEMENT_PLUGIN(MyPlugin);
   *
   * PII_REGISTER_CLASS(MyClass, MyInterface);
   *
   * // ...
   *
   * MyInterface* iface = PiiYdin::createResource<MyInterface>("MyClass");
   *
   * // Returns 0 because MyClass is not a QObject
   * QObject* obj = PiiYdin::createResource<QObject>("MyClass");
   * @endcode
   *
   * @see resourceName()
   * @see resourceDatabase()
   */
  template <class ParentType> ParentType* createResource(const char* name)
  {
    PiiSerializationFactory* pFactory = PiiSerializationFactory::factory(name);
    
    // Don't know how to create this resource.
    if (pFactory == 0)
      return 0;

    // Find the offset from the beginning of the named type to that of
    // ParentType.
    int iOffset = pointerOffset(resourceName<ParentType>(), name);
    // -1 means ParentType is not a superclass.
    if (iOffset == -1)
      return 0;

    return reinterpret_cast<ParentType*>(static_cast<char*>(pFactory->create()) +
                                         iOffset);
  }

  /**
   * @overload
   */
  template <class ParentType> inline ParentType* createResource(const QString& name)
  {
    return createResource<ParentType>(qPrintable(name));
  }

  /**
   * Returns the name of a PiiResourceConnector resource that can bind
   * the two named resources together as @a role. If no such connector
   * exists, returns an empty string.
   *
   * @relates PiiResourceConnector
   */
  PII_YDIN_EXPORT QString resourceConnectorName(const QString& resource1Name,
                                                const QString& resource2Name,
                                                const QString& role);

  /**
   * Creates a resource connector that can bind the two resources
   * together as @a role. If the resources have not been assigned a
   * connection named @a role or the connection has no associated
   * connector, 0 will be returned. The function may return either a
   * newly allocated connector object (in which case @a mustDelete
   * flag will be set to @p true) or either of the two resources (@a
   * mustDelete will be @p false).
   *
   * @relates PiiResourceConnector
   */
  template <class T, class U>
  PiiResourceConnector* createResourceConnector(T* resource1, U* resource2, const QString& role, bool* mustDelete)
  {
    QString strConnectorName = resourceConnectorName(resourceName(resource1),
                                                     resourceName(resource2),
                                                     role);
    
    *mustDelete = false;
    if (strConnectorName.isEmpty())
      return 0;
    else if (strConnectorName == "pii:object")
      return resource_cast<PiiResourceConnector*>(resource2);
    else if (strConnectorName == "pii:subject")
      return resource_cast<PiiResourceConnector*>(resource1);
    *mustDelete = true;
    return createResource<PiiResourceConnector>(qPrintable(strConnectorName));
  }
  
  /**
   * Connect @a resource1 and @a resource2 as specified by @a role.
   *
   * @param resource1 a pointer to a class instance, such as
   * PiiOperation. The resource name of type @p T must be resolvable
   * with #resourceName().
   *
   * @param resource2 a pointer to a class instance, such as QWidget. 
   * The resource name of type @p U must be resolvable with
   * #resourceName().
   *
   * @param role the type of connection between the resources, such as
   * "pii:display".
   *
   * @return @p true if the resources were successfully connected, @p
   * false otherwise. The connection will fail if the name of one of
   * the resources cannot be determined or if there is no connector
   * (see PiiYdin::resourceDatabase()) associated with the two
   * resources in the specified @a role.
   *
   * @code
   * PiiOperation* pOperation = PiiYdin::createResource<PiiOperation>("PiiVisualTrainer");
   * QWidget* pDisplay = PiiYdin::createResource<QWidget>("PiiVisualTrainerWidget");
   * PiiYdin::connectResources(pOperation, pDisplay, "pii:configurator");
   * @endcode
   *
   * @relates PiiResourceConnector
   */
  template <class T, class U> bool connectResources(T* resource1, U* resource2, const QString& role)
  {
    bool bMustDelete;
    PiiResourceConnector* pConnector = createResourceConnector(resource1, resource2, role, &bMustDelete);
    if (pConnector == 0)
      return false;
    bool bResult = pConnector->connectResources(resource1, resource2, role);
    if (bMustDelete)
      delete pConnector;
    return bResult;
  }

  /**
   * Disconnect two resources. The behavior is similar to
   * #connectResources(), except that an existing connection (if any)
   * between the two resources will be broken.
   *
   * @relates PiiResourceConnector
   */
  template <class T, class U> bool disconnectResources(T* resource1, U* resource2, const QString& role)
  {
    bool bMustDelete;
    PiiResourceConnector* pConnector = createResourceConnector(resource1, resource2, role, &bMustDelete);
    if (pConnector == 0)
      return false;
    bool bResult = pConnector->disconnectResources(resource1, resource2, role);
    if (bMustDelete)
      delete pConnector;
    return bResult;
  }
}

/**
 * Cast @a resource to the type specified by @p T. This function uses
 * the resource database to find out if @p T is a superclass of @a
 * resource. It returns the address of @a resource as @p T on success
 * and zero on failure. The behaviour is similar to @p qobject_cast,
 * but it doesn't use Qt's meta-object system and works with classes
 * that are not derived from QObject.
 *
 * @code
 * QWidget* pWidget = PiiYdin::createResource<QWidget>("PiiVisualTrainerWidget");
 * PiiResourceConnector* pConnector = resource_cast<PiiResourceConnector*>(pWidget);
 * if (pConnector != 0)
 *   cout << "PiiVisualTrainerWidget derives from PiiResourceConnector.\n";
 * @endcode
 *
 * @see resourceDatabase()
 */
template <class T, class U> T resource_cast(U* resource)
{
  typedef typename Pii::PtrToValue<T>::Type TValue;
  int iOffset = PiiYdin::pointerOffset(PiiYdin::resourceName<TValue>(), PiiYdin::resourceName(resource));
  if (iOffset == -1)
    return 0;
  return reinterpret_cast<T>(reinterpret_cast<char*>(resource) +
                             iOffset);
}

#endif //_PIIYDINRESOURCES_H
