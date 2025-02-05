////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2020 ArangoDB GmbH, Cologne, Germany
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
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "Basics/Common.h"

#include "gtest/gtest.h"
#include "Mocks/Servers.h"
#include "Mocks/StorageEngineMock.h"

#include "Aql/Projections.h"
#include "Indexes/IndexIterator.h"
#include "VocBase/Identifiers/DataSourceId.h"

#include <velocypack/Builder.h>
#include <velocypack/Parser.h>
#include <velocypack/Slice.h>

using namespace arangodb;
using namespace arangodb::aql;
using namespace arangodb::tests;

TEST(ProjectionsTest, buildEmpty) {
  Projections p;

  EXPECT_EQ(0, p.size());
  EXPECT_TRUE(p.empty());
  EXPECT_FALSE(p.isSingle("a"));
  EXPECT_FALSE(p.isSingle("_key"));
}

TEST(ProjectionsTest, buildSingleKey) {
  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath("_key"),
  };
  Projections p(std::move(attributes));

  EXPECT_EQ(1, p.size());
  EXPECT_FALSE(p.empty());
  EXPECT_EQ(AttributeNamePath("_key"), p[0].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::KeyAttribute, p[0].type);
  EXPECT_FALSE(p.isSingle("a"));
  EXPECT_TRUE(p.isSingle("_key"));
}

TEST(ProjectionsTest, buildSingleId) {
  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath("_id"),
  };
  Projections p(std::move(attributes));

  EXPECT_EQ(1, p.size());
  EXPECT_FALSE(p.empty());
  EXPECT_EQ(AttributeNamePath("_id"), p[0].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::IdAttribute, p[0].type);
  EXPECT_FALSE(p.isSingle("a"));
  EXPECT_TRUE(p.isSingle("_id"));
}

TEST(ProjectionsTest, buildSingleFrom) {
  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath("_from"),
  };
  Projections p(std::move(attributes));

  EXPECT_EQ(1, p.size());
  EXPECT_FALSE(p.empty());
  EXPECT_EQ(AttributeNamePath("_from"), p[0].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::FromAttribute, p[0].type);
  EXPECT_FALSE(p.isSingle("a"));
  EXPECT_TRUE(p.isSingle("_from"));
}

TEST(ProjectionsTest, buildSingleTo) {
  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath("_to"),
  };
  Projections p(std::move(attributes));

  EXPECT_EQ(1, p.size());
  EXPECT_FALSE(p.empty());
  EXPECT_EQ(AttributeNamePath("_to"), p[0].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::ToAttribute, p[0].type);
  EXPECT_FALSE(p.isSingle("a"));
  EXPECT_TRUE(p.isSingle("_to"));
}

TEST(ProjectionsTest, buildSingleOther) {
  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath("piff"),
  };
  Projections p(std::move(attributes));

  EXPECT_EQ(1, p.size());
  EXPECT_FALSE(p.empty());
  EXPECT_EQ(AttributeNamePath("piff"), p[0].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::SingleAttribute, p[0].type);
  EXPECT_FALSE(p.isSingle("a"));
  EXPECT_TRUE(p.isSingle("piff"));
}

TEST(ProjectionsTest, buildMulti) {
  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath("a"),
      AttributeNamePath("b"),
      AttributeNamePath("c"),
  };
  Projections p(std::move(attributes));

  EXPECT_EQ(3, p.size());
  EXPECT_FALSE(p.empty());
  EXPECT_EQ(AttributeNamePath("a"), p[0].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::SingleAttribute, p[0].type);
  EXPECT_EQ(AttributeNamePath("b"), p[1].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::SingleAttribute, p[1].type);
  EXPECT_EQ(AttributeNamePath("c"), p[2].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::SingleAttribute, p[2].type);
  EXPECT_FALSE(p.isSingle("a"));
  EXPECT_FALSE(p.isSingle("b"));
  EXPECT_FALSE(p.isSingle("c"));
  EXPECT_FALSE(p.isSingle("_key"));
}

TEST(ProjectionsTest, buildReverse) {
  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath("c"),
      AttributeNamePath("b"),
      AttributeNamePath("a"),
  };
  Projections p(std::move(attributes));

  EXPECT_EQ(3, p.size());
  EXPECT_FALSE(p.empty());
  EXPECT_EQ(AttributeNamePath("a"), p[0].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::SingleAttribute, p[0].type);
  EXPECT_EQ(AttributeNamePath("b"), p[1].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::SingleAttribute, p[1].type);
  EXPECT_EQ(AttributeNamePath("c"), p[2].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::SingleAttribute, p[2].type);
  EXPECT_FALSE(p.isSingle("a"));
  EXPECT_FALSE(p.isSingle("b"));
  EXPECT_FALSE(p.isSingle("c"));
  EXPECT_FALSE(p.isSingle("_key"));
}

