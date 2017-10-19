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

#ifndef OPCODETEST_HPP
#define OPCODETEST_HPP

#include "JitTest.hpp"

#include <string>
#include <tuple>
#include <vector>
#include <initializer_list>
#include <iterator>
#include <limits>

namespace TRTest
{

/**
 * @brief Type for holding argument to parameterized opcode tests
 * @tparam Ret the type returned by the opcode
 * @tparam Args the types of the arguments to the opcode
 *
 * This type is just a tuple that packages the different parts of a argument for
 * an opcode test. The first field is another tuple holding the input values to
 * be given to the opcode for testing. The second field is a two-tuple containing
 * the opcode's name as a string, and a pointer to an oracle function that
 * returns the expected return value of the opcode test when given the input
 * values from the first part of the outer tuple.
 */

// TODO: This is a great idea but unfortunately requires C++11 support. Emulating the same 
//        behaviour without the required C++11 features is nontrivial and only has the opposite
//         effect in complicating the code further. Hence this has been commented out.
//       We should eventually turn this back on, once we move to C++11 supported compilers 
//        (while also sifting through the Test Instances and using lambdas where appropriate).

//template <typename Ret, typename... Args>
//using ParamType = std::tuple<std::tuple<Args...>, std::tuple<std::string, Ret (*)(Args...)> >;

/**
 * @brief Type for holding argument to parameterized binary opcode tests
 */

//template <typename Ret, typename Left, typename Right>
//using BinaryOpParamType = ParamType<Ret, Left, Right>;



// TODO: Once the above has been done we should modify the function signatures appropriately.

template <typename Ret, typename Left, typename Right>
struct BinaryOpParamStruct {
        Left lhs;
        Right rhs;
        std::string opcode;
        Ret (*oracle)(Left, Right);
};

/**
 * @brief Given an instance of BinaryOpParamType, returns an equivalent instance
 *    of BinaryOpParamStruct
 */
template <typename Ret, typename Left, typename Right>
BinaryOpParamStruct<Ret, Left, Right> to_struct(std::tuple<std::tuple<Left,Right>, std::tuple<std::string, Ret (*)(Left,Right)>> param) {
    BinaryOpParamStruct<Ret, Left, Right> s;
    s.lhs = std::get<0>(std::get<0>(param));
    s.rhs = std::get<1>(std::get<0>(param));
    s.opcode = std::get<0>(std::get<1>(param));
    s.oracle = std::get<1>(std::get<1>(param));
    return s;
}

//~ Opcode test fixtures ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template <typename Ret, typename... Args>
class OpCodeTest : public JitTest, public ::testing::WithParamInterface< std::tuple<std::tuple<Args...>, std::tuple<std::string, Ret (*)(Args...)>> > {};

template <typename T>
class BinaryOpTest : public JitTest, public ::testing::WithParamInterface< std::tuple< std::tuple<T,T>, std::tuple<std::string, T (*)(T,T)>> > {};

} // namespace CompTest

#endif // OPCODETEST_HPP
