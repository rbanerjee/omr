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

#include <algorithm>                               // for std::for_each

#include "ras/ILValidator.hpp"
#include "compile/Compilation.hpp"                 // for Compilation
#include "infra/Assert.hpp"                        // for TR_ASSERT_FATAL
#include "infra/ILWalk.hpp"                        // for TR::PreorderNodeIterator
#include "ras/ILValidationRules.hpp"               // for TR::MethodValidationRules etc.

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
     /**
      * All available `*ValidationRules` are initialized during the creation of
      * ILValidator and they have the same lifetime as the ILValidator itself.
      * It is during the call to `validate` where we decide which subset
      * of the available ones we should use for Validation.
      * This also removes the need to initialize a particular set of `Rule` objects
      * every time a new Strategy is created, or call to `validate` is made.
      */
     TR::MethodValidationRule* temp_method_rules[] = { new  TR::SoundnessRule(_comp),
                                                       new  TR::ValidateLivenessBoundaries(_comp) };

     TR::BlockValidationRule* temp_block_rules[] = { new  TR::ValidateNodeRefCountWithinBlock(_comp) };

     TR::NodeValidationRule* temp_node_rules[] = { new  TR::ValidateChildCount(_comp),
                                                   new  TR::ValidateChildTypes(_comp),
                                                   new  TR::Validate_ireturnReturnType(_comp),
                                                   new  TR::Validate_axaddPlatformSpecificRequirement(_comp) };
     /**
      * NOTE: Please initialize any new *ILValidationRule here!
      *
      * Also, ILValidationRules.hpp and ILValidationStrategies.hpp
      * need to be updated everytime a new ILValidation Rule
      * is added.
      */
     // TODO: add validate_noDe.. 

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


std::vector<TR::MethodValidationRule *>
TR::ILValidator::getRequiredMethodValidationRules(const OMR::ILValidationStrategy *strategy)
   {
   std::vector<TR::MethodValidationRule *> reqMethodValidationRules;
   while (strategy->id != OMR::endRules)
      {
      for (auto it = _methodValidationRules.begin(); it != _methodValidationRules.end(); ++it)
         {
         /**
          *Each *ValidationRule has a unique id. These ids are defined in
          *ILValidationStrategies.hpp and they are assigned in ILValidationRules.cpp. 
          */
         if (strategy->id == (*it)->id())
            reqMethodValidationRules.push_back((*it));
         }
      strategy++;
      }
   return reqMethodValidationRules;
   }

std::vector<TR::BlockValidationRule *>
TR::ILValidator::getRequiredBlockValidationRules(const OMR::ILValidationStrategy *strategy)
   {
   std::vector<TR::BlockValidationRule *> reqBlockValidationRules;
   while (strategy->id != OMR::endRules)
      {
      for (auto it = _blockValidationRules.begin(); it != _blockValidationRules.end(); ++it)
         {
         if (strategy->id == (*it)->id())
            reqBlockValidationRules.push_back((*it));
         }
      strategy++;
      }
   return reqBlockValidationRules;
   }

std::vector<TR::NodeValidationRule *>
TR::ILValidator::getRequiredNodeValidationRules(const OMR::ILValidationStrategy *strategy)
   {
   std::vector<TR::NodeValidationRule *> reqNodeValidationRules;
   while (strategy->id != OMR::endRules)
      {
      for (auto it = _nodeValidationRules.begin(); it != _nodeValidationRules.end(); ++it)
         {
         if (strategy->id == (*it)->id())
            reqNodeValidationRules.push_back((*it));
         }
      strategy++;
      }
   return reqNodeValidationRules;
   }


void TR::ILValidator::validate(const OMR::ILValidationStrategy *strategy)
   {

   // CLEAN_UP: Selection Phase.
   std::vector<TR::MethodValidationRule *> reqMethodValidationRules =
      getRequiredMethodValidationRules(strategy);
   std::vector<TR::BlockValidationRule *> reqBlockValidationRules =
      getRequiredBlockValidationRules(strategy);
   std::vector<TR::NodeValidationRule *> reqNodeValidationRules =
      getRequiredNodeValidationRules(strategy);


   // CLEAN_UP: Validation Phase.
   // Rules that are veriified over the entire method.
   // TODO: find the required RULES. Instead of doing this over all available ones.
   TR::ResolvedMethodSymbol* methodSymbol = comp()->getMethodSymbol();
   for (auto it = _methodValidationRules.begin(); it != _methodValidationRules.end(); ++it)
       {
       (*it)->validate(methodSymbol);
       }

   // Checks performed across an extended blocks.
   for (auto it = _blockValidationRules.begin(); it != _blockValidationRules.end(); ++it)
       {
       TR::TreeTop *tt, *exitTreeTop;
       for (tt = methodSymbol->getFirstTreeTop(); tt; tt = exitTreeTop->getNextTreeTop())
          {
          TR::TreeTop *firstTreeTop = tt;
          exitTreeTop = tt->getExtendedBlockExitTreeTop();
          (*it)->validate(firstTreeTop, exitTreeTop);
          }
       }

   // NodeValidationRules only check per node for a specific property.
   for (auto it = _nodeValidationRules.begin(); it != _nodeValidationRules.end(); ++it)
       {
       for (TR::PreorderNodeIterator nodeIter(methodSymbol->getFirstTreeTop(), comp(), "NODE_VALIDATOR");
            nodeIter.currentTree(); ++nodeIter)
	   {
	   (*it)->validate(nodeIter.currentNode());
	   }  
       }
   }

TR::ILValidator* TR::createILValidatorObject(TR::Compilation *comp)
   {
   return new (comp->trHeapMemory()) TR::ILValidator(comp);
   }

// TODO: Implement the following. This would would be used to "fetch" the required strategy.
/*
const TR::ILValidationStrategy *OMR::ILValidator::ILValidationStrategy(TR::Compilation *comp)
   {
   // Mock Validation Strategies are used for testing, and override 
   // the default compilation strategy.
   // Not sure if I need this right now.
   if (NULL != OMR::ILValidator::_mockValidationStrategy)
      {
      traceMsg(c, "Using mock Validation Strategy %p\n", OMR::ILValidator::_mockValidationStrategy);
      return OMR::ILValidator::_mockValidationStrategy;
      }

   // Figure out which of the 3 states I'm in maybe?
   // Options: 1. Right after ILGen, in between every Optimization, right before Codegen

   // Downgrade strategy rather than crashing in prod.
   if (strategy > lastOMRStrategy)
      strategy = lastOMRStrategy;

   return omrValidationStrategies[strategy]; // Pass some default.
   }
*/
