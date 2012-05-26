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

#ifndef _PIIASYNCCALL_H
#define _PIIASYNCCALL_H

#include <QThread>
#include "PiiGlobal.h"
#include "PiiTypeTraits.h"

/**
 * A utility class for calling functions asynchronously. %PiiAsyncCall
 * is a thread that invokes a member function of a class from a new
 * thread. It can be used to conveniently throw long-lasting processes
 * into threads without deriving from QThread. This class is
 * especially useful if you need to implement many threaded functions
 * in a single class or want to add threaded processing to an object
 * that already derives from QObject.
 *
 * This class is not intended to be used directly. Use the
 * Pii::asyncCall() and Pii::createAsyncCall() functions to
 * conveniently call functions asynchronously.
 *
 * @ingroup Core
 */
template <class Object, class Function> class PiiAsyncCall : public QThread
{
public:
  void setAutoDelete()
  {
    connect(this, SIGNAL(finished()), SLOT(deleteLater()), Qt::DirectConnection);
  }
    
protected:
  /**
   * Create a new asynchronous call thread that calls @p function on
   * @p object.
   *
   * @param obj the object whose member function is to be called
   *
   * @param func the address of the function to be called
   *
   * You probably won't ever use this constructor directly. Use the
   * @ref Pii::asyncCall() helper function instead.
   */
  PiiAsyncCall(Object obj, Function function) :
    _pObject(obj), _pFunction(function)
  {}

  Object _pObject;
  Function _pFunction;
};

/// @cond null
#define PII_ASYNC_TPL_PARAM(N, PARAM) , class PARAM
#define PII_ASYNC_TPL_IMPL(N, PARAM) , PARAM
#define PII_ASYNC_CTR_PARAM(N, PARAM) , PARAM PII_JOIN(__, PARAM)
#define PII_ASYNC_INIT_PARAM(N, PARAM) , PII_JOIN(_, PARAM)(PII_JOIN(__, PARAM))
#define PII_ASYNC_FUNC_PARAM(N, PARAM) PII_JOIN(_, PARAM)
#define PII_ASYNC_FUNC2_PARAM(N, PARAM) , PII_JOIN(__, PARAM)
#define PII_ASYNC_MEMBER(N, PARAM) typename Pii::RefToValue<PARAM>::Type PII_JOIN(_, PARAM);

#define PII_CREATE_ASYNCCALL(PARAMCNT, PARAMS)                          \
  template <class Object, class Function                                \
            PII_FOR_N(PII_ASYNC_TPL_PARAM, PARAMCNT, PARAMS)>           \
  class PII_JOIN(PiiAsyncCall, PARAMCNT) : public PiiAsyncCall<Object, Function> { \
  public:                                                               \
    PII_JOIN(PiiAsyncCall, PARAMCNT)(Object object, Function function   \
                                     PII_FOR_N(PII_ASYNC_CTR_PARAM, PARAMCNT, PARAMS)) : \
      PiiAsyncCall<Object, Function>(object, function)                  \
      PII_FOR_N(PII_ASYNC_INIT_PARAM, PARAMCNT, PARAMS)                 \
    {}                                                                  \
  protected:                                                            \
    void run() { (this->_pObject->*(this->_pFunction))(PII_FOR_N_SEP(PII_ASYNC_FUNC_PARAM, PII_COMMA_SEP, PARAMCNT, PARAMS)); } \
  private:                                                              \
    PII_FOR_N(PII_ASYNC_MEMBER, PARAMCNT, PARAMS)                       \
  };                                                                    \
  namespace Pii {                                                       \
    template <class Object, class Function PII_FOR_N(PII_ASYNC_TPL_PARAM, PARAMCNT, PARAMS)> \
    QThread* createAsyncCall(Object obj, Function func PII_FOR_N(PII_ASYNC_CTR_PARAM, PARAMCNT, PARAMS)) {               \
      return new PII_JOIN(PiiAsyncCall, PARAMCNT)<Object, Function PII_FOR_N(PII_ASYNC_TPL_IMPL, PARAMCNT, PARAMS)> \
        (obj, func PII_FOR_N(PII_ASYNC_FUNC2_PARAM, PARAMCNT, PARAMS)); \
    }                                                                   \
    template <class Object, class Function PII_FOR_N(PII_ASYNC_TPL_PARAM, PARAMCNT, PARAMS)> \
    QThread* asyncCall(Object obj, Function func PII_FOR_N(PII_ASYNC_CTR_PARAM, PARAMCNT, PARAMS)) {                     \
      PiiAsyncCall<Object, Function>* call = new PII_JOIN(PiiAsyncCall, PARAMCNT)<Object, Function PII_FOR_N(PII_ASYNC_TPL_IMPL, PARAMCNT, PARAMS)> \
        (obj, func PII_FOR_N(PII_ASYNC_FUNC2_PARAM, PARAMCNT, PARAMS)); \
      call->setAutoDelete();                                            \
      call->start();                                                    \
      return call;                                                      \
    }                                                                   \
  }
  

