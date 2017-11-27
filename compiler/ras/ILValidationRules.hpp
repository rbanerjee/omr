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
 *          So please instantiate the `*ValidationRule` object
 *          corresponding to the newly added rule in
 *          TR::ILValidator's constructor.
 */

#ifndef ILVALIDATIONRULES_HPP
#define ILVALIDATIONRULES_HPP

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
   public:
   /**
    * @return returns on success.
    *
    * The Rules are guaranteed to call "FAIL()" upon
    * encountering the breach of a specified rule and exit based on
    * the defined protocol.
    * See ILValidationUtils.cpp for the definition of FAIL().
    *
    * TODO: The behaviour of "FAIL()" (as to whether we abort compilation or not) 
    *       is currently based on the specified Rule. 
    *       [For example, we always abort when the IL fails to satisfy the
    *        `SoundnessRule`. However we do not stop compilation if we encounter
    *        the use of deprecated OpCodes i.e `validate_noDeprecatedOpcodes` fails]
    *       Eventually we want to be able to make a Rule Strict or not Based on the
    *       ILValidationStrategy being employed. As in, the same Rule be `Strict` or
    *       `Lenient` based on the chosen Strategy. Note, the strategy chosen by the
    *       ILValidator is based on the state of compilation. I.e whether we are in the
    *       preCodegen phase, just after ILgeneration or inbetween a particular
    *       Optimization etc. So it makes sense to add that functionality.
    *        
    */
   virtual void validate(TR::ResolvedMethodSymbol *methodSymbol) = 0;
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
   public:
   /**
    * @return returns on success.
    *
    * The Rules are guaranteed to call "FAIL()" upon
    * encountering the breach of a specified rule and exit based on
    * the defined protocol.
    * See ILValidationUtils.cpp for the definition of FAIL().
    */
   virtual void validate(TR::TreeTop *firstTreeTop, TR::TreeTop *exitTreeTop) = 0;
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
   public:
   /**
    * Verify the node(TR::Node) of a method has certain properties.
    *
    * @return returns on success.
    *
    * The Rules are guaranteed to call "FAIL()" upon
    * encountering the breach of a specified rule and exit based on
    * the defined protocol.
    * See ILValidationUtils.cpp for the definition of FAIL().
    * @return 0 on success, or a non-zero error code. If non-zero is returned,
    * compilation stops.
    */
   virtual void validate(TR::Node *node) = 0;
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
