// Copyright 2019 The Dawn Authors
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

#ifndef SRC_DAWN_TESTS_PARAMGENERATOR_H_
#define SRC_DAWN_TESTS_PARAMGENERATOR_H_

#include <tuple>
#include <utility>
#include <vector>

#include "dawn/tests/AdapterTestConfig.h"

namespace dawn {

// ParamStruct is a custom struct which ParamStruct will yield when iterating.
// The types Params... should be the same as the types passed to the constructor
// of ParamStruct.
template <typename ParamStruct, typename... Params>
class ParamGenerator {
    using ParamTuple = std::tuple<std::vector<Params>...>;
    using Index = std::array<size_t, sizeof...(Params)>;

    static constexpr auto s_indexSequence = std::make_index_sequence<sizeof...(Params)>{};

    // Using an N-dimensional Index, extract params from ParamTuple and pass
    // them to the constructor of ParamStruct.
    template <size_t... Is>
    static ParamStruct GetParam(const ParamTuple& params,
                                const Index& index,
                                std::index_sequence<Is...>) {
        return ParamStruct(std::get<Is>(params)[std::get<Is>(index)]...);
    }

    // Get the last value index into a ParamTuple.
    template <size_t... Is>
    static Index GetLastIndex(const ParamTuple& params, std::index_sequence<Is...>) {
        return Index{std::get<Is>(params).size() - 1 ...};
    }

  public:
    using value_type = ParamStruct;

    explicit ParamGenerator(std::vector<Params>... params) : mParams(params...), mIsEmpty(false) {
        for (bool isEmpty : {params.empty()...}) {
            mIsEmpty |= isEmpty;
        }
    }

    class Iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = ParamStruct;
        using difference_type = size_t;
        using pointer = ParamStruct*;
        using reference = ParamStruct&;

        Iterator& operator++() {
            // Increment the Index by 1. If the i'th place reaches the maximum,
            // reset it to 0 and continue with the i+1'th place.
            for (int i = mIndex.size() - 1; i >= 0; --i) {
                if (mIndex[i] >= mLastIndex[i]) {
                    mIndex[i] = 0;
                } else {
                    mIndex[i]++;
                    return *this;
                }
            }

            // Set a marker that the iterator has reached the end.
            mEnd = true;
            return *this;
        }

        bool operator==(const Iterator& other) const {
            return mEnd == other.mEnd && mIndex == other.mIndex;
        }

        bool operator!=(const Iterator& other) const { return !(*this == other); }

        ParamStruct operator*() const { return GetParam(mParams, mIndex, s_indexSequence); }

      private:
        friend class ParamGenerator;

        Iterator(ParamTuple params, Index index)
            : mParams(params), mIndex(index), mLastIndex{GetLastIndex(params, s_indexSequence)} {}

        ParamTuple mParams;
        Index mIndex;
        Index mLastIndex;
        bool mEnd = false;
    };

    Iterator begin() const {
        if (mIsEmpty) {
            return end();
        }
        return Iterator(mParams, {});
    }

    Iterator end() const {
        Iterator iter(mParams, GetLastIndex(mParams, s_indexSequence));
        ++iter;
        return iter;
    }

  private:
    ParamTuple mParams;
    bool mIsEmpty;
};

namespace detail {
std::vector<AdapterTestParam> GetAvailableAdapterTestParamsForBackends(
    const BackendTestConfig* params,
    size_t numParams);
}

template <typename Param, typename... Params>
auto MakeParamGenerator(std::vector<BackendTestConfig>&& first,
                        std::initializer_list<Params>&&... params) {
    return ParamGenerator<Param, AdapterTestParam, Params...>(
        ::dawn::detail::GetAvailableAdapterTestParamsForBackends(first.data(), first.size()),
        std::forward<std::initializer_list<Params>&&>(params)...);
}
template <typename Param, typename... Params>
auto MakeParamGenerator(std::vector<BackendTestConfig>&& first, std::vector<Params>&&... params) {
    return ParamGenerator<Param, AdapterTestParam, Params...>(
        ::dawn::detail::GetAvailableAdapterTestParamsForBackends(first.data(), first.size()),
        std::forward<std::vector<Params>&&>(params)...);
}

}  // namespace dawn

#endif  // SRC_DAWN_TESTS_PARAMGENERATOR_H_
