// Copyright 2020 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/symbol_table.h"

#include "gtest/gtest.h"

namespace tint {
namespace {

using SymbolTableTest = testing::Test;

TEST_F(SymbolTableTest, GeneratesSymbolForName) {
  SymbolTable s;
  EXPECT_EQ(Symbol(1), s.Register("name"));
  EXPECT_EQ(Symbol(2), s.Register("another_name"));
}

TEST_F(SymbolTableTest, DeduplicatesNames) {
  SymbolTable s;
  EXPECT_EQ(Symbol(1), s.Register("name"));
  EXPECT_EQ(Symbol(2), s.Register("another_name"));
  EXPECT_EQ(Symbol(1), s.Register("name"));
}

TEST_F(SymbolTableTest, ReturnsNameForSymbol) {
  SymbolTable s;
  auto sym = s.Register("name");
  EXPECT_EQ("name", s.NameFor(sym));
}

TEST_F(SymbolTableTest, ReturnsBlankForMissingSymbol) {
  SymbolTable s;
  EXPECT_EQ("", s.NameFor(Symbol(2)));
}

TEST_F(SymbolTableTest, ReturnsInvalidForBlankString) {
  SymbolTable s;
  EXPECT_FALSE(s.Register("").IsValid());
}

}  // namespace
}  // namespace tint
