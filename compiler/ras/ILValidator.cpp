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

#include "ras/ILValidator.hpp"

#include <algorithm>                               // for std::for_each

#include "compile/Compilation.hpp"                 // for Compilation
#include "infra/ILWalk.hpp"                        // for PostorderNodeOccurrenceIterator
#include "ras/BlockValidationRules.hpp"            // for BlockValidationRules
#include "ras/MethodValidationRules.hpp"           // for MethodValidationRules
#include "ras/NodeValidationRules.hpp"             // for NodeValidationRules

template <typename T, size_t N> static
T* begin(T(&reqArray)[N])
  {
  return &reqArray[0];
  }
template <typename T, size_t N> static
T* end(T(&reqArray)[N])
   {
   return &reqArray[0]+N;
   }

TR::ILValidator::ILValidator(TR::Compilation *comp)
   :_comp(comp)
   {
     // Initialize all the available rules in the constructor.
     // It is during the `validate` call where we decide which subset
     // of the available ones we should call.
     // This also lets one specify the ordering in which the rules are validated.
     TR::MethodValidationRule* temp_method_rules[] = { new  TR::SoundnessRule(_comp),
                                                       new  TR::ValidateLivenessBoundaries(_comp) };

     TR::BlockValidationRule* temp_block_rules[] = { new  TR::ValidateNodeRefCountWithinBlock(_comp) };

     TR::NodeValidationRule* temp_node_rules[] = { new  TR::ValidateChildCount(_comp),
                                                   new  TR::ValidateChildTypes(_comp),
                                                   new  TR::Validate_ireturnReturnType(_comp),
                                                   new  TR::Validate_axaddPlatformSpecificRequirement(_comp) };

     _methodValidationRules.assign(begin(temp_method_rules), end(temp_method_rules));
     _blockValidationRules.assign(begin(temp_block_rules), end(temp_block_rules));
     _nodeValidationRules.assign(begin(temp_node_rules), end(temp_node_rules));
   }


template <typename T> static
void delete_pointed_object(T* const ptr)
   {
    delete ptr;
   }

TR::ILValidator::~ILValidator()
   {
   // CLEAN_UP: Somewhat crude. We can probably do better.
   //           Another solution would be to initialize the *Rule objects
   //           using trHeapMemeory.
   std::for_each(_methodValidationRules.begin(), _methodValidationRules.end(),
                 delete_pointed_object<TR::MethodValidationRule>);
   std::for_each(_blockValidationRules.begin(), _blockValidationRules.end(),
                 delete_pointed_object<TR::BlockValidationRule>);
   std::for_each(_nodeValidationRules.begin(), _nodeValidationRules.end(),
                 delete_pointed_object<TR::NodeValidationRule>);
   }



TR::Compilation *TR::ILValidator::comp()
   {
   return _comp;
   }

bool TR::ILValidator::validate()
   {
   // TODO: As things stand, the Rules are guranteed to call "FAIL()" upon
   //       encountering the breach of a specified rule and exit based on
   //       the defined protocol.
   //       See: ILValidationUtils.cpp for the definition of FAIL().
   //       We might eventually choose to not Abort but still report associated failures.
   //       (Note: If the IL fails on the SoundnessRule then it's almost
   //       always a good idea to Abort immediately.)

   // Rules that are veriified over the entire method.
   TR::ResolvedMethodSymbol* methodSymbol = comp()->getMethodSymbol();
   for (auto it = _methodValidationRules.begin(); it != _methodValidationRules.end(); ++it)
       {
       int32_t ret = (*it)->validate(methodSymbol);
       if (ret)
	  // CLEAN_UP: The return code here is rather redundant since if the IL
          //           fails Validation then this part of the code never gets reached.
          //           It might be worth investigating whether that's the best course of action
          //           and if not, then handle the error here instead.
          /*maybe do something with ret*/
          return false;
       }

   // Checks performed across an extended blocks.
   for (auto it = _blockValidationRules.begin(); it != _blockValidationRules.end(); ++it)
       {
       TR::TreeTop *tt, *exitTreeTop;
       for (tt = methodSymbol->getFirstTreeTop(); tt; tt = exitTreeTop->getNextTreeTop())
          {
          TR::TreeTop *firstTreeTop = tt;
          exitTreeTop = tt->getExtendedBlockExitTreeTop();
          int32_t ret = (*it)->validate(firstTreeTop, exitTreeTop);
          // CLEAN_UP: See the comment above.
          if (ret)
             return false;
          }

      }

   // NodeValidationRules only check per node for a specific property.
   for (auto it = _nodeValidationRules.begin(); it != _nodeValidationRules.end(); ++it)
       {
       for (TR::PreorderNodeIterator nodeIter(methodSymbol->getFirstTreeTop(), comp(), "NODE_VALIDATOR");
            nodeIter.currentTree(); ++nodeIter)
	   {
	   int32_t ret = (*it)->validate(nodeIter.currentNode());
           // CLEAN_UP: See the comment above.
	   if (ret)
	      return false;
	   }
       }

   return true;
   }

TR::ILValidator* TR::createILValidatorObject(TR::Compilation *comp)
   {
   TR::ILValidator *ilValidator = NULL;
   ilValidator = new (comp->trHeapMemory()) TR::ILValidator(comp);
   return ilValidator;
   }

// WIP:
/*
const TR::ILValidationStrategy *OMR::ILValidator::ILValidationStrategy(TR::Compilation *comp)
   {
   // Mock Validation Strategies are used for testing, and override 
   // the default compilation strategy.
   // Not sure if I need this right now.
   if (NULL != OMR::Optimizer::_mockValidationStrategy)
      {
      traceMsg(c, "Using mock Validation Strategy %p\n", OMR::ILValidator::_mockValidationStrategy);
      return OMR::ILValidator::_mockValidationStrategy;
      }

   TR_Hotness strategy = c->getMethodHotness();
   TR_ASSERT(strategy <= lastOMRStrategy, "Invalid optimization strategy");

   // Downgrade strategy rather than crashing in prod.
   if (strategy > lastOMRStrategy)
      strategy = lastOMRStrategy;

   return omrCompilationStrategies[strategy]; // Pass some default.
   }
*/
