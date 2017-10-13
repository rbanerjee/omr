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

#include "ras/ILValidatorUtils.hpp"

#include <stdarg.h>


// CLEAN_UP: I might have some extra ones here. Take them eventually.
#include "il/DataTypes.hpp"
#include "il/Node.hpp"
#include "il/Node_inlines.hpp"
#include "il/ILProps.hpp"
#include "il/ILOps.hpp"
#include "il/Block.hpp"
#include "il/Block_inlines.hpp"
#include "infra/Assert.hpp"


#if defined(DEBUG) || defined(PROD_WITH_ASSUMES)
#define ABORT() TR::trap()
#else
#define ABORT() comp->failCompilation<TR::CompilationException>("Validation error")
#endif

#define FAIL() if (!feGetEnv("TR_continueAfterValidationError")) ABORT()

TR::LiveNodeWindow::LiveNodeWindow(NodeSideTable<NodeState> &sideTable,
                                   TR_Memory *memory)
   :_sideTable(sideTable)
   ,_basis(0)
   ,_liveOffsets(10, memory, stackAlloc, growable)
   {
   }

bool TR::isLoggingEnabled(TR::Compilation *comp)
   {
   // TODO: IL validation should have its own logging option.
   return (comp->getOption(TR_TraceILWalks));
   }

void TR::checkCondition(TR::Node *node, bool condition,
                           TR::Compilation *comp, const char *formatStr, ...)
   {
   if (!condition)
      {
      printDiagnostic("*** VALIDATION ERROR ***\nNode: %s n%dn\nMethod: %s\n",
                      node->getOpCode().getName(), node->getGlobalIndex(),
                      comp->signature());
      va_list args;
      va_start(args, formatStr);
      vprintDiagnostic(formatStr, args);
      va_end(args);
      printDiagnostic("\n");
      FAIL();
      }
   }

// CLEAN_UP: Maybe make it the ref constant.
void TR::updateNodeState(TR::Node *node,
                         NodeSideTable<NodeState>  &nodeStates,
                         LiveNodeWindow &liveNodes, TR::Compilation *comp)
   {
   NodeState &state = nodeStates[node];
   if (node->getReferenceCount() == state._futureReferenceCount)
      {
      // First occurrence -- do some bookkeeping
      if (node->getReferenceCount() == 0)
         {
         checkCondition(node, node->getOpCode().isTreeTop(), comp,
                           "Only nodes with isTreeTop opcodes can have refcount == 0");
         }
      else
         {
         liveNodes.add(node);
         }
      }

   if (liveNodes.contains(node))
      {
      checkCondition(node, state._futureReferenceCount >= 1, comp
                   "Node already has reference count 0");
      if (--state._futureReferenceCount == 0)
         {
         liveNodes.remove(node);
         }
      }
   else
      {
      checkCondition(node, node->getOpCode().isTreeTop(), comp,
                   "Node has already gone dead");
      }

   if (isLoggingEnabled(comp))
      {
      static const char *traceLiveNodesDuringValidation =
                           feGetEnv("TR_traceLiveNodesDuringValidation");
      if (traceLiveNodesDuringValidation && !liveNodes.isEmpty())
         {
         traceMsg(comp, "    -- Live nodes: {");
         char *separator = "";
         for (LiveNodeWindow::Iterator lnwi(liveNodes); lnwi.currentNode(); ++lnwi)
            {
            traceMsg(comp, "%sn%dn", separator,
                     lnwi.currentNode()->getGlobalIndex());
            separator = ", ";
            }
         traceMsg(comp, "}\n");
         }
      }

   }

void TR::printDiagnostic(TR::Compilation *comp, const char *formatStr, ...)
   {
   va_list stderr_args;
   va_start(stderr_args, formatStr);
   vfprintf(stderr, formatStr, stderr_args);
   va_end(stderr_args);
   if (comp->getOutFile() != NULL)
      {
      va_list log_args;
      va_start(log_args, formatStr);
      comp->diagnosticImplVA(formatStr, log_args);
      va_end(log_args);
      }
   }

void TR::vprintDiagnostic(TR::Compilation *comp, const char *formatStr,
                          va_list ap)
   {
   va_list stderr_copy;
   va_copy(stderr_copy, ap);
   vfprintf(stderr, formatStr, stderr_copy);
   va_end(stderr_copy);
   if (comp->getOutFile() != NULL)
      {
      va_list log_copy;
      va_copy(log_copy, ap);
      comp->diagnosticImplVA(formatStr, log_copy);
      va_end(log_copy);
      }
   }
