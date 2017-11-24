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

#ifndef BLOCKVALIDATIONRULES_HPP
#define BLOCKVALIDATIONRULES_HPP

#include <stdint.h>                            // for int32_t

#include "infra/BitVector.hpp"                 // for TR_BitVector
#include "infra/SideTable.hpp"                 // for NodeSideTable

namespace TR { class Compilation; }


namespace TR {

class BlockValidationRule
   {
   public:
   /**
    * Verify that the IL for a particular extended block has certain properties.
    *
    * @return 0 on success, or a non-zero error code. If non-zero is returned,
    * compilation stops.
    */
   // Upon creation, a method Validation is Strict and set to checked by default.
   // Note, these choices can be overriden by the associated ILValidationStrategy
   // specifying
   BlockValidationRule()
      :_isEnabled(true)
      ,_isStrictRule(true)
      {
      }
   virtual int32_t validate(TR::TreeTop *firstTreeTop, TR::TreeTop *exitTreeTop) = 0;
   protected:
   bool _isEnabled;
   // For a Strict Rule, compilation aborts upon encountering a failure.
   // Rules can be set to "Strict" (or "Lenient") depending on the ILValidationStrategy
   // being employed by the ILValidator.
   bool _isStrictRule;
   };


class ValidateNodeRefCountWithinBlock : public TR::BlockValidationRule
   {
   TR::Compilation  *_comp;
   TR_BitVector  _nodeChecklist;

   public:
   ValidateNodeRefCountWithinBlock(TR::Compilation *comp);
   int32_t validate(TR::TreeTop *firstTreeTop, TR::TreeTop *exitTreeTop);

   private:
   void validateRefCountPass1(TR::Node *node);
   void validateRefCountPass2(TR::Node *node);
   };

} // namespace TR

#endif
