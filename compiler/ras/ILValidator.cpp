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

// CLEAN_UP: See if you need all of these. Almost certain that
//           I don't need most of these.
#include "il/DataTypes.hpp"
#include "il/Node.hpp"
#include "il/Node_inlines.hpp"
#include "il/ILProps.hpp"
#include "il/ILOps.hpp"
#include "il/Block.hpp"
#include "il/Block_inlines.hpp"
#include "infra/Assert.hpp"


TR::ILValidator::ILValidator(TR::Compilation *comp)
   :_comp(comp)
   {
   }

TR::ILValidator::validate()
   {
   // CLEAN_UP: Add seperate TRACE messages for each of them.

   // The Rules are guranteed to call "FAIL()" upon encountering
   // the breach of a specificied rule and exit based on the defined protocol
   // See: ILValidator.hpp for the definition of FAIL().

   // Validation is performed across the entire compilation unit.
   for (auto it = method_rules.begin(); it != method_rules.end(); ++it) 
       {
       // CLEAN_UP: Kind of subtle but Iterators can only be used if the IL is
       //           Sound. And Soundness Check is therefore our first method_rule.
       int32_t ret = (*it)->validate(comp, this);
       if (ret)
	  return ret;
       }
   // CLEAN_UP: Node Validation rules only check per node for a specific property.
   for (auto it = node_rules.begin(); it != node_rules.end(); ++it) 
       {
       // CLEAN_UP: Traverse the IL and call verify on each of the nodes. 
       //           No need to use the custom iterator class.
       for (TR::PreorderNodeIterator nodeIter(comp->getFirstTreeTop(), comp, "NODE_VALIDATOR");
            nodeIter.currentTree(); ++nodeIter)
	   {
	   // CLEAN_UP: TRACE Message which Node upon which rule failed.
	   int32_t ret = (*it)->validate(nodeIter.currentNode(), this);
	   if (ret)
	      return ret;
	   }
       }
   // Checks performed across a particular block.
   for (auto it = block_rules.begin(); it != block_rules.end(); ++it)
       {
       for (TR::PostorderNodeOccurrenceIterator nodeIter(start, _comp, "BLOCK_VALIDATOR");
            nodeIter.currentTree(); ++nodeIter)
	   {
	   if (iter->getOpCodeValue() == TR::BBEnd)
	      // CLEAN_UP: This might need the use of fancy iterators.
	      //           End of block validation can still be done here I think. Not sure.
	      int32_t ret = (*it)->validate(nodeIter.currentNode(), this);
	      if (ret)
		 return ret;
	   }
       }
   return true;
   }


// CLEAN_UP: This needs to be part of the NodeValidationRule that checks IL Soundess.
/*
void TR::ILValidator::soundnessRule(TR::TreeTop *location, bool condition, const char *formatStr, ...)
   {
   if (!condition)
      {
      if (location && location->getNode())
         printDiagnostic("*** VALIDATION ERROR: IL is unsound at n%dn ***\nMethod: %s\n", location->getNode()->getGlobalIndex(), comp()->signature());
      else
         printDiagnostic("*** VALIDATION ERROR: IL is unsound ***\nMethod: %s\n", comp()->signature());
      va_list args;
      va_start(args, formatStr);
      vprintDiagnostic(formatStr, args);
      va_end(args);
      printDiagnostic("\n");
      FAIL();
      }
   }

*/