PII_CREATE_ASYNCCALL(0, ());
PII_CREATE_ASYNCCALL(1, (P1));
PII_CREATE_ASYNCCALL(2, (P1, P2));
PII_CREATE_ASYNCCALL(3, (P1, P2, P3));
PII_CREATE_ASYNCCALL(4, (P1, P2, P3, P4));
PII_CREATE_ASYNCCALL(5, (P1, P2, P3, P4, P5));
PII_CREATE_ASYNCCALL(6, (P1, P2, P3, P4, P5, P6));
PII_CREATE_ASYNCCALL(7, (P1, P2, P3, P4, P5, P6, P7));
PII_CREATE_ASYNCCALL(8, (P1, P2, P3, P4, P5, P6, P7, P8));

/// @endcond

/**
 * @fn template <class Object, class Function> QThread* Pii::asyncCall(Object object, Function function, ...)
 *
 * Calls a function asynchronously from another thread.
 *
 * @param obj the address of the object whose member function is to
 * be called. You may also use a reference type if the type
 * overrides @p operator->*.
 *
 * @param func the address of the function to be called
 *
 * @return a pointer to a new QThread object. The thread will be
 * automatically deleted once it is finished and control returns to
 * the event loop of the calling thread.
 *
 * @code
 * struct MyStruct
 * {
 *   void func() { std::cout << "Moi!" << std::endl; }
 *   void func2(const char* message) { std::cout << message << std::endl; }
 * };
 *
 * MyStruct s;
 * Pii::asyncCall(&s, &MyStruct::func);
 * Pii::asyncCall(&s, &MyStruct::func2, message);
 * // Must have event loop in this thread
 * @endcode
 *
 * @note Make sure the object pointer is valid in the context of the
 * new thread. Do not pass a pointer to an object on the stack of
 * the calling thread unless you ensure that the async call finishes
 * during the lifetime of the object.
 *
 * @note All pararameters passed to the asynchronous function must be
 * copyable.
 *
 * @note %PiiAsyncCall can call private functions. If you are
 * allowed to take the pointer of a virtual function (i.e. you are
 * in the class itself or in a friend class/function) %PiiAsyncCall
 * will be able to use it.
 *
 * @relates PiiAsyncCall
 */

 
/**
 * @fn template <class Object, class Function> QThread* Pii::createAsyncCall(Object object, Function function, ...)
 *
 * Creates a thread that will call a function asynchronously.
 *
 * @code
 * struct MyStruct
 * {
 *   void func() { std::cout << "Moi!" << std::endl; }
 * };
 *
 * MyStruct s;
 * QThread *pThread = Pii::createAsyncCall(&s, &MyStruct::func);
 *
 * // No automatic delete -> must control the thread myself
 * pThread->start();
 * pThread->wait();
 * delete pThread;
 * @endcode
 *
 * @relates PiiAsyncCall
 */


#endif //_PIIASYNCCALL_H
