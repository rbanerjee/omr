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

#ifndef ILVALIDATIONSTRATEGIES_HPP
#define ILVALIDATIONSTRATEGIES_HPP

#include <stdint.h>                     // for uint16_t

namespace OMR {

enum ILValidationRule
   {
   soundnessRule,
   /**
    *TODO: For Binary operations that are commutative constants must only
    *      appear as the rightmost child.
    */
   validateBinaryOpcodeChildLayout,
   validateChildCount,
   validateChildTypes,
   validateLivenessBoundaries,
   validateNodeRefCountWithinBlock,
   validate_axaddPlatformSpecificRequirement,
   validate_ireturnReturnType,
   /* TODO: Report when deprecated opcodes are encountered. Issue #1971 */
   validate_noDeprecatedOpcodes,
   /**
    * NOTE: Add `id`s for any new ILValidationContext here!
    *       This needs to match the implementation of said *ILValidationRule.
    *       Please see ILValidationRules.cpp for relevant examples.
    */

   /* Used to mark the end of a ILValidationStrategy */
   endRules
   };

struct ILValidationStrategy
   {
   OMR::ILValidationRule        id;
   /**
    * TODO: Eventually ILValidationStrategy should be implemented
    *       to encapsulate information regarding a particular
    *       Validation Rule and the associated option used by the
    *       ILValidator while verifying it.
    *       Such options might include whether the Rule should be treated
    *       as a Strict Rule or not (i.e whether you abort compilation
    *       upon encountering a failure).
    *       For example, a Rule might be treated as a Strict Rule during
    *       preCodeGen validation, where as, it might not be immediately
    *       after ILgen.
    *       Also, a field like `ifEnable` can be used dictate whether
    *       a specific  option was passed during Compilation, and based on
    *       that the Validator can further filter out the rules from a given
    *       Strategy.
    *
    */
//   OMR::ILValidationOptions     options;
   };

extern const ILValidationStrategy emptyStrategy[];

extern const ILValidationStrategy postILgenValidatonStrategy[];

extern const ILValidationStrategy preCodegenValidationStrategy[];

} //namespace OMR

namespace TR {

enum ILValidationContext
   {
   noValidation,
   preCodegenValidation,
   postILgenValidation
   /* NOTE: Please add any new ILValidationContext here! */
   };

extern const OMR::ILValidationStrategy *omrValidationStrategies[];

/**
 *TODO:  Something to look at in the future.
 *       Futher down the road, I think we might even want to
 *       make ILValidator an extentisible class. Where projects
 *       downstream can enforce their own set of Validation Rules,
 *       while still having the ability to choose from the ones provided
 *       by OMR. This conforms with the idea that the IL doesn't
 *       have to be "language-agnostic".
 *       For example it makes sense for Java to only use a Subset of the available OpCodes
 *       provided by our IL. Which in turn, makes it perfectly reasonable for such
 *       a downstream project to employ their own set of restrictions on the said IL.
 */
//      extern const ILValidationStrategy * javaValidationStrategy[];

} // namespace TR

#endif
