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

// TODO: Put this in the TR namespace.
namespace OMR {

struct ILValidationStrategy
   {
   OMR::ILValidationRules        id;
   OMR::ILValidationOptions      options;
   };


enum ILValidationRules
   {
   // TODO: Implement this.
   detectDeprecatedOpcodes,
   soundnessRule,
   // TODO: Implement this.
   validateBinaryOpcodeChildLayout,
   validateChildCount,
   validateChildTypes,
   validateLivenessBoundaries,
   validateNodeRefCountWithinBlock,
   validate_axaddPlatformSpecificRequirement,
   validate_ireturnReturnType,
   endRules
   };

// TODO: Right now the `IfEnabled` aspect of the _option param is unused.
//       Eventually all of these would have their own options, i.e 
//       TR_validateChildCount etc.
//       Right now all Validation Rules are Enabled by default.
//       Of course they are only invoked if they are part of a ILValidationStrategy
//       being employed by the ILValidator.
// ILValidation Options (these can be applies to individual Rules in a particular Strategy).
enum ILValidationOptions
   {
   // Under a certain strategy, this Rule will always be Validated.
   // Does not abort if Validation fails, still emits error messages.
   AlwaysValidateAndLiberal,
   // Under a certain strategy, this Rule will always be Validated
   // Aborts if Validation fails.
   AlwaysValidateAndStrict,
   // Do only if Enabled. Do not Abort if Validation fails (still emit warnings).
   IfEnabledAndLiberal,
   // Do only if Enabled. Abort if Validation fails.
   IfEnabledAndStrict
   }

// Do not perform any Validation under this Strategy.
const ILValidationStrategy emptyStrategy[] =
   {
   { endRules                                                                }
   };

// Strategy used to Validate right after ILGeneration.
const ILValidationStrategy postILgenValidatonStrategy[] =
   {
   { soundnessRule,                                  AlwaysValidateAndStrict },
   // CLEAN_UP: THIS should fail and hence the need to have a more state aware
   //           form of validation.
   //           Keep for now for testing purposes.
   // This should fail right after ILGeneration.
   // Since currently TreeSimplifier takes care of this.
   { validateBinaryOpcodeChildLayout,                IfEnabledAndStrict },
   { validateChildCount,                             IfEnabledAndStrict },
   { validateChildTypes,                             IfEnabledAndStrict },
   { validateLivenessBoundaries,                     IfEnabledAndStrict },
   { validateNodeRefCountWithinBlock,                IfEnabledAndStrict },
   { detectDeprecatedOpcodes,                        IfEnabledAndLiberal },
   { endRules                                                            }
   };

// Strategy used to Validate right before Codegen.
// At this point the IL is expected to uphold almost all the Validation Rules.
const ILValidationStrategy preCodegenValidationStrategy[] =
   {
   { detectDeprecatedOpcodes,                        IfEnabledAndLiberal },
   { soundnessRule,                                  AlwaysValidateAndStrict },
   { validateBinaryOpcodeChildLayout                 IfEnabledAndStrict },
   { validateChildCount,                             IfEnabledAndStrict },
   { validateChildTypes,                             IfEnabledAndStrict },
   { validateLivenessBoundaries,                     AlwaysValidateAndStrict },
   { validateNodeRefCountWithinBlock,                AlwaysValidateAndStrict },
   { validate_axaddPlatformSpecificRequirement,      IfEnabledAndLiberal },
   { validate_ireturnReturnType,                     IfEnabledAndLiberal },
   { endRules                                                            }
   }

// The most rigours Validation Strategy available. Used for testing purposes.
// Enable all available rules and Abort upon Failure.
const ILValidationStrategy validateAllStrict[] =
   {
   { detectDeprecatedOpcodes,                        IfEnabledAndStrict },
   { soundnessRule,                                  AlwaysValidateAndStrict },
   { validateBinaryOpcodeChildLayout,                AlwaysValidateAndStrict },
   { validateChildCount,                             IfEnabledAndStrict },
   { validateChildTypes,                             IfEnabledAndStrict },
   { validateLivenessBoundaries,                     AlwaysValidateAndStrict },
   { validateNodeRefCountWithinBlock,                AlwaysValidateAndStrict },
   { validate_axaddPlatformSpecificRequirement,      IfEnabledAndStrict },
   { validate_ireturnReturnType,                     IfEnabledAndStrict },
   { endRules                                                           }
   }


enum ILValidationTypes
   {
   noValidation
   preCodegenValidation
   postILgenValidation
   validateAllStrict
   // NOTE: Please add any new ILValidationStrategies here!
   }

//comp()->validateIL(omrValidationStrategies[OMR::preCodegenValidation]);


const ILValidationStrategy *omrValidationStrategies[] =
   {
   emptyStrategy,
   preCodegenValidationStrategy,
   postILgenValidatonStrategy,
   validateAllStrict,
   };

// TODO: Something to look at in the future.
//       Futher down the road, I think we might even want to
//       make ILValidator an extentisible class. Where projects
//       downstream can enforce their own set of Validation Rules,
//       while still having the ability to choose from the ones provided
//       by OMR. This conforms with the idea that the IL doesn't
//       have to be "language-agnostic".
//       For example it makes sense for Java to only use a Subset of the available OpCodes
//       provided by our IL. Which in turn, makes it perfectly reasonable for such
//       a downstream project to employ their own set of restrictions on the said IL.
/*
extern const ILValidationStrategy * javaValidationStrategy[]; 
*/

} //namespace TR


#endif