TEST(ProjectionsTest, buildWithSystem) {
  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath("a"),
      AttributeNamePath("_key"),
      AttributeNamePath("_id"),
  };
  Projections p(std::move(attributes));

  EXPECT_EQ(3, p.size());
  EXPECT_FALSE(p.empty());
  EXPECT_EQ(AttributeNamePath("_id"), p[0].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::IdAttribute, p[0].type);
  EXPECT_EQ(AttributeNamePath("_key"), p[1].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::KeyAttribute, p[1].type);
  EXPECT_EQ(AttributeNamePath("a"), p[2].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::SingleAttribute, p[2].type);
  EXPECT_FALSE(p.isSingle("a"));
  EXPECT_FALSE(p.isSingle("_key"));
  EXPECT_FALSE(p.isSingle("_id"));
}

TEST(ProjectionsTest, buildNested1) {
  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath(std::vector<std::string>({"a", "b"})),
      AttributeNamePath("_key"),
      AttributeNamePath(std::vector<std::string>({"a", "z", "A"})),
  };
  Projections p(std::move(attributes));

  EXPECT_EQ(3, p.size());
  EXPECT_FALSE(p.empty());
  EXPECT_EQ(AttributeNamePath("_key"), p[0].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::KeyAttribute, p[0].type);
  EXPECT_EQ(AttributeNamePath(std::vector<std::string>({{"a"}, {"b"}})),
            p[1].path);
  EXPECT_EQ(AttributeNamePath(std::vector<std::string>({{"a"}, {"z"}, {"A"}})),
            p[2].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::MultiAttribute, p[1].type);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::MultiAttribute, p[2].type);
  EXPECT_FALSE(p.isSingle("a"));
  EXPECT_FALSE(p.isSingle("b"));
  EXPECT_FALSE(p.isSingle("z"));
  EXPECT_FALSE(p.isSingle("_key"));
}

TEST(ProjectionsTest, buildNested2) {
  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath(std::vector<std::string>({"b", "b"})),
      AttributeNamePath("_key"),
      AttributeNamePath(std::vector<std::string>({"a", "z", "A"})),
      AttributeNamePath("A"),
  };
  Projections p(std::move(attributes));

  EXPECT_EQ(4, p.size());
  EXPECT_FALSE(p.empty());
  EXPECT_EQ(AttributeNamePath("A"), p[0].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::SingleAttribute, p[0].type);
  EXPECT_EQ(AttributeNamePath("_key"), p[1].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::KeyAttribute, p[1].type);
  EXPECT_EQ(AttributeNamePath(std::vector<std::string>({{"a"}, {"z"}, {"A"}})),
            p[2].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::MultiAttribute, p[2].type);
  EXPECT_EQ(AttributeNamePath(std::vector<std::string>({{"b"}, {"b"}})),
            p[3].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::MultiAttribute, p[3].type);
}

TEST(ProjectionsTest, buildOverlapping1) {
  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath("a"),
      AttributeNamePath(std::vector<std::string>({"a", "b", "c"})),
  };
  Projections p(std::move(attributes));

  EXPECT_EQ(1, p.size());
  EXPECT_EQ(AttributeNamePath("a"), p[0].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::SingleAttribute, p[0].type);
}

TEST(ProjectionsTest, buildOverlapping2) {
  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath(std::vector<std::string>({"a", "b", "c"})),
      AttributeNamePath("a"),
  };
  Projections p(std::move(attributes));

  EXPECT_EQ(1, p.size());
  EXPECT_EQ(AttributeNamePath("a"), p[0].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::SingleAttribute, p[0].type);
}

TEST(ProjectionsTest, buildOverlapping3) {
  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath(std::vector<std::string>({"a", "b", "c"})),
      AttributeNamePath(std::vector<std::string>({"a", "b"})),
  };
  Projections p(std::move(attributes));

  EXPECT_EQ(1, p.size());
  EXPECT_EQ(AttributeNamePath(std::vector<std::string>({{"a"}, {"b"}})),
            p[0].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::MultiAttribute, p[0].type);
}

