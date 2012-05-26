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

#ifndef _PIIFLOWCONTROLLER_H
#define _PIIFLOWCONTROLLER_H

#include "PiiExecutionException.h"
#include "PiiYdinTypes.h"

#include <QList>

class PiiInputSocket;
class PiiOutputSocket;

/**
 * An interface for algorithms that synchronize object flow in
 * sockets. Instances of this class are used by PiiDefaultOperation to
 * control the flow of objects through operations.
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiFlowController
{
public:
  /**
   * Syncronization event. Instances of this class are passed to the
   * SyncListener of a PiiFlowController.
   */
  class SyncEvent
  {
  public:
    /**
     * Event types.
     *
     * @lip StartInput - the specified group, or one of its child
     * groups is about to receive an object. This type of event will
     * be sent just before an input group is activated, but only if
     * the group has child groups.
     *
     * @lip EndInput - all child groups of the specified group have
     * got all objects related to the last object in the parent group.
     * This event indicates that there will be no more objects to be
     * handled before the next object in the parent group.
     */
    enum Type { StartInput, EndInput };
    
    SyncEvent(Type type = StartInput, int groupId = 0) : _type(type), _iGroupId(groupId) {}

    /**
     * Get the type of the event.
     */
    Type type() const { return _type; }
    /**
     * Get the id of the input socket group for which the event was
     * activated.
     */
    int groupId() const { return _iGroupId; }
    
  private:
    Type _type;
    int _iGroupId;
  };
  
  /**
   * An interface for objects that receive synchronization events from
   * the flow controller.
   */
  class PII_YDIN_EXPORT SyncListener
  {
  public:
    virtual ~SyncListener() {}

    /**
     * Calls the protected #syncEvent() function and ensures no
     * successive events of the same type are delivered to any input
     * group. This is necessary to make it possible to add operations
     * into a configuration during pause.
     */
    inline void sendEvent(SyncEvent* event)
    {
      // See if the group has been started
      int index = _lstStartedGroups.indexOf(event->groupId());

      // Nope... We can accept start event.
      if (index == -1 && event->type() == SyncEvent::StartInput)
        {
          _lstStartedGroups << event->groupId();
          syncEvent(event);
        }
      // Yes... Only end is allowed.
      else if (event->type() == SyncEvent::EndInput && index != -1)
        {
          _lstStartedGroups.removeAt(index);
          syncEvent(event);
        }
    }

    void reset()
    {
      _lstStartedGroups.clear();
    }

  protected:
    /**
     * Informs the listener about a synchronization event, such as
     * start or end of input.
     *
     * @see PiiDefaultOperation::syncEvent()
     */
    virtual void syncEvent(SyncEvent* event) = 0;

  private:
    QList<int> _lstStartedGroups;
  };
  
  virtual ~PiiFlowController();
  
  /**
   * Flow states.
   *
   * @lip IncompleteState - processing is not possible because no
   * group of synchronized input sockets that could be handled now is
   * completely filled.
   *
   * @lip ProcessableState - processing can be performed because a
   * group of synchronized input is full. There may be more objects to
   * be processed, and the caller should send sync events, process the
   * objects and try #prepareProcess() again.
   *
   * @lip SynchronizedState - the operation received a start/end tag. 
   * There is nothing to be processed now, but the caller should try
   * #prepareProcess() again as with @p ProcessableState.
   *
   * @lip PausedState - the operation should be paused
   *
   * @lip FinishedState - the operation should fully stop processing
   *
   * @lip ResumedState - the operation was resumed after pause and
   * should continue where ever it was left.
   */
  enum FlowState
    {
      IncompleteState,
      ProcessableState,
      SynchronizedState,
      PausedState,
      FinishedState,
      ResumedState
    };

  /**
   * Type bit masks for input objects. These values are used by
   * #inputGroupTypeMask() when checking the state of a group of
   * synchronized input sockets.
   *
   * @lip NoObject - not all inputs in the group have been filled yet
   *
   * @lip NormalObject - all inputs contain a normal object to be
   * processed.
   *
   * @lip EndTag - all inputs contain a synchronization end tag. This
   * indicates that all objects related to an object at a lower flow
   * level have been received.
   *
   * @lip StartTag - all inputs contain a synchronization start tag.
   * This indicates that a set of objects related to an object at a
   * lower flow level are going to be received.
   *
   * @lip StopTag - all inputs contain a stop tag. If all groups
   * contain stop tags, the operation should be stopped.
   *
   * @lip PauseTag - all inputs contain a pause tag. If all groups
   * contain pause tags, the operation should be paused.
   *
   * @lip ResumeTag - all inputs contain a resume tag. Resume tags are
   * used to restore flow levels after pausing. Their functioning is
   * similar to a start tag.
   */
  enum InputGroupType
    {
      NoObject = 0,
      NormalObject = 1,
      EndTag = 2,
      StartTag = 4,
      StopTag = 8,
      PauseTag = 16,
      ResumeTag = 32
    };

  /**
   * Check a group of synchronized input sockets.
   *
   * @param begin an interator to the first input to be checked. 
   * Typically, this value is returned by
   * QVector<PiiInputSocket*>::begin(). The iterator must point to a
   * PiiInputSocket*.
   *
   * @param end an iterator the the last input to be checked
   *
   * @return a logical OR composition of #InputGroupType masks. If the
   * returned value is none of the values listed in InputGroupType,
   * the group's synchronization is in error.
   *
   * @note In some rare occasions, it may happen that the type mask is
   * (legally) a composition of @p PauseTag and @p NormalObject, or @p
   * ResumeTag and @p NormalObject. This function automatically
   * resolves such situations by modifying the object queues in input
   * sockets, if possible. If a resolution is found, the function
   * returns @p PauseTag or @p ResumeTag, and leaves some input
   * objects in the queues.
   */
  template <class InputIterator> static inline int inputGroupTypeMask(InputIterator begin, InputIterator end);

  /**
   * A utility function that creates a textual dump of incoming
   * objects in a group of sockets. Can be used to create descriptive
   * error messages for synchronization errors.
   *   
   * @param begin an interator to the first input to be checked. 
   * Typically, this value is returned by
   * QVector<PiiInputSocket*>::begin(). The iterator must point to a
   * PiiInputSocket*.
   *
   * @param end the end of the iterable sequence
   *
   * @param queueIndex the index of the object to be dumped. The
   * default value is 1, which dumps the first incoming objects. Use 0
   * to dump the objects currently at the head of the queue.
   */
   template <class InputIterator>
   static QString dumpInputObjects(InputIterator begin, InputIterator end, int queueIndex = 1);
  
  /**
   * Prepare the sockets for processing. This function is called by
   * PiiOperationProcessor just before it starts a processing round. 
   * If processing can be performed, this function places the
   * necessary objects into the input sockets.
   *
   * If you reimplement this function yourself, you must conform to
   * the following:
   *
   * @li If any of the connected sockets in a group does not contain
   * an object, @p IncompleteState must be returned.
   *
   * @li If all inputs contain sync tags, they need to be passed to
   * synchronized outputs (@ref PiiOutputSocket::emitObject()) and @p
   * SynchronizedState must be returned. The inputs must be
   * released (@ref PiiInputSocket::release()).
   *
   * @li If all inputs contain stop/pause/resume tags, the operation
   * must be either stopped, paused or resumed. The function must
   * return @p FinishedState, @p PausedState, or @p ResumedState,
   * respectively. The caller is responsible for passing the control
   * objects in these cases. PiiOperationProcessor uses
   * PiiBasicOperation::operationStopped(),
   * PiiBasicOperation::operationPaused(), and
   * PiiBasicOperation::operationResumed() helper functions for this. 
   * The inputs must be released.
   *
   * @li If all inputs in a group contain ordinary objects
   * (PiiYdin::isNonControlType()), move objects to be processed from
   * incoming to outgoing slots with PiiInputSocket::release() and
   * return @p ProcessableState. Only one group can be processed
   * at once. The caller is responsible for re-invoking this function
   * to make sure all processable groups will be handled. Set the @ref
   * setActiveInputGroup() "active group" to the group id of the
   * active group.
   *
   * @li If a group of synchronized inputs contains mixed inputs (e.g. 
   * ordinary objects and sync tags), a PiiExecutionException must be
   * thrown.
   *
   * @li Synchronization events (if any) should be queued. The
   * #sendSyncEvents(SyncListener*) function empties the queue.
   *
   * @return If all inputs contain stop, pause, or resume tags, either
   * @p FinishedState or @p PausedState, or @p ResumedState will be
   * returned. Otherwise, returns @p ProcessableState when a
   * processing round can be performed and @p IncompleteState
   * when not. Processing can be performed if a group of synchronized
   * sockets is filled with ordinary objects. If a group is filled
   * with synchronization objects, @p SynchronizedState will be
   * returned.
   *
   * @exception PiiExecutionException& if a synchronization error
   * occurs or the listener's @p inputInSync() function throws.
   */
  virtual FlowState prepareProcess() = 0;

  /**
   * Send queued sync events to @p listener. Empty the event queue.
   * The default implementation does nothing.
   */
  virtual void sendSyncEvents(SyncListener* listener);

  /**
   * Get the ID of the currently active synchronization group.
   */
  int activeInputGroup() const;

