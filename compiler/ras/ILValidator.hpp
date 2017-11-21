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

#ifndef ILVALIDATOR_HPP
#define ILVALIDATOR_HPP

#include "infra/Flags.hpp"                  // for flags32_t

#include <vector>                           // for templated Vectors.

namespace TR { class BlockValidationRule; }
namespace TR { class Compilation; }
namespace TR { class MethodValidationRule; }
namespace TR { class NodeValidationRule; }

namespace TR {


class ILValidator
   {
   public:
   ILValidator(TR::Compilation *comp);
   ~ILValidator();

   // Validate via the provided rules.
   // CLEAN_UP: Provide a BETTER description as to what this does.
   bool validate();

   enum
      {
      soundnessRule                                     = 0x00000001,
      validateLivenessBoundaries                        = 0x00000002,
      validateNodeRefCountWithinBlock                   = 0x00000004,
      validateChildCount                                = 0x00000008,
      validateChildTypes                                = 0x00000010,
      // TODO: Add the following commented out ones.
//      validateBinaryOpcodeChildLayout                   = 0x00000020,
      validate_ireturnReturnType                        = 0x00000040,
      validate_axaddPlatformSpecificRequirement         = 0x00000080,
//      warnAboutDeprecatedOpcodes                        = 0x00000100,
//      verifyTrees[Maybe]                                       = 0x00000200,
      };

   // TODO: Add flag setting methods


   private:
   TR::Compilation  *_comp;
   TR::Compilation *comp();

   // The Validation rules are categorized into three types based on the
   // "scope" required to validate them.
   // For example, a MethodValidationRule would require information about
   // the entire Method(encapsulated as a TR::ResolvedMethodSymbol) in order
   // to find out if the IL satisifies the given condition.
   // Whereas a NodeValidationRule checks whether a particular TR::Node has
   // some property. Meaning a NodeValidationRule doesn't need to keep track
   // of already encountered nodes or peek into other blocks to see whether
   // a particular Node is valid or not.
   std::vector<TR::MethodValidationRule *> _methodValidationRules;
   std::vector<TR::BlockValidationRule *> _blockValidationRules;
   std::vector<TR::NodeValidationRule *> _nodeValidationRules;

   flags32_t                 _flags;
   };

TR::ILValidator *createILValidatorObject(TR::Compilation *comp);

}


#endif

