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

#include "PiiDefaultFlowController.h"
#include "PiiInputSocket.h"
#include "PiiOutputSocket.h"
#include "PiiSocketState.h"
#include <PiiYdinTypes.h>

PiiDefaultFlowController::SyncGroup::SyncGroup(int groupId) :
  _iGroupId(groupId),
  _iFlowLevel(0),
  _iActiveChildren(0),
  _bSyncStartSent(false),
  _bSiblingsInSync(true),
  _pParentGroup(0),
  _bStrictRelationship(false)
{}

PiiDefaultFlowController::SyncGroup::~SyncGroup()
{
  setParentGroup(0);
  for (int i=_lstChildGroups.size(); i--; )
    _lstChildGroups[i]->_pParentGroup = 0;
}

void PiiDefaultFlowController::SyncGroup::sendTag()
{
  // Take the first tag in the group because all should be similar.
  PiiVariant tag = at(0)->objectAt(1);
          
  // Send the tag to all synchronized outputs
  for (int i=lstOutputs.size(); i--; )
    lstOutputs[i]->emitObject(tag);

  int flowLevelChange = tag.valueAs<int>();
  
  releaseInputs();

  _iFlowLevel += flowLevelChange;
}

void PiiDefaultFlowController::SyncGroup::releaseInputs()
{
  for (int i=size(); i--; )
    at(i)->release();
}

// Create a descriptive message of a synchronization error.
void PiiDefaultFlowController::SyncGroup::unexpectedInputError()
{
  PII_THROW(PiiExecutionException,
            PiiDefaultFlowController::tr("Synchronization error: input group %1 got unexpected input (type mask 0x%2).\n")
            .arg(_iGroupId)
            .arg(PiiFlowController::inputGroupTypeMask(begin(), end()), 0, 16) +
            PiiFlowController::dumpInputObjects(begin(), end()));
}

void PiiDefaultFlowController::SyncGroup::flowLevelError()
{
  PII_THROW(PiiExecutionException,
            PiiDefaultFlowController::tr("Synchronization error: the flow level of input group %1 (%2) is not lower than that of group %3 (%4) "
                                         "even though %1 is the parent of %3.")
            .arg(_pParentGroup->_iGroupId).arg(_pParentGroup->_iFlowLevel)
            .arg(_iGroupId).arg(_iFlowLevel));
}

void PiiDefaultFlowController::SyncGroup::activateParents(QVector<SyncEvent>& syncEvents)
{
  if (!_bSyncStartSent)
    {
      // Sync start events are sent from bottom to top (parent group
      // first).
      if (_pParentGroup != 0)
        _pParentGroup->activateParents(syncEvents);
      
      syncEvents << SyncEvent(SyncEvent::StartInput, _iGroupId);
      _bSyncStartSent = true;
    }
}

void PiiDefaultFlowController::SyncGroup::setChildrenInSync(bool inSync)
{
  for (int i=_lstChildGroups.size(); i--; )
    _lstChildGroups[i]->_bSiblingsInSync = inSync;
}

