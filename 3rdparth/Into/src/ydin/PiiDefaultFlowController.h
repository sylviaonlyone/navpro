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

#ifndef _PIIDEFAULTFLOWCONTROLLER_H
#define _PIIDEFAULTFLOWCONTROLLER_H

#include "PiiFlowController.h"
#include <QVector>

/**
 * The default flow controller handles inter-operation synchronization
 * by grouping synchronous sockets and injecting special
 * synchronization tags into the sockets. It handles hierarchical
 * relations between synchronized socket groups. Due to the extra
 * checking needed for this, @p %PiiDefaultFlowController is used by
 * PiiDefaultOperation only if there is no more efficient
 * specialization available.
 *
 * @see PiiOneInputFlowController
 * @see PiiOneGroupFlowController
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiDefaultFlowController : public PiiFlowController
{
public:
  /**
   * A structure for storing information about parent-child
   * relationships.
   */
  struct Relation
  {
    /// The group id of the parent group.
    int iParent;
    /// The group id of the child group.
    int iChild;
    /// Strict relationship flag.
    bool bStrict;

    Relation() : iParent(0), iChild(0), bStrict(false) {}
      
    /// Create a new relationship.
    Relation(int parent, int child, bool strict) :
      iParent(parent), iChild(child), bStrict(strict)
    {}
  };
  
  typedef QList<Relation> RelationList;

  /**
   * Create a new flow controller.
   *
   * @param inputs a list of inputs to be controlled
   *
   * @param outputs the a list of outputs to be controlled
   *
   * @param relations a list of parent-child relations. Each entry in
   * the list assigns a parent-child relationship between the given
   * two group ids.
   *
   * The @p relations parameter is used to assign parent-child
   * relations between input groups. @p %PiiDefaultFlowController can
   * handle many independent groups of synchronized sockets. This
   * parameter makes it possible to place restrictions to inter-group
   * synchronization.
   *
   * There are two types of parent-child relationships: @e strict and
   * @e loose. In a strict relationship, a child group can only
   * receive data once its parent has received data. This is sometimes
   * necessary, but requires special care in building the
   * configuration to prevent deadlocks. For example,
   * PiiObjectReplicator needs to receive the object to be replicated
   * before it can replicate it. In a loose relationship, either the
   * parent or the child may become first. In both cases, a sync event
   * will be sent just before an object in a parent group or any of
   * its children is to be processed. The parent group will be
   * suspended until all data related to the object(s) in the parent
   * group has been processed by the child groups. Once this happens,
   * another sync event will be sent with the parent's group id.
   *
   * @code
   * MyOperation::MyOperation()
   * {
   *   // Receives large images
   *   PiiInputSocket* imageInput = new PiiInputSocket("image");
   *   // Receives smaller pieces of the large image, for example
   *   // via a PiiImageSplitter
   *   PiiInputSocket* pieceInput = new PiiInputSocket("subimage");
   *   // Receives the locations of the image pieces.
   *   PiiInputSocket* locationInput = new PiiInputSocket("location");
   *
   *   addSocket(imageInput);
   *   addSocket(pieceInput);
   *   addSocket(locationInput);
   *
   *   // Group the inputs. (imageInput remains is the default group 0)
   *   pieceInput->setGroupId(1);
   *   locationInput->setGroupId(1);
   *   // The default implementation of createFlowController() would
   *   // assign a loose relationship between groups 0 and 1. We want
   *   // a strict relationship.
   * }
   *
   * PiiFlowController* MyOperation::createFlowController()
   * {
   *   // Establish a strict parent-child relationship.
   *   PiiDefaultFlowController::RelationList lstRelations;
   *   lstRelations << PiiDefaultFlowController::strictRelation(0, 1);
   *   return new PiiDefaultFlowController(_lstInputs,
   *                                       _lstOutputs,
   *                                       lstRelations);
   * }
   * @endcode
   */
  PiiDefaultFlowController(const QList<PiiInputSocket*>& inputs,
                           const QList<PiiOutputSocket*>& outputs,
                           const RelationList& relations = RelationList());

  ~PiiDefaultFlowController();

  /**
   * A utility function that creates a strict relationship between two
   * groups, denoted by @p parent and @p child.
   */
  static Relation strictRelation(int parent, int child);

  /**
   * A utility function that creates a loose relationship between two
   * groups, denoted by @p parent and @p child.
   */
  static Relation looseRelation(int parent, int child);
  
  FlowState prepareProcess();
  void sendSyncEvents(SyncListener* listener);

private:
  /**
   * Stores information related to a group of sockets working in sync.
   */
  class SyncGroup : public QVector<PiiInputSocket*>
  {
  public:
    SyncGroup(int groupId = 0);
    ~SyncGroup();

    void setGroupId(int groupId) { _iGroupId = groupId; }
    int groupId() const { return _iGroupId; }

    /**
     * Add this group as a child to @p parent.
     */
    void setParentGroup(SyncGroup* parent);
    /**
     * Get the parent group. If there is no parent, 0 will be
     * returned.
     */
    SyncGroup* parentGroup() const { return _pParentGroup; }

    void setStrictRelationship(bool strict) { _bStrictRelationship = strict; }
    bool isStrictRelationship() const { return _bStrictRelationship; }
    
    /**
     * Get the current level of the flow through this group. The default
     * level is zero.
     */
    int flowLevel() const { return _iFlowLevel; }

    /**
     * Set the current flow level.
     */
    void setFlowLevel(int level) { _iFlowLevel = level; }
  
    /**
     * A convenience function to change the flow level. This is
     * equivalent to setFlowLevel(flowLevel()+change).
     */
    void changeFlowLevel(int change) { _iFlowLevel += change; }

    inline void releaseInputs();
    void sendTag();
    void resume();

    void setSyncStartSent(bool sent) { _bSyncStartSent = sent; }
    bool isSyncStartSent() const { return _bSyncStartSent; }

    /**
     * Prepares this group of sockets for processing.
     */
    inline FlowState prepareProcess(QVector<SyncEvent>& syncEvents);
    /**
     * Outputs synchronized to this group.
     */
    QVector<PiiOutputSocket*> lstOutputs;

  private:
    void unexpectedInputError();
    void flowLevelError();
    inline void activateParents(QVector<SyncEvent>& syncEvents);
    void setChildrenInSync(bool inSync);
    
    int _iGroupId;
    int _iFlowLevel;
    int _iActiveChildren;
    bool _bSyncStartSent;
    bool _bSiblingsInSync; // true when all sibling groups are at parent's flow level
    SyncGroup* _pParentGroup;
    bool _bStrictRelationship;
    QVector<SyncGroup*> _lstChildGroups;
    bool _bProcessable;
  };

  friend class SyncGroup;

  class Data : public PiiFlowController::Data
  {
  public:
    Data();
    Data(const QList<PiiInputSocket*>& inputs,
         const QList<PiiOutputSocket*>& outputs,
         const RelationList& relations);
    ~Data();
    
    /**
     * Groups of synchronized input and output sockets.
     */
    QVector<SyncGroup*> vecSyncGroups;
    /**
     * Active (non-finished) groups.
     */
    QVector<SyncGroup*> vecActiveSyncGroups;
    QVector<SyncEvent> vecSyncEvents;

    bool bStateChanged;

  private:
    void addSynchronousInput(PiiInputSocket* input);
    void addSynchronousOutput(PiiOutputSocket* output);
    SyncGroup* findOrCreate(int groupId);
    void initHierarchy(const RelationList& relations);
  };

  PII_D_FUNC;
  
  static QString tr(const char* message);
};

#endif //_PIIDEFAULTFLOWCONTROLLER_H