TEST(ProjectionsTest, buildOverlapping4) {
  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath("m"),
      AttributeNamePath(std::vector<std::string>({"a", "b", "c"})),
      AttributeNamePath(std::vector<std::string>({"a", "b", "c"})),
      AttributeNamePath("b"),
  };
  Projections p(std::move(attributes));

  EXPECT_EQ(3, p.size());
  EXPECT_EQ(AttributeNamePath(std::vector<std::string>({{"a"}, {"b"}, {"c"}})),
            p[0].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::MultiAttribute, p[0].type);
  EXPECT_EQ(AttributeNamePath("b"), p[1].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::SingleAttribute, p[1].type);
  EXPECT_EQ(AttributeNamePath("m"), p[2].path);
  EXPECT_EQ(arangodb::aql::AttributeNamePath::Type::SingleAttribute, p[2].type);
}

TEST(ProjectionsTest, buildOverlapping5) {
  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath("a"),
      AttributeNamePath(std::vector<std::string>({"a", "b"})),
      AttributeNamePath(std::vector<std::string>({"a", "c"})),
  };
  Projections p(std::move(attributes));

  EXPECT_EQ(1, p.size());
  EXPECT_EQ(AttributeNamePath("a"), p[0].path);
}

TEST(ProjectionsTest, buildOverlapping6) {
  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath(std::vector<std::string>({"a", "c"})),
      AttributeNamePath(std::vector<std::string>({"a", "b"})),
  };
  Projections p(std::move(attributes));

  EXPECT_EQ(2, p.size());
  EXPECT_EQ(AttributeNamePath(std::vector<std::string>({{"a"}, {"b"}})),
            p[0].path);
  EXPECT_EQ(AttributeNamePath(std::vector<std::string>({{"a"}, {"c"}})),
            p[1].path);
}

TEST(ProjectionsTest, toVelocyPackFromDocumentSimple1) {
  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath(std::vector<std::string>({"a"})),
      AttributeNamePath(std::vector<std::string>({"b"})),
      AttributeNamePath(std::vector<std::string>({"c"})),
  };
  Projections p(std::move(attributes));

  velocypack::Builder out;

  auto prepareResult = [&p](std::string_view json, velocypack::Builder& out) {
    auto document = velocypack::Parser::fromJson(json.data(), json.size());
    out.clear();
    out.openObject();
    p.toVelocyPackFromDocument(out, document->slice(), nullptr);
    out.close();
    return out.slice();
  };

  {
    VPackSlice s = prepareResult("{}", out);
    EXPECT_EQ(3, s.length());
    EXPECT_TRUE(s.hasKey("a"));
    EXPECT_TRUE(s.get("a").isNull());
    EXPECT_TRUE(s.hasKey("b"));
    EXPECT_TRUE(s.get("b").isNull());
    EXPECT_TRUE(s.hasKey("c"));
    EXPECT_TRUE(s.get("c").isNull());
  }

  {
    VPackSlice s = prepareResult("{\"z\":1}", out);
    EXPECT_EQ(3, s.length());
    EXPECT_TRUE(s.hasKey("a"));
    EXPECT_TRUE(s.get("a").isNull());
    EXPECT_TRUE(s.hasKey("b"));
    EXPECT_TRUE(s.get("b").isNull());
    EXPECT_TRUE(s.hasKey("c"));
    EXPECT_TRUE(s.get("c").isNull());
  }

  {
    VPackSlice s = prepareResult("{\"a\":null,\"b\":1,\"c\":true}", out);
    EXPECT_EQ(3, s.length());
    EXPECT_TRUE(s.get("a").isNull());
    EXPECT_TRUE(s.get("b").isInteger());
    EXPECT_TRUE(s.get("c").getBool());
  }

  {
    VPackSlice s = prepareResult("{\"a\":1,\"b\":false,\"z\":\"foo\"}", out);
    EXPECT_EQ(3, s.length());
    EXPECT_TRUE(s.get("a").isInteger());
    EXPECT_FALSE(s.get("b").getBool());
    EXPECT_TRUE(s.get("c").isNull());
  }

  {
    VPackSlice s = prepareResult(
        "{\"a\":{\"subA\":1},\"b\":{\"subB\":false},\"c\":{\"subC\":\"foo\"}}",
        out);
    EXPECT_EQ(3, s.length());
    EXPECT_TRUE(s.get("a").isObject());
    EXPECT_TRUE(s.get({"a", "subA"}).isInteger());
    EXPECT_TRUE(s.get("b").isObject());
    EXPECT_TRUE(s.get({"b", "subB"}).isBoolean());
    EXPECT_TRUE(s.get("c").isObject());
    EXPECT_TRUE(s.get({"c", "subC"}).isString());
  }
}

