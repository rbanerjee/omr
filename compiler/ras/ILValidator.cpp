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

TR::ILValidator::ILValidator(TR::Compilation *comp)
   :_comp(comp)
   {
   // Initialize the List of rules here. Via creating a new clever constructor.
   }



template <typename T> static
void delete_pointed_object(T* const ptr) {
    delete ptr;
}


TR::ILValidator::~ILValidator()
   :_comp(comp)
   {
   // CLEAN_UP: Somewhat crude. We can probably do better.
   std::for_each(method_rules.begin(), method_rules.end(),
                     delete_pointed_object<TR::MethodValidationRule>);
   std::for_each(node_rules.begin(), node_rules.end(),
                     delete_pointed_object<TR::NodeValidationRule>);
   // TODO: We would probably want to do the same as above for block_rules.
/*
   std::for_each(validators.begin(), validators.end(),
                     delete_pointed_object<TR::MethodValidationRule>);
*/
   }

TR::Compilation *TR::ILValidator::comp()
   {
   return _comp;
   }

bool TR::ILValidator::validate()
   {
   // TODO: As things are, the Rules are guranteed to call "FAIL()" upon 
   //       encountering the breach of a specified rule and exit based on the defined protocol.
   //       See: ILValidationUtils.cpp for the definition of FAIL().
   //       We might eventually choose to not Abort and report associated failures.
   //       (If the IL is unsound then it's almost always a good idea to Abort immediately.)

   // Validation is performed across the entire compilation unit.
   for (auto it = method_rules.begin(); it != method_rules.end(); ++it) 
       {
       int32_t ret = (*it)->validate(comp()->getMethodSymbol());
       if (ret)
	  return ret;
       }
   // NodeValidationRules only check per node for a specific property.
   for (auto it = node_rules.begin(); it != node_rules.end(); ++it) 
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

