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

#include <algorithm>                           // for std::for_each

#include "compile/Compilation.hpp"             // for Compilation
#include "ras/MethodValidationRules"           // for MethodValidationRules
#include "ras/NodeValidationRules"             // for NodeValidationRules

template <typename T, size_t N> static
T* begin(T(&reqArray)[N]) { return &reqArray[0]; }
template <typename T, size_t N> static
T* end(T(&reqArray)[N]) { return &reqArray[0]+N; }

TR::ILValidator::ILValidator(TR::Compilation *comp
//                             , std::vector<TR::MethodValidationRule *> methodValidationRules, 
//                             std::vector<TR::NodeValidationRule *> nodeValidationRules
//                             , std::vector<TR::NodeValidationRule *> blockValidationRules
                             )
   :_comp(comp)
   {
   // TODO: This is heading towards the right direction. But we can
   //       do a lot better. 

/*
   _methodValidationRules.reserve(methodValidationRules.size());
   for (int32_t i = 0; i <= methodValidationRules.size(); ++i)
      {
      _methodValidationRules[i] = methodValidationRules[i];
      }
   _nodeValidationRules.reserve(nodeValidationRules.size());
   for (int32_t i = 0; i <= nodeValidationRules.size(); ++i)
      {
      _methodValidationRules[i] = methodValidationRules[i];
      }
*/
     // CLEAN_UP: For testing purposes, we can just do this for now.
     MethodValidationRule *temp_method_rules = { new SoundnessRule(_comp),
                                                 new ValidateLivenessBoundaries(_comp) };
     NodeValidationRule *temp_node_rules = { new ValidateChildCount(_comp),
                                             new ValidateChildTypes(_comp),
                                             new Validate_ireturnReturnType(_comp) }
     _methodValidationRules.assign(begin(temp_method_rules), end(temp_method_rules));
     _nodeValidationRules.assign(begin(temp_node_rules), end(temp_node_rules));
   }



template <typename T> static
void delete_pointed_object(T* const ptr) 
   {
    delete ptr;
   }


// TODO: Not sure who should take care of the freeing of these Rule Objects.
//       Enable this if we want the ILValidator to do itself.
TR::ILValidator::~ILValidator()
   {
   // CLEAN_UP: Somewhat crude. We can probably do better.
   //           Something like std::unique_ptr should do the trick.
   std::for_each(_methodValidationRules.begin(), _methodValidationRules.end(),
                     delete_pointed_object<TR::MethodValidationRule>);
   std::for_each(node_rules.begin(), node_rules.end(),
                     delete_pointed_object<TR::NodeValidationRule>);
   // TODO: We would probably want to do the same as above for block_rules.
//   std::for_each(validators.begin(), validators.end(),
//                     delete_pointed_object<TR::MethodValidationRule>);
   }

TR::Compilation *TR::ILValidator::comp()
   {
   return _comp;
   }

bool TR::ILValidator::validate()
   {
   // TODO: As things stand, the Rules are guranteed to call "FAIL()" upon
   //       encountering the breach of a specified rule and exit based on the defined protocol.
   //       See: ILValidationUtils.cpp for the definition of FAIL().
   //       We might eventually choose to not Abort and report associated failures.
   //       (If the IL is unsound then it's almost always a good idea to Abort immediately.)

   // Validation is performed across the entire compilation unit.
   for (auto it = _methodValidationRules.begin(); it != _methodValidationRules.end(); ++it) 
       {
       int32_t ret = (*it)->validate(comp()->getMethodSymbol());
       if (ret)
	  return ret;
       }
   // NodeValidationRules only check per node for a specific property.
   for (auto it = _nodeValidationRules.begin(); it != _nodeValidationRules.end(); ++it) 
       {
       for (TR::PreorderNodeIterator nodeIter(comp()->getFirstTreeTop(), comp(), "NODE_VALIDATOR");
            nodeIter.currentTree(); ++nodeIter)
	   {
	   int32_t ret = (*it)->validate(nodeIter.currentNode());
	   if (ret)
	      return ret;
	   }
       }
   // TODO: Add the same functionality for "BlockValidationRules". In the event we
   //       we choose to implement them. 
/*
   // Checks performed across a particular extended block.
   for (auto it = block_rules.begin(); it != block_rules.end(); ++it)
       {
       for (TR::PostorderNodeOccurrenceIterator nodeIter(start, comp(), "BLOCK_VALIDATOR");
            nodeIter.currentTree(); ++nodeIter)
          {
            bool isEndOfExtendedBlock = false;
            TR::TreeTop *nextTree = iter.currentTree()->getNextTreeTop();
            if (nextTree)
               {
               isEndOfExtendedBlock = ! nextTree->getNode()->getBlock()->isExtensionOfPreviousBlock();
               }
            else
               {
               isEndOfExtendedBlock = true;
               }
            
            if (isEndOfExtendedBlock)
               // Ensure there are no nodes live across the end of a block
               (*it)->validate(nodeIter); // Maybe be pass the START and END nodes for the block.
          }

      }
*/


   return true;
   }

