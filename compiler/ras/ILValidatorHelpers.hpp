/*******************************************************************************
 * Copyright (c) 2017 IBM Corp. and others
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
 * This file contains base classes that Validators can be based upon.
 */

#ifndef ILValidatorHelpers_hpp
#define ILValidatorHelpers_hpp

#include <vector>
#include "ras/ILValidator.hpp"

namespace TR {

/**
 * A logical && operation on IL Validators
 * This is similar to: true && ver1() == 0 && ver2() == 0 && ...
 *
 * If all Validators added to this verifier return, then
 * this Validator also returns successfully. On the first call to FAIL(),
 * validation stops and the appropriate diagnostic message is printed.
 */
class AllILValidator : public TR::ILValidator {
   public:

   AllILValidator(TR::Compilation *comp)
   : ILValidator(comp)
   {
   }

   void validateNode(Location &location) {
      for (auto it = validators.begin(); it != validators.end(); ++it) {
          // The Validators are guranteed to call "FAIL()" upon encountering
          //  the breach of a specificied rule and exit based on the defined protocol
          //   See: ILValidator.hpp for the definition of FAIL().
          (*it)->validateNode(location);
      }
   }

   void add(TR::ILValidator *v) {
      validators.push_back(v);
   }

   private:
   std::vector<TR::ILVadlidator*> validators;
};

} // namespace TR

#endif
