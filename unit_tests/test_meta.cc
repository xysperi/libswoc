/** @file

    Unit tests for ts_meta.h and other meta programming.

    @section license License

    Licensed to the Apache Software Foundation (ASF) under one or more contributor license
    agreements.  See the NOTICE file distributed with this work for additional information regarding
    copyright ownership.  The ASF licenses this file to you under the Apache License, Version 2.0
    (the "License"); you may not use this file except in compliance with the License.  You may
    obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software distributed under the
    License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
    express or implied. See the License for the specific language governing permissions and
    limitations under the License.
 */

#include <cstring>
#include <variant>

#include "swoc/swoc_meta.h"
#include "swoc/TextView.h"

#include "catch.hpp"

using swoc::TextView;
using namespace swoc::literals;

struct A {
  int _value;
};

struct AA : public A {
};

struct B {
  std::string _value;
};

struct C {
};

struct D {
};

// Some example meta-programming, saving it for possible later use.

template <typename...> struct is_any_of_1 {
  static constexpr bool value = false;
};
template <typename T, typename T0> struct is_any_of_1<T, T0> {
  static constexpr bool value = std::is_same<T, T0>::value;
};
template <typename T, typename T0, typename... Rest> struct is_any_of_1<T, T0, Rest...> {
  static constexpr bool value = std::is_same<T, T0>::value || (sizeof...(Rest) > 0 && is_any_of_1<T, Rest...>::value);
};

// Requires C++17
template <typename T, typename... Rest> struct is_any_of_2 {
  static constexpr bool value = std::disjunction<std::is_same<T, Rest>...>::value;
};

TEST_CASE("Meta Example", "[meta][example]")
{
  REQUIRE(is_any_of_1<A, A, B, C>::value);
  REQUIRE(!is_any_of_1<D, A, B, C>::value);
  REQUIRE(is_any_of_1<A, A>::value);
  REQUIRE(!is_any_of_1<A, D>::value);
  REQUIRE(!is_any_of_1<A>::value);

  REQUIRE(is_any_of_2<A, A, B, C>::value);
  REQUIRE(!is_any_of_2<D, A, B, C>::value);
  REQUIRE(is_any_of_2<A, A>::value);
  REQUIRE(!is_any_of_2<A, D>::value);
  REQUIRE(!is_any_of_2<A>::value);
}

// Start of ts::meta testing.

namespace
{
template <typename T>
auto
detect(T &&t, swoc::meta::CaseTag<0>) -> std::string_view
{
  return "none";
}
template <typename T>
auto
detect(T &&t, swoc::meta::CaseTag<1>) -> decltype(t._value, std::string_view())
{
  return "value";
}
template <typename T>
std::string_view
detect(T &&t)
{
  return detect(t, swoc::meta::CaseArg);
}
} // namespace

TEST_CASE("Meta", "[meta]")
{
  REQUIRE(detect(A()) == "value");
  REQUIRE(detect(B()) == "value");
  REQUIRE(detect(C()) == "none");
  REQUIRE(detect(AA()) == "value");
}

TEST_CASE("Meta vary", "[meta][vary]")
{
  std::variant<int, bool, TextView> v;
  auto visitor = swoc::meta::vary{[](int &i) -> int { return i; }, [](bool &b) -> int { return b ? -1 : -2; },
                                  [](TextView &tv) -> int { return swoc::svtou(tv); }};

  v = 37;
  REQUIRE(std::visit(visitor, v) == 37);
  v = true;
  REQUIRE(std::visit(visitor, v) == -1);
  v = "956"_tv;
  REQUIRE(std::visit(visitor, v) == 956);
}