TEST(ProjectionsTest, toVelocyPackFromDocumentComplex) {
  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath(std::vector<std::string>({"a", "b", "c"})),
      AttributeNamePath(std::vector<std::string>({"a", "b", "d"})),
      AttributeNamePath(std::vector<std::string>({"a", "c"})),
      AttributeNamePath(std::vector<std::string>({"a", "d", "e", "f"})),
      AttributeNamePath(std::vector<std::string>({"a", "z"})),
  };
  Projections p(std::move(attributes));

  velocypack::Builder out;

  auto prepareResult = [&p](std::string_view json, velocypack::Builder& out) {
    auto document = velocypack::Parser::fromJson(json.data(), json.size());
    out.clear();
    out.openObject();
    p.toVelocyPackFromDocument(out, document->slice(), nullptr);
    out.close();
    return out.slice();
  };

  {
    VPackSlice s = prepareResult("{}", out);
    EXPECT_EQ(1, s.length());
    EXPECT_TRUE(s.hasKey("a"));
    EXPECT_TRUE(s.get("a").isNull());
  }

  {
    VPackSlice s = prepareResult("{\"z\":1}", out);
    EXPECT_EQ(1, s.length());
    EXPECT_TRUE(s.hasKey("a"));
    EXPECT_TRUE(s.get("a").isNull());
  }

  {
    VPackSlice s = prepareResult("{\"a\":null}", out);
    EXPECT_EQ(1, s.length());
    EXPECT_TRUE(s.hasKey("a"));
    EXPECT_TRUE(s.get("a").isNull());
  }

  {
    VPackSlice s = prepareResult("{\"a\":1}", out);
    EXPECT_EQ(1, s.length());
    EXPECT_TRUE(s.hasKey("a"));
    EXPECT_TRUE(s.get("a").isInteger());
  }

  {
    VPackSlice s = prepareResult("{\"a\":\"foo\"}", out);
    EXPECT_EQ(1, s.length());
    EXPECT_TRUE(s.hasKey("a"));
    EXPECT_TRUE(s.get("a").isString());
  }

  {
    VPackSlice s = prepareResult(
        "{\"a\":{\"b\":{\"c\":1,\"d\":2},\"c\":\"foo\",\"d\":{\"e\":\"fuxx\"},"
        "\"z\":\"raton\"}}",
        out);
    EXPECT_EQ(1, s.length());
    EXPECT_TRUE(s.hasKey("a"));
    EXPECT_TRUE(s.get("a").isObject());
    EXPECT_TRUE(s.get({"a", "b"}).isObject());
    EXPECT_TRUE(s.get({"a", "b", "c"}).isInteger());
    EXPECT_TRUE(s.get({"a", "b", "d"}).isInteger());
    EXPECT_TRUE(s.get({"a", "c"}).isString());
    EXPECT_TRUE(s.get({"a", "d", "e"}).isString());
    EXPECT_TRUE(s.get({"a", "d", "f"}).isNone());
    EXPECT_TRUE(s.get({"a", "z"}).isString());
  }

  {
    VPackSlice s = prepareResult(
        "{\"a\":{\"b\":{\"c\":1,\"d\":2},\"c\":\"foo\",\"d\":{\"e\":{\"f\":4}},"
        "\"z\":\"raton\"}}",
        out);
    EXPECT_EQ(1, s.length());
    EXPECT_TRUE(s.hasKey("a"));
    EXPECT_TRUE(s.get("a").isObject());
    EXPECT_TRUE(s.get({"a", "b"}).isObject());
    EXPECT_TRUE(s.get({"a", "b", "c"}).isInteger());
    EXPECT_TRUE(s.get({"a", "b", "d"}).isInteger());
    EXPECT_TRUE(s.get({"a", "c"}).isString());
    EXPECT_TRUE(s.get({"a", "d"}).isObject());
    EXPECT_TRUE(s.get({"a", "d", "e", "f"}).isInteger());
    EXPECT_TRUE(s.get({"a", "z"}).isString());
  }
}

