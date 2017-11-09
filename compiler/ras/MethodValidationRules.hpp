/*******************************************************************************
 * Copyright (c) 2017 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at http://eclipse.org/legal/epl-2.0
 * or the Apache License, Version 2.0 which accompanies this distribution
 * and is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following Secondary
 * Licenses when the conditions for such availability set forth in the
 * Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
 * version 2 with the GNU Classpath Exception [1] and GNU General Public
 * License, version 2 with the OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
 *******************************************************************************/

/**
 * @file
 *
 * This file contains classes that specify certain Validation rules
 * that the ILValidator class can then use to validate the given IL.
 * The utilities for writing generic ValidationRules are provided
 * in ILValidationUtils.hpp. 
 */

#ifndef METHODVALIDATIONRULES_HPP
#define METHODVALIDATIONRULES_HPP

#include "ras/ILValidationUtils.hpp"


#include "il/symbol/ResolvedMethodSymbol.hpp"        // for ResolvedMethodSymbol

namespace TR {

class MethodValidationRule
   {
   public:
   /**
    * Verify that the IL for method(ResolvedMethodSymbol) has certain properties.
    *
    * @return 0 on success, or a non-zero error code. If non-zero is returned,
    * compilation stops.
    */
   // CLEAN_UP: As things stand, there's really no point in returning an int32_t.
   //           Since the ones defined here all take advantage of ILValidationUtils.
   //           Note that TR::checkCondition is guranteed to FAIL() under a certain protocol
   //           (which is defined in ILValidationUtils.cpp) if the `condition` is not fulfilled.
   //           So as long as this function returns, the node upholds the given rule.
   //           Therefore we could very well make it a boolean or even a void function.
   //           BUT there is a case to be made for future NodeValidationRules that don't use
   //           the utilities defined in ILValidationUtils. In which case they might
   //           choose to use error codes for specific scenerios (personally I'm not a big
   //           fan of doing things that way, specially since there are much better alternatives in
   //           this case).
   virtual int32_t validate(TR::ResolvedMethodSymbol *methodSymbol) = 0; 
   };



class SoundnessRule : public MethodValidationRule
   {
   TR::Compilation  *_comp;

   public:

   ValidateChildCount(TR::Compilation *comp)
   : _comp(comp)
   {
   }

   int32_t validate(TR::ResolvedMethodSymbol *methodSymbol)
      {
      TR::TreeTop *start = methodSymbol->getFirstTreeTop(); 
      TR::TreeTop *stop = methodSymbol->getLastTreeTop();
      soundnessRule(start, start != NULL, "Start tree must exist");
      soundnessRule(stop, !stop || stop->getNode() != NULL,
                    "Stop tree must have a node");

      TR::NodeChecklist treetopNodes(_comp), ancestorNodes(_comp), visitedNodes(_comp);

      // Can't use iterators here, because iterators presuppose that the IL is sound.
      for (TR::TreeTop *currentTree = start; currentTree != stop;
           currentTree = currentTree->getNextTreeTop())
	 {
	 checkSoundnessCondition(currentTree, currentTree->getNode() != NULL,
                       "Tree must have a node");
	 checkSoundnessCondition(currentTree, !treetopNodes.contains(currentTree->getNode()),
                       "Treetop node n%dn encountered twice", currentTree->getNode()->getGlobalIndex());

	 treetopNodes.add(currentTree->getNode());

	 TR::TreeTop *next = currentTree->getNextTreeTop();
	 if (next)
	    {
	    checkSoundnessCondition(currentTree, next->getNode() != NULL, 
                                    "Tree after n%dn must have a node", 
                                    currentTree->getNode()->getGlobalIndex());
	    checkSoundnessCondition(currentTree, next->getPrevTreeTop() == currentTree, 
                                   "Doubly-linked treetop list must be consistent: n%dn->n%dn<-n%dn",
                                    currentTree->getNode()->getGlobalIndex(),
                                    next->getNode()->getGlobalIndex(),
                                    next->getPrevTreeTop()->getNode()->getGlobalIndex());
	    }
	 else
	    {
	    checkSoundnessCondition(currentTree, stop == NULL,
                                    "Reached the end of the trees after n%dn without encountering the stop tree n%dn",
                                    currentTree->getNode()->getGlobalIndex(),
                                    stop? stop->getNode()->getGlobalIndex() : 0);
	    checkNodeSoundness(currentTree, currentTree->getNode(),
                               ancestorNodes, visitedNodes);
	    }
	 }
      return 0;
      }

