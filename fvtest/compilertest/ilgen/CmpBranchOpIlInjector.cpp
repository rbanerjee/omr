/*******************************************************************************
 * Copyright (c) 2000, 2016 IBM Corp. and others
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

#include "compile/Compilation.hpp"
#include "env/FrontEnd.hpp"
#include "compile/Method.hpp"
#include "ilgen/CmpBranchOpIlInjector.hpp"

namespace TestCompiler
{
bool
CmpBranchOpIlInjector::injectIL()
   {
   if (!isOpCodeSupported())
      return false;
   createBlocks(3);

   // 3 blocks requested start at 2 (0 is entry, 1 is exit)
   // by default, generate to block 2

   // Block2: blocks(0)
   // if () goto Block4;
   ifjump(_opCode, parm(1), parm(2), 2);

   // Block3: blocks(1)
   // return 0;
   returnValue(iconst(0));

   // Block4: blocks(2)
   // return 1;
   generateToBlock(2);
   returnValue(iconst(1));

   return true;
   }

} // namespace TestCompiler