protected:
  /// @internal
  class Data
  {
  public:
    Data();
    virtual ~Data();
    
    /// The ID of the currently active sync group.
    int iActiveInputGroup;
  } *d;
  
  /**
   * Construct a new flow controller.
   */
  PiiFlowController();

  PiiFlowController(Data* data);

  void setActiveInputGroup(int group);

  /**
   * Utility function for translating a message.
   */
  static QString tr(const char* msg) { return QCoreApplication::translate("PiiFlowController", msg); }

private:
  template <class InputIterator> static bool resolvePausedState(int type, InputIterator begin, InputIterator end);
};

// Create a descriptive message for a synchronization error.
template <class InputIterator>
QString PiiFlowController::dumpInputObjects(InputIterator begin, InputIterator end, int queueIndex)
{
  QString msg;
  for (; begin != end; ++begin)
    {
      QString objType;
      PiiVariant obj = (*begin)->objectAt(queueIndex);
      if (!obj.isValid())
        objType = "<null>";
      else
        {
          switch (obj.type())
            {
            case PiiYdin::StopTagType:
              objType = tr("<stop tag>");
              break;
            case PiiYdin::PauseTagType:
              objType = tr("<pause tag>");
              break;
            case PiiYdin::ResumeTagType:
              objType = tr("<resume tag>");
              break;
            case PiiYdin::SynchronizationTagType:
              objType = obj.valueAs<int>() < 0 ? tr("<synchronization end tag>") : tr("<synchronization start tag>");
              break;
            default:
              objType = tr("ordinary object, type id 0x%1").arg(obj.type(), 0, 16);
            }
        }
      msg += tr("Object in \"%1\" input is %2.\n").arg((*begin)->objectName()).arg(objType);
    }
  return msg;
}


