/*******************************************************************************
 * Copyright (c) 2016, 2017 IBM Corp. and others
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

   ILValidator(TR::Compilation *comp);

   // Validate via the provided rules.
   // CLEAN_UP: Provide a BETTER description as to what this does.
   bool validate();

   private:
   TR::Compilation  *_comp;
   TR::Compilation *comp();
   // CLEAN_UP: The Validation rules are categorized into three types
   //           based on the "scope" required to verify them.
   //           As in, a MethodValidationRule would require information
   //           about the entire Method (encapsulated as a TR::ResolvedMethodSymbol)
   //           in order to find out if the IL satisifies the given condition.
   //           Whereas a NodeValidationRule checks whether a particular TR::Node has some
   //           property. Meaning a NodeValidationRule doesn't need to keep track of 
   //           already encountered nodes or peek into other blocks to see whether a particular
   //           Node is valid or not.
   // CLEAN_UP: Note that currently we don't have any rules that require us to set up
   //           a scope which encompasses only one particular block. However I can't
   //           really see why something like this wouldn't exist in the future. Specially
   //           once we start to add more complicated ones. Also, it's viable to check
   //           "User defined" using the ILValidator class. A set of utilities is provided to 
   //           make this process easier in the form of ILValidationUtils. 
   std::vector<TR::MethodValidationRule> method_rules;
//   std::vector<TR::BlockValidationRule> block_rules;
   std::vector<TR::NodeValidationRule> node_rules;
   };

}

#endif