void PiiDefaultFlowController::SyncGroup::resume()
{
  // Sanity check: all flow levels must match.
  PiiSocketState firstState = at(0)->firstObject().valueAs<PiiSocketState>();
  int iFirstDelay = firstState.delay;
  bool bDelayedObjects = iFirstDelay > 0; //false;
  int iFlowLevel = firstState.flowLevel;
  for (int i=1; i<size(); ++i)
    {
      PiiSocketState state = at(i)->firstObject().valueAs<PiiSocketState>();
      if (state.delay > 0) //!= iFirstDelay)
        bDelayedObjects = true;
      if (state.flowLevel != iFlowLevel)
        PII_THROW(PiiExecutionException,
                  PiiDefaultFlowController::tr("Synchronization error: mismatched flow levels in input group %1 while resuming.")
                  .arg(_iGroupId));
    }

  _iFlowLevel = iFlowLevel;

  /*
   * 1) If all delays in a group are the same, the group has been
   *    processed or it is in the initial state.
   * 1.1) If the group has child groups with higher flow levels, it
   *      is still active (processed).
   * 1.2) If all child groups are at the same level, the group is
   *      not active (initial state).
   *
   * 2) If there are different delays, the group has not been
   *    processed yet. Since all objects in the input with the largest
   *    delay must have been processed before receiving a pause tag,
   *    mixed delays means that at least one input is needed before
   *    the group can be processed.
   *
   * It is assumed that child groups are always resumed before the
   * parent.
   */

  _iActiveChildren = 0;

  // No delayed objects
  if (!bDelayedObjects)
    {
      // Count active children
      for (int i=_lstChildGroups.size(); i--; )
        if (_lstChildGroups[i]->_iFlowLevel > _iFlowLevel)
          ++_iActiveChildren;
      
      // If _iActiveChildren is now zero, there is no delay and no
      // active children. This branch of groups is thus completely
      // processed.
    }

  //qDebug("PiiDefaultFlowController::SyncGroup::resume():
  //_iActiveChildren = %d, _iFlowLevel = %d, delayed = %s",
  //_iActiveChildren, _iFlowLevel, bDelayedObjects ? "yes" : "no");

  /* PENDING
   * What if _iActiveChildren != _lstChildGroups.size()?
   * We may need to set the _bSiblingsInSync flag accordingly.
   * Must also check PiiOutputSocket::resume() to ensure correct
   * behavior if a subgroup has not received anything due to a delayed
   * object.
   *
   *       +--------
   *    RP.| parent
   *  RP..>| child 0
   *     DP| child 1
   *
   * In the illustration, _bSiblingsInSync must be true to allow start
   * tag in child 1. R stands for a resume tag and D for a resume tag
   * with delay.
   *
   *       +--------
   *    RP.| parent
   *  RP..>| child 0
   *   P<.>| child 1
   *
   * In this case _bSiblingsInSync must be false to make sure child 1
   * won't start a new round before child 0 is done.
   */

  /* There is no way of knowing if the sync start was already sent. If
     a child group received objects before pause, the event was sent. 
     If neither a child nor the parent received an object, the event
     was not sent. This may happen if the parent group has a non-zero
     delay and a child group pauses before receiving a single object. 
     This could be solved by making PiiOutputSocket to set a flag into
     PiiSocketState that tells if any objects have been sent. But in
     this case a new operation added during pause would never get the
     sync start event. Therefore, we have to risk resending the event
     here and make PiiFlowController::SyncListener filter out
     successive sync events to the same group.
     
     The same problem happens when end tags are received immediately
     after resume. Since we don't know if sync start was sent, we
     cannot filter out an unnecessary sync end event. The SyncListener
     needs to work as a filter for successive events.
  */
  _bSyncStartSent = false;
}