   void checkNodeSoundness(TR::TreeTop *location, TR::Node *node,
                           NodeChecklist &ancestorNodes, NodeChecklist &visitedNodes)
      {
      TR_ASSERT(node != NULL, "checkNodeSoundness requires that node is not NULL");

      if (visitedNodes.contains(node))
	 return;
      visitedNodes.add(node);

      checkSoundnessCondition(location, !ancestorNodes.contains(node),
                              "n%dn must not be its own ancestor", 
                              node->getGlobalIndex());
      ancestorNodes.add(node);

      for (int32_t i = 0; i < node->getNumChildren(); i++)
	 {
	 TR::Node *child = node->getChild(i);
	 checkSoundnessCondition(location, child != NULL, "n%dn child %d must not be NULL",
                       node->getGlobalIndex(), i);

	 checkNodeSoundness(location, child, ancestorNodes, visitedNodes);
	 }

      ancestorNodes.remove(node);
      }


   void checkSoundnessCondition(TR::TreeTop *location, bool condition, const char *formatStr, ...)
      {
      if (!condition)
	 {
	 if (location && location->getNode())
	    TR::printDiagnostic(_comp, "*** VALIDATION ERROR: IL is unsound at n%dn ***\nMethod: %s\n", location->getNode()->getGlobalIndex(), _comp->signature());
	 else
	    TR::printDiagnostic(_comp, "*** VALIDATION ERROR: IL is unsound ***\nMethod: %s\n", _comp->signature());
	 va_list args;
	 va_start(args, formatStr);
	 TR::vprintDiagnostic(_comp, formatStr, args);
	 va_end(args);
	 TR::printDiagnostic(_comp, "\n");
	 FAIL();
	 }
      }
   }

class ValidateLivenessBoundaries : public TR::MethodValidationRule
   {
   TR::Compilation  *_comp;
   TR::NodeSideTable<NodeState>  _nodeStates;
   // Similar to NodeChecklist, but more compact. Rather than track
   // node global indexes, which can be sparse, this tracks local
   // indexes, which are relatively dense.  Furthermore, the _basis field
   // allows us not to waste space on nodes we saw in prior blocks.
   // As the name suggests, used to keep track of live Nodes.
   TR::LiveNodeWindow            _liveNodes; 

   public:

   ValidateChildTypes(TR::Compilation *comp)
   : _comp(comp),
   _nodeStates(comp->trMemory()),
   _liveNodes(_nodeStates, comp->trMemory())
   {
   }

   int32_t validate(TR::ResolvedMethodSymbol *methodSymbol)
      {
      TR::TreeTop *start = methodSymbol->getFirstTreeTop();
      TR::TreeTop *stop = methodSymbol->getLastTreeTop();

      for (PostorderNodeOccurrenceIterator iter(start, _comp, "VALIDATE_LIVENESS_BOUNDARIES"); 
           iter != stop; ++iter)
         {
         TR::Node *node = iter.currentNode();
         TR::updateNodeState(node, _nodeStates, _liveNodes, _comp);
         if (node->getOpCodeValue() == TR::BBEnd)
	    {
	    // Determine whether this is the end of an extended block
	    //
	    bool isEndOfExtendedBlock = false;
	    TR::TreeTop *nextTree = iter.currentTree()->getNextTreeTop();
	    if (nextTree)
	       {
               // CLEAN_UP: Small nit, but I probably should put this check somewhere
               //           else since it is not directly related to Liveness Boundaries.
	       checkCondition(node, nextTree->getNode()->getOpCodeValue() == TR::BBStart,
                              _comp, "Expected BBStart after BBEnd");
	       isEndOfExtendedBlock = ! nextTree->getNode()->getBlock()->isExtensionOfPreviousBlock();
	       }
	    else
	       {
	       isEndOfExtendedBlock = true;
	       }

	    if (isEndOfExtendedBlock)
               // Ensure there are no nodes live across the end of a block
	       validateEndOfExtendedBlockBoundary(node);
	    }
         }
      return 0;
      }

   void TR::ILValidator::validateEndOfExtendedBlockBoundary(TR::Node *node)
      {
      for (LiveNodeWindow::Iterator lnwi(_liveNodes); lnwi.currentNode(); ++lnwi)
         {
	 checkCondition(node, false, _comp, 
                        "Node cannot live across block boundary at n%dn",
                        lnwi.currentNode()->getGlobalIndex());
         }

      // At the end of an extended block, no node we've already seen could ever be seen again.
      // Slide the live node window to keep its bitvector compact.
      if (_liveNodes.isEmpty())
         {
	 _liveNodes.startNewWindow();
         }
      }

   };

} // namespace TR

#endif
