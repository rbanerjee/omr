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

#ifndef ILVALIDATIONSTRATEGIES_INCL
#define ILVALIDATIONSTRATEGIES_INCL

#include <stdint.h>                     // for uint16_t

namespace OMR {

enum ILValidationRule
   {
   soundnessRule,
   // TODO: Implement this.
   validateBinaryOpcodeChildLayout,
   validateChildCount,
   validateChildTypes,
   validateLivenessBoundaries,
   validateNodeRefCountWithinBlock,
   validate_axaddPlatformSpecificRequirement,
   validate_ireturnReturnType,
   // TODO: Implement this.
   validate_noDeprecatedOpcodes,
   // Used to mark the end of a ILValidationStrategy.
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

/* Do not perform any Validation under this Strategy. */
const ILValidationStrategy emptyStrategy[] =
   {
   {endRules}
   };

/* Strategy used to Validate right after ILGeneration. */
const ILValidationStrategy postILgenValidatonStrategy[] =
   {
   {soundnessRule               /*TODO: specify default options for this Rule under the said strategy*/},
   // CLEAN_UP: THIS should fail and hence the need to have a more state aware
   //           form of validation.
   //           Keep for now for testing purposes.
   // This should fail right after ILGeneration.
   // Since currently TreeSimplifier takes care of this.
   {validateBinaryOpcodeChildLayout},
   {validateChildCount},
   {validateChildTypes},
   {validateLivenessBoundaries},
   {validateNodeRefCountWithinBlock},
   {validate_noDeprecatedOpcodes},
   {endRules}
   };

/**
 * Strategy used to Validate right before Codegen.
 * At this point the IL is expected to uphold almost all the Validation Rules.
 */
const ILValidationStrategy preCodegenValidationStrategy[] =
   {
   {soundnessRule},
   {validateBinaryOpcodeChildLayout},
   {validateChildCount},
   {validateChildTypes},
   {validateLivenessBoundaries},
   {validateNodeRefCountWithinBlock},
   {validate_axaddPlatformSpecificRequirement},
   {validate_ireturnReturnType},
   {validate_noDeprecatedOpcodes},
   {endRules}
   };



} //namespace OMR

namespace TR {

enum ILValidationContext
   {
   noValidation,
   preCodegenValidation,
   postILgenValidation
   // NOTE: Please add any new ILValidationContext here!
   };

/**
 * Example
 *
 * At any point after ILgeneration, a call of the following form will
 * validate the IL generated from the asscoiated MethodSymbol based on
 * the employed ILValidationStrategy.
 *
 * comp->validateIL(TR::omrValidationStrategies[OMR::preCodegenValidation]);
 */
const OMR::ILValidationStrategy *omrValidationStrategies[] =
   {
   OMR::emptyStrategy,
   OMR::preCodegenValidationStrategy,
   OMR::postILgenValidatonStrategy
   };

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

//      const ILValidationStrategy * javaValidationStrategy[];

} // namespace TR

#endif
