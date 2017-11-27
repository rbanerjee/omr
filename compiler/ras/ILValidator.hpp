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

#include <vector>                           // for templated Vectors

#include "ras/ILValidationStrategies.hpp"   // for OMR::ILValidationStrategy


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

  // CLEAN_UP: This needs updating.
  /** \brief
   *     ...
   *
   *  \return
   *     ...
   */
   void validate(const OMR::ILValidationStrategy *strategy);

   private:
   TR::Compilation  *_comp;
   TR::Compilation* comp();

   /**
    * The Validation rules are categorized into three types based on the
    *  "scope" required to validate them.
    */

   /**
    * A MethodValidationRule would require information about
    * the entire Method(encapsulated as a TR::ResolvedMethodSymbol) in order
    * to find out if the IL satisifies the given condition.
    */
   std::vector<TR::MethodValidationRule *> _methodValidationRules;
   /**
    * Used for checking properties across an extended Block.
    */
   std::vector<TR::BlockValidationRule *> _blockValidationRules;
   /**
    * NodeValidationRules check whether a particular TR::Node has
    * some property. Meaning a NodeValidationRule doesn't need to keep track
    * of already encountered nodes or peek into other blocks to see whether
    * a particular Node is valid or not.
    */
   std::vector<TR::NodeValidationRule *> _nodeValidationRules;

  // CLEAN_UP: This needs updating.
  // Selects the required set of Rules to validate against based on the passed
  // strategy.
  /** \brief
   *     ...
   *
   *  \return
   *     ...
   */
   std::vector<TR::MethodValidationRule *>
   getRequiredMethodValidationRules(const OMR::ILValidationStrategy *strategy);

   std::vector<TR::BlockValidationRule *>
   getRequiredBlockValidationRules(const OMR::ILValidationStrategy *strategy);

   std::vector<TR::NodeValidationRule *>
   getRequiredNodeValidationRules(const OMR::ILValidationStrategy *strategy);


   };

   TR::ILValidator* createILValidatorObject(TR::Compilation *comp);

}


#endif

