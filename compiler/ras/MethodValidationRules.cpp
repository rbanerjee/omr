/*******************************************************************************
 * Copyright (c) 2017, 2017 IBM Corp. and others
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

#include "ras/MethodValidationRules.hpp"
#include "ras/ILValidationUtils.hpp"                 // for TR::LiveNodeWindow, TR::checkCondition, etc


#include "il/Block.hpp"                              // for TR::Block
#include "il/symbol/ResolvedMethodSymbol.hpp"        // for ResolvedMethodSymbol
#include "infra/Checklist.hpp"                       // for NodeChecklist
#include "infra/ILWalk.hpp"                          // for PostorderNodeOccurrenceIterator

// Exit protocol upon encountering a failure.
#if defined(DEBUG) || defined(PROD_WITH_ASSUMES)
#define ABORT() TR::trap()
#else
#define ABORT() comp->failCompilation<TR::CompilationException>("Validation error: IL is unsound")
#endif
#define FAIL() if (!feGetEnv("TR_continueAfterValidationError")) ABORT()


TR::SoundnessRule::SoundnessRule(TR::Compilation *comp)
   : _comp(comp)
   {
   }

int32_t TR::SoundnessRule::validate(TR::ResolvedMethodSymbol *methodSymbol)
   {
   TR::TreeTop *start = methodSymbol->getFirstTreeTop();
   TR::TreeTop *stop = methodSymbol->getLastTreeTop();
   checkSoundnessCondition(start, start != NULL, "Start tree must exist");
   checkSoundnessCondition(stop, !stop || stop->getNode() != NULL,
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

void TR::SoundnessRule::checkNodeSoundness(TR::TreeTop *location, TR::Node *node,
                                           TR::NodeChecklist &ancestorNodes,
                                           TR::NodeChecklist &visitedNodes)
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


void TR::SoundnessRule::checkSoundnessCondition(TR::TreeTop *location, bool condition,
                                                const char *formatStr, ...)
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


TR::ValidateLivenessBoundaries::ValidateLivenessBoundaries(TR::Compilation *comp)
   : _comp(comp)
   {
   }

int32_t TR::ValidateLivenessBoundaries::validate(TR::ResolvedMethodSymbol *methodSymbol)
   {
   // These must be initialized at the start of every validate call,
   // since the same Rule object can be used multiple times to validate
   // the IL at different stages of the compilation.
   TR::NodeSideTable<NodeState> nodeStates(_comp->trMemory());
   // Similar to NodeChecklist, but more compact. Rather than track
   // node global indexes, which can be sparse, this tracks local
   // indexes, which are relatively dense.  Furthermore, the _basis field
   // allows us not to waste space on nodes we saw in prior blocks.
   // As the name suggests, used to keep track of live Nodes.
   TR::LiveNodeWindow           liveNodes(nodeStates, _comp->trMemory());

   TR::TreeTop *start = methodSymbol->getFirstTreeTop();
   TR::TreeTop *stop = methodSymbol->getLastTreeTop();
   for (TR::PostorderNodeOccurrenceIterator iter(start, _comp, "VALIDATE_LIVENESS_BOUNDARIES");
	iter != stop; ++iter)
      {
      TR::Node *node = iter.currentNode();
      TR::updateNodeState(node, nodeStates, liveNodes, _comp);
      if (node->getOpCodeValue() == TR::BBEnd)
         {
         // Determine whether this is the end of an extended block
         bool isEndOfExtendedBlock = false;
         TR::TreeTop *nextTree = iter.currentTree()->getNextTreeTop();
         if (nextTree)
            {
            // CLEAN_UP: Small nit, but this check should probably be put somewhere
            //           else since it is not directly related to Liveness Boundaries
            //           (just not quite sure where).
            TR::checkCondition(node, nextTree->getNode()->getOpCodeValue() == TR::BBStart,
                               _comp, "Expected BBStart after BBEnd");
            isEndOfExtendedBlock = ! nextTree->getNode()->getBlock()->isExtensionOfPreviousBlock();
            }
         else
            {
            isEndOfExtendedBlock = true;
            }

         if (isEndOfExtendedBlock)
            // Ensure there are no nodes live across the end of a block
            validateEndOfExtendedBlockBoundary(node, liveNodes);
         }
      }
   return 0;
   }

void TR::ValidateLivenessBoundaries::validateEndOfExtendedBlockBoundary(TR::Node *node,
                                                                        LiveNodeWindow &liveNodes)
   {
   for (LiveNodeWindow::Iterator lnwi(liveNodes); lnwi.currentNode(); ++lnwi)
      {
      TR::checkCondition(node, false, _comp,
		     "Node cannot live across block boundary at n%dn",
		     lnwi.currentNode()->getGlobalIndex());
      }

   // At the end of an extended block, no node we've already seen could ever be seen again.
   // Slide the live node window to keep its bitvector compact.
   if (liveNodes.isEmpty())
      {
      liveNodes.startNewWindow();
      }
   }
