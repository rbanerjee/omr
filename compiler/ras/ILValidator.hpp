/*******************************************************************************
 * Copyright (c) 2016, 2016 IBM Corp. and others
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

namespace TR { class Compilation; }

namespace TR {

class ILValidator
   {
   public:
   ILValidator(TR::Compilation *comp);

   // Validate via the provided rules.
   // CLEAN_UP: Provdide a BETTER description as to what this does.
   bool validate();

   private:
   TR::Compilation  *_comp;
   std::vector<TR::MethodValidationRule> method_rules;
   std::vector<TR::BlockValidationRule> block_rules;
   std::vector<TR::NodeValidationRule> node_rules;
   };

}

#endif

