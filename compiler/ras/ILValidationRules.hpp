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
 *
 * NOTE: 1. Please add any new `*ValidationRule`s here!
 *
 *       2. ILValidationStrategies.hpp must also be updated
 *          for a newly added `Rule` to become part of a particular
 *          Validation Strategy.
 *     
 *       3. Finally, the ILValidator is responsible for validating
 *          the IL based on a certain ILValidationStrategy.
 *          So please instantiate the said `*ValidationRule` object
 *          during the creation of TR::ILValidator.
 *
 */

#ifndef ILVALIDATIONRULES_HPP
#define ILVALIDATIONRULES_HPP

#include "ras/ILValidationStrategies.hpp"      // for OMR::ILValidationRule

#include "infra/BitVector.hpp"                 // for TR_BitVector
#include "infra/SideTable.hpp"                 // for NodeSideTable


namespace TR { class Compilation; }
namespace TR { class LiveNodeWindow; }
namespace TR { class Node; }
namespace TR { class NodeChecklist; }
namespace TR { class ResolvedMethodSymbol; }
namespace TR { class TreeTop; }

namespace TR {

/**
 * MethodValidationRule:
 *
 * Verify that the IL of a method (ResolvedMethodSymbol) has certain properties.
 *
 */
class MethodValidationRule
   {
   OMR::ILValidationRule _id;
   public:
   MethodValidationRule(OMR::ILValidationRule id)
   : _id(id)
   {
   }
   /**
    * @return returns on success.
    *
    * Otherwise, reports the relevant errors.
    * After which, safely brings down the VM if the `continueAfterILValidationError`
    * Compiler Option is not set.
    *
    * TODO: The decision as to whether we abort compilation or not,
    *       is currently based solely on the said Compiler Option.
    *       However, it makes sense to be a able to make this choice
    *       based on the Rule we are validating against.
    *       For example, we should always abort when the IL fails to satisfy the
    *       `SoundnessRule`. However we should not be required to stop compilation
    *       upon encountering a deprecated OpCode.
    *       Furthermore, this notion of "strictness" should be employable on a
    *       `ILValidationStrategy` level. As in, the same Rule can be `Strict` or
    *       `Lenient` based on the chosen Strategy.
    *       Note, the strategy chosen by the ILValidator is currently
    *       based on the state of Compilation.
    *        
    */
   virtual void validate(TR::ResolvedMethodSymbol *methodSymbol) = 0;
   // CLEAN_UP: explain why.

   OMR::ILValidationRule id()   { return _id; }
   };


class SoundnessRule : public MethodValidationRule
   {
   TR::Compilation  *_comp;

   public:
   SoundnessRule(TR::Compilation *comp);
   void validate(TR::ResolvedMethodSymbol *methodSymbol);

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
   void validate(TR::ResolvedMethodSymbol *methodSymbol);

   private:
   void validateEndOfExtendedBlockBoundary(TR::Node *node,
                                           LiveNodeWindow &liveNodes);
   };

/* NOTE: Please add any new MethodValidationRules here */



/**
 * BlockValidationRule: 
 * 
 * Verify that the IL for a particular extended block has certain properties.
 */

class BlockValidationRule
   {
   OMR::ILValidationRule _id;
   public:
   BlockValidationRule(OMR::ILValidationRule id)
   : _id(id)
   {
   }
   /**
    * @return returns on success.
    *
    * Otherwise, reports the relevant errors.
    * After which, safely brings down the VM if the `continueAfterILValidationError`
    * Compiler Option is not set.
    */
   virtual void validate(TR::TreeTop *firstTreeTop, TR::TreeTop *exitTreeTop) = 0;

   OMR::ILValidationRule id()   { return _id; }
   };


class ValidateNodeRefCountWithinBlock : public TR::BlockValidationRule
   {
   TR::Compilation  *_comp;
   TR_BitVector  _nodeChecklist;

   public:
   ValidateNodeRefCountWithinBlock(TR::Compilation *comp);
   void validate(TR::TreeTop *firstTreeTop, TR::TreeTop *exitTreeTop);

   private:
   void validateRefCountPass1(TR::Node *node);
   void validateRefCountPass2(TR::Node *node);
   };

/* NOTE: Please add any new BlockValidationRules here */



/**
 * NodeValidationRule: 
 * 
 * Verify that the IL for a particular TR::Node has certain properties.
 */
class NodeValidationRule
   {
   OMR::ILValidationRule _id;
   public:
   NodeValidationRule(OMR::ILValidationRule id)
   : _id(id)
   {
   }
   /**
    * @return returns on success.
    *
    * Otherwise, reports the relevant errors.
    * After which, safely brings down the VM if the `continueAfterILValidationError`
    * Compiler Option is not set.
    */
   virtual void validate(TR::Node *node) = 0;

   OMR::ILValidationRule id()   { return _id; }
   };



class ValidateChildCount : public NodeValidationRule
   {
   TR::Compilation  *_comp;

   public:

   ValidateChildCount(TR::Compilation *comp);

   void validate(TR::Node *node);
   };


class ValidateChildTypes : public TR::NodeValidationRule
   {
   TR::Compilation  *_comp;

   public:

   ValidateChildTypes(TR::Compilation *comp);

   void validate(TR::Node *node);
   };

/**
 * TODO: As things stand, the expected child type for `ireturn` is
 *       one of Int{8,16,32}.
 *       Though we have yet to resolve the issue regarding:
 *       "What should be the canonical way to return integers smaller than Int32?"
 *       See Issue #1901 for more details.
 */
class Validate_ireturnReturnType : public TR::NodeValidationRule
   {
   TR::Compilation  *_comp;

   public:

   Validate_ireturnReturnType(TR::Compilation *comp);

   void validate(TR::Node *node);
   };

/**
 * The OpCodes aiadd and aiuadd are only valid on 32 bit platforms.
 * See: Issue #556
 */
class Validate_axaddPlatformSpecificRequirement : public TR::NodeValidationRule
   {
   TR::Compilation  *_comp;

   public:

   Validate_axaddPlatformSpecificRequirement(TR::Compilation *comp);

   void validate(TR::Node *node);
   };

/* NOTE: Please add any new NodeValidationRules here */

} //namespace TR


#endif
