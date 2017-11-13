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

#include "ras/BlockValidationRules.hpp"
#include "ras/ILValidationUtils.hpp"


TR::ValidateNodeRefCountWithinBlock::ValidateNodeRefCountWithinBlock(TR::Compilation *comp)
   : _comp(comp)
   {
   }

int32_t TR::ValidateNodeRefCountWithinBlock::validate(TR::TreeTop *firstTreeTop, TR::TreeTop *exitTreeTop)
   {
   _nodeChecklist.empty();
   for (TR::TreeTop *tt = firstTreeTop; tt != exitTreeTop->getNextTreeTop(); tt = tt->getNextTreeTop())
      {
      TR::Node *node = tt->getNode();
      node->setLocalIndex(node->getReferenceCount());
      validateRefCountPass1(node);
      }

   // We start again from the start of the block, and check the localIndex to make sure it is 0.
   // Note, walking the tree backwards causes huge stack usage in validateRefCountPass2.
   _nodeChecklist.empty();
   for (TR::TreeTop *tt = firstTreeTop; tt != exitTreeTop->getNextTreeTop(); tt = tt->getNextTreeTop())
      {
      validateRefCountPass2(tt->getNode());
      }

   return 0;
   }

// In pass 1 (validateRefCountPass1), the Local Index(which is set to the Ref Count) for each child is
// decremented for each visit. The second pass is to make sure that the Local Index is zero
// by the end of the block. A non-zero Local Index would indicate that the Ref count was wrong at the start
// of the Validation Process.
void TR::ValidateNodeRefCountWithinBlock::validateRefCountPass1(TR::Node *node)
   {
   // If this is the first time through this node, verify the children.
   if (!_nodeChecklist.isSet(node->getGlobalIndex()))
      {
      _nodeChecklist.set(node->getGlobalIndex());
      for (int32_t i = node->getNumChildren() - 1; i >= 0; --i)
         {
         TR::Node *child = node->getChild(i);
         if (_nodeChecklist.isSet(child->getGlobalIndex()))
            {
            // If the child has already been visited, decrement its verifyRefCount.
            child->decLocalIndex();
            }
         else
            {
            // If the child has not yet been visited, set its localIndex and visit it
            child->setLocalIndex(child->getReferenceCount() - 1);
            validateRefCountPass1(child);
            }
         }
      }
   }

void TR::ValidateNodeRefCountWithinBlock::validateRefCountPass2(TR::Node *node)
   {
   // Pass through and make sure that the localIndex == 0 for each child.
   if (!_nodeChecklist.isSet(node->getGlobalIndex()))
      {
      _nodeChecklist.set(node->getGlobalIndex());
      for (int32_t i = node->getNumChildren() - 1; i >= 0; --i)
         {
         validateRefCountPass2(node->getChild(i));
         }

      TR::checkCondition(node, node->getLocalIndex() == 0, _comp,
                         "Node accessed outside of its (extended) basic block: %d time(s)",
                         node->getLocalIndex());
      }
   }