PiiFlowController::FlowState PiiDefaultFlowController::SyncGroup::prepareProcess(QVector<SyncEvent>& syncEvents)
{
  // Can't handle objects if children have not been processed yet. 
#define CHECK_ACTIVE_CHILDREN  if (_iActiveChildren > 0) return IncompleteState

  /*
  int iTypeMask = PiiFlowController::inputGroupTypeMask(begin(), end());
  qDebug("Input group %d type mask: 0x%x. Flow level: %d, Active children: %d",
         _iGroupId,
         iTypeMask,
         _iFlowLevel,
         _iActiveChildren);
  */
  
  switch (PiiFlowController::inputGroupTypeMask(begin(), end()))
    {
    case NoObject: // (Partially) empty group
      return IncompleteState;
      
    case NormalObject: // Normal object in all sockets
      CHECK_ACTIVE_CHILDREN;

      if (_pParentGroup != 0)
        {
          // Ensure sync events have been sent to all parents
          _pParentGroup->activateParents(syncEvents);
          // Flow level must be higher than that of the parent.
          if (_iFlowLevel <= _pParentGroup->_iFlowLevel)
            flowLevelError();
          // If the parent-child relationship is of a strict type, we
          // need to wait for the parent first.
          if (_bStrictRelationship && _pParentGroup->_iActiveChildren == 0)
            return IncompleteState;
        }

      // We are going to process this group
      releaseInputs();

      // Setting this value makes it possible for the child groups to
      // decrease their flow level.
      _iActiveChildren = _lstChildGroups.size();

      // If this group is a parent, a sync event must be sent. The
      // event may already have been sent if any child group received
      // a start tag earlier.
      if (_iActiveChildren > 0 && !_bSyncStartSent)
        {
          syncEvents << SyncEvent(SyncEvent::StartInput, _iGroupId);
          _bSyncStartSent = true;
        }

      // Process this group
      return ProcessableState;

    case StartTag:
      CHECK_ACTIVE_CHILDREN;
      /*qDebug("Sync start in group %d (%s)\n"
             "  Parent: %d\n"
             "  Active siblings: %d\n"
             "  Siblings in sync: %s",
             _iGroupId, qPrintable(at(0)->objectName()),
             _pParentGroup->_iGroupId,
             _pParentGroup->_iActiveChildren,
             _bSiblingsInSync ? "yes" : "no");
      */
      // We can only accept start tags when in sync. The sync
      // condition turns to false when this group drops to parent's
      // flow level and restores when all siblings drop to the same
      // level.
      if (_bSiblingsInSync)
        sendTag();

      return SynchronizedState;

    case EndTag: // Sync end
      CHECK_ACTIVE_CHILDREN;

      /*qDebug("Sync end in group %d (%s)\n"
             "  Parent: %d\n"
             "  Active siblings: %d\n",
             _iGroupId, qPrintable(at(0)->objectName()),
             _pParentGroup ? _pParentGroup->_iGroupId : -1,
             _pParentGroup ? _pParentGroup->_iActiveChildren : -1);
      */
      // If there is no parent, just pass the tag.
      if (_pParentGroup == 0)
        sendTag();
      // We do have a parent, don't we? Are we in sync?
      // If the parent's active child count is zero, it has not
      // received an object yet and we can't pass the tag.
      else if (_pParentGroup->_iActiveChildren > 0)
        {
          sendTag();
          // If we are back at the parent's flow level...
          if (_iFlowLevel == _pParentGroup->_iFlowLevel)
            {
              // Decrease active child count at parent, and check if
              // we were the last one. 
              if (--_pParentGroup->_iActiveChildren == 0)
                {
                  // All siblings are done now -> send sync end event
                  syncEvents << SyncEvent(SyncEvent::EndInput, _pParentGroup->_iGroupId);
                  _pParentGroup->_bSyncStartSent = false;
                  _pParentGroup->setChildrenInSync(true);
                }
              else
                // No start tags will be accepted until all siblings
                // are done.
                _bSiblingsInSync = false;
            }
        }
      else
        return IncompleteState;

      // This allows child groups to accept end tags again.
      _iActiveChildren = _lstChildGroups.size();

      return SynchronizedState;
      
    case StopTag:
      releaseInputs();
      return FinishedState;
    case PauseTag:
      releaseInputs();
      return PausedState;
    case ResumeTag: // Resume tag in all sockets
      releaseInputs();
      return ResumedState;

    default:
      unexpectedInputError();
    }
  
  return IncompleteState;

#undef CHECK_ACTIVE_CHILDREN
}

void PiiDefaultFlowController::SyncGroup::setParentGroup(SyncGroup* parent)
{
  if (_pParentGroup)
    _pParentGroup->_lstChildGroups.remove(_pParentGroup->_lstChildGroups.indexOf(this));

  _pParentGroup = parent;

  if (_pParentGroup)
    _pParentGroup->_lstChildGroups.append(this);
}

PiiDefaultFlowController::Data::Data(const QList<PiiInputSocket*>& inputs,
                                     const QList<PiiOutputSocket*>& outputs,
                                     const RelationList& relations)
{
  initHierarchy(relations);

  // Group sockets
  for (int i=inputs.size(); i--; )
    if (inputs[i]->isConnected())
      addSynchronousInput(inputs[i]);

  // Store connected outputs
  for (int i=outputs.size(); i--; )
    if (outputs[i]->isConnected())
      addSynchronousOutput(outputs[i]);

  // Get rid of empty groups
  for (int i=vecSyncGroups.size(); i--; )
    if (vecSyncGroups[i]->size() == 0)
      {
        delete vecSyncGroups[i];
        vecSyncGroups.remove(i);
      }

  vecActiveSyncGroups = vecSyncGroups;
  vecSyncEvents.reserve(vecSyncGroups.size());
}

PiiDefaultFlowController::Data::~Data()
{
  qDeleteAll(vecSyncGroups);
}

