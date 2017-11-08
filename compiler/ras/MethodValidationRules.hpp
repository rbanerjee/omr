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

// CLEAN_UP: update the file description.
/**
 * @file
 *
 * This file contains classes that specify certain rules
 * that the base Validator class can use to validate the given IL.
 * The helpers defined in ILValidatorHelpers.hpp further help to extend
 * this functionality. For example, AllILValidators can be used to
 * test for a specific subset of the rules provided here, along with any
 * newly defined constraints one would want to test with.
 */

#ifndef ILValidatorCompletenessRules_hpp
#define ILValidatorCompletenessRules_hpp

#include "ras/ILValidationUtils.hpp"

// CLEAN_UP: This should be enough. Probably don't even need this.
namespace TR { class ResolvedMethodSymbol; }

namespace TR {

class MethodValidationRule
   {
   public:
   /**
    * Verify the node of a method has certain properties.
    *
    * @return 0 on success, or a non-zero error code. If non-zero is returned,
    * compilation stops.
    */
   // CLEAN_UP: As things stand, there's really no point in returning an int32_t.
   //           Since the ones I'm defining now all take advantage of ILValidationUtils.
   //           Note that TR::checkCondition is guranteed to FAIL() under a certain protocol
   //           (which is defined in ILValidationUtils.cpp) if the `condition` is not fulfilled.
   //           So as long as this function returns, the node upholds the given rule.
   //           Therefore we could very well make it a boolean or even a void function.
   //           BUT there is a case to be made for future NodeValidationRules that don't use
   //           the utilities defined in ILValidatorUtils.cpp. In which case they might
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
         // CLEAN_UP: This needs to be fixed eventually. I'm not fond of the FAIL macro.
	 FAIL();
	 }
      }
   }
// CLEAN_UP: add the ireturn one here.
class validateLivenessBoundaries : public TR::MethodValidationRule
   {
   TR::Compilation  *_comp;

   public:

   ValidateChildTypes(TR::Compilation *comp)
   : _comp(comp)
   {
   }

   int32_t validate(TR::Node *node)
      {
      auto opcode = node->getOpCode();

      if (opcode.expectedChildCount() != ILChildProp::UnspecifiedChildCount)
         {
         const auto expChildCount = opcode.expectedChildCount();
         const auto actChildCount = node->getNumChildren();

         // Validate child types.
         for (auto i = 0; i < actChildCount; ++i)
            {
            auto childOpcode = node->getChild(i)->getOpCode();
            if (childOpcode.getOpCodeValue() != TR::GlRegDeps)
               {
               const auto expChildType = opcode.expectedChildType(i);
               const auto actChildType = childOpcode.getDataType().getDataType();
               const auto expChildTypeName = (expChildType >= TR::NumTypes) ?
                                              "UnspecifiedChildType" :
                                              TR::DataType::getName(expChildType);
               const auto actChildTypeName = TR::DataType::getName(actChildType);
               TR::checkCondition(node, ((expChildType >= TR::NumTypes) ||
                                         (actChildType == expChildTypei)),
                                  _comp, "Child %d has unexpected type %s (expected %s)",
                                  i, actChildTypeName, expChildTypeName);
               }
            else
               {
               // Make sure the node is allowed to have a GlRegDeps child
               // and check that it is the last child.
               TR::checkCondition(node, opcode.canHaveGlRegDeps() && (i == actChildCount - 1),
                                  _comp, "Unexpected GlRegDeps child %d", i);
               }
            }
         }
      return 0;
      }
   };

} // namespace TR




#endif
