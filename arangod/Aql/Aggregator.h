////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2022 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Jan Steemann
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Aql/AqlValue.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace arangodb {
namespace velocypack {
class Slice;
struct Options;
}  // namespace velocypack

namespace aql {

struct Aggregator {
  Aggregator() = delete;
  Aggregator(Aggregator const&) = delete;
  Aggregator& operator=(Aggregator const&) = delete;

  struct Factory {
    virtual ~Factory() = default;
    virtual std::unique_ptr<Aggregator> operator()(
        velocypack::Options const*) const = 0;
    virtual void createInPlace(void*, velocypack::Options const*) const = 0;
    virtual std::size_t getAggregatorSize() const = 0;
  };

  explicit Aggregator(velocypack::Options const* opts) : _vpackOptions(opts) {}
  virtual ~Aggregator() = default;
  virtual void reset() = 0;
  virtual void reduce(AqlValue const&) = 0;
  virtual AqlValue get() const = 0;
  AqlValue stealValue() {
    AqlValue r = this->get();
    this->reset();
    return r;
  }

  /// @brief creates an aggregator from a name string
  static std::unique_ptr<Aggregator> fromTypeString(velocypack::Options const*,
                                                    std::string_view type);

  /// @brief creates an aggregator from a velocypack slice
  static std::unique_ptr<Aggregator> fromVPack(velocypack::Options const*,
                                               arangodb::velocypack::Slice,
                                               std::string_view nameAttribute);

  /// @brief return a pointer to an aggregator factory for an aggregator type
  /// throws if the aggregator cannot be found
  static Factory const& factoryFromTypeString(std::string_view type);

  /// @brief translates an alias to an actual aggregator name
  /// returns the original value if the name was not an alias
  static std::string_view translateAlias(std::string_view name);

  /// @brief name/type of aggregator to use for the DB server part of the
  /// aggregation when a COLLECT is pushed from coordinator to DB server. for
  /// example, the MAX aggregator is commutative. it can be pushed from the
  /// coordinator to the DB server and be used as a MAX aggregator there too.
  /// other aggregators may need slight adjustment or type changes when they are
  /// pushed to DB servers an empty return value means that the aggregator is
  /// not suitable for being pushed to a DB server
  static std::string_view pushToDBServerAs(std::string_view type);

  /// @brief name/type of aggregator to use for the coordinator part of the
  /// aggregation when a COLLECT is pushed from coordinator to DB server. for
  /// example, the COUNT aggregator is commutative. it can be pushed from the
  /// coordinator to the DB server and be used there too. However, on the
  /// coordinator we must not use COUNT on the aggregated results from the DB
  /// server, but use SUM instead
  static std::string_view runOnCoordinatorAs(std::string_view type);

  /// @brief whether or not the aggregator name is supported and part of the
  /// public API. all internal-only aggregators count as not supported here
  static bool isValid(std::string_view type);

  /// @brief whether or not the aggregator requires any input or if the input
  /// can be optimized away (note current: COUNT/LENGTH don't, all others do)
  static bool requiresInput(std::string_view type);

 protected:
  velocypack::Options const* _vpackOptions;
};

}  // namespace aql
}  // namespace arangodb