void PiiDefaultFlowController::Data::initHierarchy(const PiiDefaultFlowController::RelationList& relations)
{
  bool bGroupAdded = true;
  RelationList tmpRelations = relations;

  // This loop adds empty groups to vecSyncGroups so that parents are
  // always before children.
  while (bGroupAdded && relations.size() > 0)
    {
      bGroupAdded = false;
      
      // Look for a group without a parent
      for (int i=tmpRelations.size(); i--;)
        {
          int iParent = tmpRelations[i].iParent;
          bool bRoot = true;
          for (int j=tmpRelations.size(); j--;)
            // Found as a child -> not a root group
            if (tmpRelations[j].iChild == iParent)
              {
                bRoot = false;
                break;
              }
          if (bRoot)
            {
              // Go through the relations list and "release" all
              // children of the root group.
              SyncGroup* pParentGroup = findOrCreate(iParent);
              for (int j=tmpRelations.size(); j--;)
                if (tmpRelations[j].iParent == iParent)
                  {
                    SyncGroup* pChildGroup = findOrCreate(tmpRelations[j].iChild);
                    pChildGroup->setParentGroup(pParentGroup);
                    pChildGroup->setStrictRelationship(tmpRelations[j].bStrict);
                    tmpRelations.removeAt(j);
                  }
              
              bGroupAdded = true;
              break;
            }
        }
    }
  
  // If there are relations left, the user set up a loop. She may have
  // added a group with no sockets as well. But we don't care.
}

PiiDefaultFlowController::SyncGroup* PiiDefaultFlowController::Data::findOrCreate(int groupId)
{
  // First see if we already have sockets in this group
  for (int i=vecSyncGroups.size(); i--; )
    if (vecSyncGroups[i]->groupId() == groupId) // yep
      return vecSyncGroups[i];
  
  // No match -> create a new group
  SyncGroup* grp = new SyncGroup(groupId);
  vecSyncGroups << grp;
  return grp;
}

void PiiDefaultFlowController::Data::addSynchronousInput(PiiInputSocket* input)
{
  SyncGroup *grp = findOrCreate(input->groupId());
  grp->append(input);
}

void PiiDefaultFlowController::Data::addSynchronousOutput(PiiOutputSocket* output)
{
  // Add to the correct group
  for (int i=vecSyncGroups.size(); i--; )
    if (vecSyncGroups[i]->groupId() == output->groupId())
      vecSyncGroups[i]->lstOutputs.append(output);
}

PiiDefaultFlowController::PiiDefaultFlowController(const QList<PiiInputSocket*>& inputs,
                                                   const QList<PiiOutputSocket*>& outputs,
                                                   const RelationList& relations) :
  PiiFlowController(new Data(inputs, outputs, relations))
{
}

PiiDefaultFlowController::~PiiDefaultFlowController()
{
}

QString PiiDefaultFlowController::tr(const char* message)
{
  return QCoreApplication::translate("PiiDefaultFlowController", message);
}

PiiDefaultFlowController::Relation PiiDefaultFlowController::strictRelation(int parent, int child)
{
  return Relation(parent, child, true);
}

PiiDefaultFlowController::Relation PiiDefaultFlowController::looseRelation(int parent, int child)
{
  return Relation(parent, child, false);
}

void PiiDefaultFlowController::sendSyncEvents(SyncListener* listener)
{
  PII_D;
  for (int i=0; i<d->vecSyncEvents.size(); ++i)
    listener->sendEvent(&d->vecSyncEvents[i]);
}

PiiFlowController::FlowState PiiDefaultFlowController::prepareProcess()
{
  PII_D;
  d->vecSyncEvents.clear();

  // Check all input groups from last to first. This order ensures
  // that parents are always handled after their children, which
  // usually receive more data.
  for (int i=d->vecActiveSyncGroups.size(); i--; )
    {
      SyncGroup* pGroup = d->vecActiveSyncGroups[i];
      FlowState state = pGroup->prepareProcess(d->vecSyncEvents);

      switch (state)
        {
        case ProcessableState:
          // Process this group.
          d->iActiveInputGroup = pGroup->groupId();
          return ProcessableState;
        case SynchronizedState:
          return SynchronizedState;
        case FinishedState:
        case PausedState:
        case ResumedState:
          // This group is done -> remove from active groups
          d->vecActiveSyncGroups.remove(i);
        case IncompleteState:
          break;
        }
    }

  // All input groups are finished -> it is time to stop/pause/resume
  if (d->vecActiveSyncGroups.size() == 0)
    {
      // All sync groups are active again.
      d->vecActiveSyncGroups = d->vecSyncGroups;
      
      // Take the type of the object in the first input
      int tagType = d->vecSyncGroups[0]->at(0)->typeAt(0);
      if (tagType == PiiYdin::StopTagType)
        return FinishedState;
      else if (tagType == PiiYdin::PauseTagType)
        return PausedState;
      else // if (tagType == PiiYdin::ResumeTagType)
        {
          // Last to first to ensure children are handled first. 
          for (int i=d->vecSyncGroups.size(); i--; )
            d->vecSyncGroups[i]->resume();
          return ResumedState;
        }
    }
  
  return IncompleteState;
}