template <class InputIterator>
int PiiFlowController::inputGroupTypeMask(InputIterator begin, InputIterator end)
{
  int typeMask = NoObject;
  for (InputIterator i=begin; i != end; ++i)
    {
      unsigned int uiType = (*i)->typeAt(1);
      if (uiType == PiiVariant::InvalidType)
        return NoObject; // this group is not full yet

      if (PiiYdin::isNonControlType(uiType))
        typeMask |= NormalObject;
      else if (uiType == PiiYdin::SynchronizationTagType)
        // Value is either -1 or 1. Thus, we OR with either 2 or 4
        typeMask |= (*i)->objectAt(1).template valueAs<int>() + 3;
      else if (uiType == PiiYdin::StopTagType)
        typeMask |= StopTag;
      else if (uiType == PiiYdin::PauseTagType)
        typeMask |= PauseTag;
      else // if (uiType == PiiYdin::ResumeTagType)
        typeMask |= ResumeTag;
    }

  // Special case: pause tags and normal objects mixed. This can
  // happen if an operation paused while emitting a set of subobjects
  // between startMany() and endMany() calls AND another operation
  // delayed the emission of one object until the end of the
  // subobjects. The same happens when resuming from such a situation.
  if (typeMask == (PauseTag | NormalObject))
    {
      // Try to resolve the situation
      if (resolvePausedState(PiiYdin::PauseTagType, begin, end))
        return PauseTag;
      // No luck... Maybe we need to wait for more objects.
      return NoObject;
    }
  else if (typeMask == (ResumeTag | NormalObject))
    {
      if (resolvePausedState(PiiYdin::ResumeTagType, begin, end))
        return ResumeTag;
      return NoObject;
    }
  
  return typeMask;
}

template <class InputIterator>
bool PiiFlowController::resolvePausedState(int type, InputIterator begin, InputIterator end)
{
  /* If all input queues do have a pause/resume tag, we are able to
     pause/resume even if they are not at the head. But we need to
     move the pause/resume tags to the head and leave other objects
     into the queue.
   */
  for (; begin != end ; ++begin)
    {
      // Find a tag in the queue
      int tagIndex = (*begin)->indexOf(type, 1);
      if (tagIndex == -1)
        return false;
      // Move the tag to the head of the queue
      (*begin)->jump(tagIndex, 1);
    }
  return true;
}

#endif //_PIIFLOWCONTROLLER_H
