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

namespace TR {

class NodeValidationRule
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
   virtual int32_t validate(TR::Node *node) = 0; 
   };



class ValidateChildCount : public NodeValidationRule
   {
   TR::Compilation  *_comp;

   public:

   ValidateChildCount(TR::Compilation *comp)
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

         // Validate child count.
         if (!opcode.canHaveGlRegDeps())
            {
            // In the common case, no GlRegDeps child is expect nor present.
            TR::checkCondition(node, actChildCount == expChildCount, _comp,
                               "Child count %d does not match expected value of %d",
                               actChildCount, expChildCount);
            }
         else if (actChildCount == (expChildCount + 1))
            {
            // Adjust expected child number to account for a possible extra GlRegDeps
            // child and make sure the last child is actually a GlRegDeps.
            TR::checkCondition(node,
                               node->getChild(actChildCount - 1)->getOpCodeValue() == TR::GlRegDeps, _comp,
                               "Child count %d does not match expected value of %d (%d without GlRegDeps) and last child is not a GlRegDeps",
                               actChildCount, expChildCount + 1, expChildCount);
            } 
         else
            {
            // If expected and actual child counts don't match, then the child
            // count is just wrong, even with an expected GlRegDeps.
            TR::checkCondition(node, actChildCount == expChildCount, _comp,
                               "Child count %d matches neither expected values of %d (without GlRegDeps) nor %d (with GlRegDeps)",
                               actChildCount, expChildCount, expChildCount + 1);
            }
         }
      return 0; 
      }
   };


class ValidateChildTypes : public TR::NodeValidationRule
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


class Validate_ireturnReturnType : public TR::NodeValidationRule
   {
   TR::Compilation  *_comp;

   public:

   Validate_ireturnReturnType(TR::Compilation *comp)
   : _comp(comp)
   {
   }

   int32_t validate(TR::Node *node)
      {
      auto opcode = node->getOpCode();
      if (opcode.getOpCodeValue() == TR::ireturn)
         {
	  if (opcode.expectedChildCount() != ILChildProp::UnspecifiedChildCount)
	     {
	     for (auto i = 0; i < actChildCount; ++i)
		{
		auto childOpcode = node->getChild(i)->getOpCode();
		const auto actChildType = childOpcode.getDataType().getDataType();
		checkCondition(node, (actChildType == TR::Int32 ||
                                      actChildType == TR::Int16 ||
                                      actChildType == TR::Int8), _comp,
                               "ireturn has an invalid child type %s (expected Int{8,16,32})",
                               actChildTypeName);
		}
             }
         }
      return 0;
      }
   };



// TODO: Add a rules regarding platform specific opcodes.
//       (See: Issue #566 regarding the one about `aiadd`)

} // namespace TR




#endif
