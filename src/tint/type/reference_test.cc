// Copyright 2022 The Tint Authors.
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

#include "src/tint/type/reference.h"
#include "src/tint/type/test_helper.h"

namespace tint::type {
namespace {

using ReferenceTest = TestHelper;

TEST_F(ReferenceTest, Creation) {
    auto* a = create<Reference>(create<type::I32>(), ast::AddressSpace::kStorage,
                                ast::Access::kReadWrite);
    auto* b = create<Reference>(create<type::I32>(), ast::AddressSpace::kStorage,
                                ast::Access::kReadWrite);
    auto* c = create<Reference>(create<type::F32>(), ast::AddressSpace::kStorage,
                                ast::Access::kReadWrite);
    auto* d = create<Reference>(create<type::I32>(), ast::AddressSpace::kPrivate,
                                ast::Access::kReadWrite);
    auto* e =
        create<Reference>(create<type::I32>(), ast::AddressSpace::kStorage, ast::Access::kRead);

    EXPECT_TRUE(a->StoreType()->Is<type::I32>());
    EXPECT_EQ(a->AddressSpace(), ast::AddressSpace::kStorage);
    EXPECT_EQ(a->Access(), ast::Access::kReadWrite);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_NE(a, e);
}

TEST_F(ReferenceTest, Hash) {
    auto* a = create<Reference>(create<type::I32>(), ast::AddressSpace::kStorage,
                                ast::Access::kReadWrite);
    auto* b = create<Reference>(create<type::I32>(), ast::AddressSpace::kStorage,
                                ast::Access::kReadWrite);
    auto* c = create<Reference>(create<type::F32>(), ast::AddressSpace::kStorage,
                                ast::Access::kReadWrite);
    auto* d = create<Reference>(create<type::I32>(), ast::AddressSpace::kPrivate,
                                ast::Access::kReadWrite);
    auto* e =
        create<Reference>(create<type::I32>(), ast::AddressSpace::kStorage, ast::Access::kRead);

    EXPECT_EQ(a->Hash(), b->Hash());
    EXPECT_NE(a->Hash(), c->Hash());
    EXPECT_NE(a->Hash(), d->Hash());
    EXPECT_NE(a->Hash(), e->Hash());
}

TEST_F(ReferenceTest, Equals) {
    auto* a = create<Reference>(create<type::I32>(), ast::AddressSpace::kStorage,
                                ast::Access::kReadWrite);
    auto* b = create<Reference>(create<type::I32>(), ast::AddressSpace::kStorage,
                                ast::Access::kReadWrite);
    auto* c = create<Reference>(create<type::F32>(), ast::AddressSpace::kStorage,
                                ast::Access::kReadWrite);
    auto* d = create<Reference>(create<type::I32>(), ast::AddressSpace::kPrivate,
                                ast::Access::kReadWrite);
    auto* e =
        create<Reference>(create<type::I32>(), ast::AddressSpace::kStorage, ast::Access::kRead);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(*e));
    EXPECT_FALSE(a->Equals(type::Void{}));
}

TEST_F(ReferenceTest, FriendlyName) {
    auto* r = create<Reference>(create<type::I32>(), ast::AddressSpace::kNone, ast::Access::kRead);
    EXPECT_EQ(r->FriendlyName(Symbols()), "ref<i32, read>");
}

TEST_F(ReferenceTest, FriendlyNameWithAddressSpace) {
    auto* r =
        create<Reference>(create<type::I32>(), ast::AddressSpace::kWorkgroup, ast::Access::kRead);
    EXPECT_EQ(r->FriendlyName(Symbols()), "ref<workgroup, i32, read>");
}

}  // namespace
}  // namespace tint::type