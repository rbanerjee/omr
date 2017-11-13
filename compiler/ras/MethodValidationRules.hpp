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

#include <stdint.h>                   // for int32_t

namespace TR { class Compilation; }
namespace TR { class LiveNodeWindow; }
namespace TR { class Node; }
namespace TR { class NodeChecklist; }
namespace TR { class ResolvedMethodSymbol; }
namespace TR { class TreeTop; }

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
   //           BUT there is a case to be made for future MethodValidationRules that don't use
   //           the utilities defined in ILValidationUtils. In which case they might
   //           choose to use error codes for specific scenerios (personally I'm not a big
   //           fan of doing things that way, specially since there are much better alternatives in
   //           this case) [same applies for Node and Block Validation Rules].
   virtual int32_t validate(TR::ResolvedMethodSymbol *methodSymbol) = 0;
   };


// TODO: Even though the Rule(class) names are somewhat "self explanatory",
//       we might still want to formally define each of these rules.
//       And if we want these definitions to live with the code, then
//       this seems like a good place to do that.
//       (Another option would be to create a doc on github for these, but I can
//        justify providing a brief description here as well.)
class SoundnessRule : public MethodValidationRule
   {
   TR::Compilation  *_comp;

   public:
   SoundnessRule(TR::Compilation *comp);
   int32_t validate(TR::ResolvedMethodSymbol *methodSymbol);

   private:
   void checkNodeSoundness(TR::TreeTop *location, TR::Node *node,
                           TR::NodeChecklist &ancestorNodes,
                           TR::NodeChecklist &visitedNodes);

   void checkSoundnessCondition(TR::TreeTop *location, bool condition,
                                const char *formatStr, ...);
   };

class ValidateLivenessBoundaries : public TR::MethodValidationRule
   {
   TR::Compilation  *_comp;

   public:
   ValidateLivenessBoundaries(TR::Compilation *comp);
   int32_t validate(TR::ResolvedMethodSymbol *methodSymbol);

   private:
   void validateEndOfExtendedBlockBoundary(TR::Node *node,
                                           LiveNodeWindow &liveNodes);
   };

} // namespace TR

#endif