TEST(ProjectionsTest, toVelocyPackFromIndexSimple) {
  mocks::MockAqlServer server;
  auto& vocbase = server.getSystemDatabase();
  auto collectionJson = velocypack::Parser::fromJson("{\"name\":\"test\"}");
  auto logicalCollection = vocbase.createCollection(collectionJson->slice());

  bool created;
  auto indexJson = velocypack::Parser::fromJson(
      "{\"type\":\"hash\", \"fields\":[\"a\", \"b\"]}");
  auto index = logicalCollection->createIndex(indexJson->slice(), created);

  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath(std::vector<std::string>({"a"})),
      AttributeNamePath(std::vector<std::string>({"b"})),
  };
  Projections p(std::move(attributes));

  p.setCoveringContext(DataSourceId(1), index);

  velocypack::Builder out;

  auto prepareResult = [&p](std::string_view json, velocypack::Builder& out) {
    auto document = velocypack::Parser::fromJson(json.data(), json.size());
    out.clear();
    out.openObject();
    auto data = IndexIterator::SliceCoveringData(document->slice());
    p.toVelocyPackFromIndex(out, data, nullptr);
    out.close();
    return out.slice();
  };

  EXPECT_TRUE(index->covers(p));

  {
    VPackSlice s = prepareResult("[null,1]", out);
    EXPECT_EQ(2, s.length());
    EXPECT_TRUE(s.get("a").isNull());
    EXPECT_TRUE(s.get("b").isInteger());
  }

  {
    VPackSlice s = prepareResult("[\"1234\",true]", out);
    EXPECT_EQ(2, s.length());
    EXPECT_TRUE(s.get("a").isString());
    EXPECT_TRUE(s.get("b").getBool());
  }
}

TEST(ProjectionsTest, toVelocyPackFromIndexComplex1) {
  mocks::MockAqlServer server;
  auto& vocbase = server.getSystemDatabase();
  auto collectionJson = velocypack::Parser::fromJson("{\"name\":\"test\"}");
  auto logicalCollection = vocbase.createCollection(collectionJson->slice());

  bool created;
  auto indexJson = velocypack::Parser::fromJson(
      "{\"type\":\"hash\", \"fields\":[\"sub.a\", \"sub.b\", \"c\"]}");
  auto index = logicalCollection->createIndex(indexJson->slice(), created);

  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath(std::vector<std::string>({"sub", "a"})),
      AttributeNamePath(std::vector<std::string>({"sub", "b"})),
      AttributeNamePath(std::vector<std::string>({"c"})),
  };
  Projections p(std::move(attributes));

  p.setCoveringContext(DataSourceId(1), index);

  velocypack::Builder out;

  auto prepareResult = [&p](std::string_view json, velocypack::Builder& out) {
    auto document = velocypack::Parser::fromJson(json.data(), json.size());
    out.clear();
    out.openObject();
    auto data = IndexIterator::SliceCoveringData(document->slice());
    p.toVelocyPackFromIndex(out, data, nullptr);
    out.close();
    return out.slice();
  };

  EXPECT_TRUE(index->covers(p));

  {
    VPackSlice s = prepareResult("[\"foo\",\"bar\",false]", out);
    EXPECT_EQ(2, s.length());
    EXPECT_TRUE(s.get({"sub", "a"}).isString());
    EXPECT_TRUE(s.get({"sub", "b"}).isString());
    EXPECT_FALSE(s.get("c").getBool());
  }
}

TEST(ProjectionsTest, toVelocyPackFromIndexComplex2) {
  mocks::MockAqlServer server;
  auto& vocbase = server.getSystemDatabase();
  auto collectionJson = velocypack::Parser::fromJson("{\"name\":\"test\"}");
  auto logicalCollection = vocbase.createCollection(collectionJson->slice());

  bool created;
  auto indexJson = velocypack::Parser::fromJson(
      "{\"type\":\"hash\", \"fields\":[\"sub\", \"c\"]}");
  auto index = logicalCollection->createIndex(indexJson->slice(), created);

  std::vector<arangodb::aql::AttributeNamePath> attributes = {
      AttributeNamePath(std::vector<std::string>({"sub", "a"})),
      AttributeNamePath(std::vector<std::string>({"sub", "b"})),
      AttributeNamePath(std::vector<std::string>({"c"})),
  };
  Projections p(std::move(attributes));

  p.setCoveringContext(DataSourceId(1), index);

  velocypack::Builder out;

  auto prepareResult = [&p](std::string_view json, velocypack::Builder& out) {
    auto document = velocypack::Parser::fromJson(json.data(), json.size());
    out.clear();
    out.openObject();
    auto data = IndexIterator::SliceCoveringData(document->slice());
    p.toVelocyPackFromIndex(out, data, nullptr);
    out.close();
    return out.slice();
  };

  EXPECT_TRUE(index->covers(p));

  {
    VPackSlice s = prepareResult("[{\"a\":\"foo\",\"b\":\"bar\"},false]", out);
    EXPECT_EQ(2, s.length());
    EXPECT_TRUE(s.get({"sub", "a"}).isString());
    EXPECT_TRUE(s.get({"sub", "b"}).isString());
    EXPECT_FALSE(s.get("c").getBool());
  }
}
