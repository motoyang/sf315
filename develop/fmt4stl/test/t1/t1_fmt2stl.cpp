#include <string>
#include <array>
#include <vector>
#include <tuple>
#include <set>
#include <map>

#include <fmt4stl.hpp>
#include <doctest/extend.h>

// --

struct point {
  double x, y;
};

namespace fmt {

template <> struct formatter<point> {
  template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(const point &p, FormatContext &ctx) {
    return format_to(ctx.out(), "({:.1f}, {:.1f})", p.x, p.y);
  }
};

} // namespace fmt

// --

// Customization option 1: Direct partial/full specialization.
// Here we specialize for std::set<std::string>.
template <>
const fmt4stl::delimiters_values<char>
    fmt4stl::delimiters<std::set<std::string>, char>::values = {"|", ", ",
                                                                "|"};

// --

TEST_SUITE("fmt4stl.hpp") {

  TEST_CASE("point") {
    DEFINE_SUBCASE_NO(sc_no);

    SUBCASE(sc_no.c_str()) {
      point p1{3.433, 4.565};
      auto p1_str = fmt::format("{}", p1);
      CHECK(p1_str == "(3.4, 4.6)");
    }
  }

  TEST_CASE("std::valarray") {
    DEFINE_SUBCASE_NO(sc_no);

    SUBCASE(sc_no.c_str()) {
      std::valarray<int> vai{3, 4, 5, 6};
      CHECK(fmt4stl::is_container<decltype(vai)>::value);
      auto vai_str = fmt::format("{}", vai);
      CHECK(vai_str == "[3, 4, 5, 6]");
    }
    SUBCASE(sc_no.c_str()) {
      std::valarray<double> vad{0.3, 0.4, 1.5, 2.6, 3.7};
      CHECK(fmt4stl::is_container<decltype(vad)>::value);
      auto vad_str = fmt::format("{}", vad);
      CHECK(vad_str == "[0.3, 0.4, 1.5, 2.6, 3.7]");
    }
  }

  TEST_CASE("std::array") {
    DEFINE_SUBCASE_NO(sc_no);

    SUBCASE(sc_no.c_str()) {
      std::array<int, 4> ai{3, 4, 5, 6};
      CHECK(fmt4stl::is_container<decltype(ai)>::value);
      auto ai_str = fmt::format("{}", ai);
      CHECK(ai_str == "[3, 4, 5, 6]");
    }
    SUBCASE(sc_no.c_str()) {
      std::array<double, 5> ad{0.3, 0.4, 1.5, 2.6, 3.7};
      CHECK(fmt4stl::is_container<decltype(ad)>::value);
      auto ad_str = fmt::format("{}", ad);
      CHECK(ad_str == "[0.3, 0.4, 1.5, 2.6, 3.7]");
    }
  }

  TEST_CASE("std::vector") {
    DEFINE_SUBCASE_NO(sc_no);

    SUBCASE(sc_no.c_str()) {

      std::vector<int> vi{1, 2, 3, 5};
      auto vi_str = fmt::format("{}", vi);
      CHECK(vi_str == "[1, 2, 3, 5]");

      std::vector<std::string> vs{"abcd", "ddd", "eee", "ffff"};
      auto vs_str = fmt::format("{}", vs);
      CHECK(vs_str == "[\"abcd\", \"ddd\", \"eee\", \"ffff\"]");

      std::vector<std::string_view> vsv{"234", "abc", "345", "eff"};
      auto vsv_str = fmt::format("{}", vsv);
      CHECK(vsv_str == "[\"234\", \"abc\", \"345\", \"eff\"]");
    }
  }

  TEST_CASE("std::set") {
    DEFINE_SUBCASE_NO(sc_no);

    SUBCASE(sc_no.c_str()) {
      std::set<std::string_view> ssv{"234", "abc", "345", "eff"};
      std::set<double> sd{1.2334, 3.345, 4.4335, 2.334, 100.123};
      std::multiset<std::string> mss{"2334", "2334", "abc", "345", "eff"};

      auto ssv_str = fmt::format("{}", ssv);
      CHECK(ssv_str == "{\"234\", \"345\", \"abc\", \"eff\"}");

      auto sd_str = fmt::format("{}", sd);
      CHECK(sd_str == "{1.2334, 2.334, 3.345, 4.4335, 100.123}");

      auto mss_str = fmt::format("{}", mss);
      CHECK(mss_str == "{\"2334\", \"2334\", \"345\", \"abc\", \"eff\"}");
    }

    SUBCASE(sc_no.c_str()) {
      std::set<std::string> ssv{"234", "abc", "345", "eff"};
      auto ss_str = fmt::format("{}", ssv);
      CHECK(ss_str == "|\"234\", \"345\", \"abc\", \"eff\"|");
    }
  }

  TEST_CASE("std::map") {
    DEFINE_SUBCASE_NO(sc_no);

    SUBCASE(sc_no.c_str()) {
      int i1 = 11, i2 = 12, i3 = 13;
      std::map<int, std::string> mis{{1, "abc"}, {2, "cdb"}, {3, "def"}};
      std::map<int, const char *> mis2{{i1, "abc"}, {i2, "cdb"}, {i3, "def"}};

      auto mis_str = fmt::format("{}", mis);
      CHECK(mis_str == "[(1: \"abc\"), (2: \"cdb\"), (3: \"def\")]");

      auto mis2_str = fmt::format("{}", mis2);
      CHECK(mis2_str == "[(11: \"abc\"), (12: \"cdb\"), (13: \"def\")]");
    }
  }

  TEST_CASE("std::pair") {
    DEFINE_SUBCASE_NO(sc_no);

    SUBCASE(sc_no.c_str()) {
      auto p1 = std::make_pair(1, "abc");
      CHECK(fmt4stl::is_container<decltype(p1)>::value == false);

      auto p1_str = fmt::format("{}", p1);
      CHECK(p1_str == "(1: \"abc\")");
    }
  }

  TEST_CASE("std::tuple") {
    DEFINE_SUBCASE_NO(sc_no);

    SUBCASE(sc_no.c_str()) {
      auto t1 = std::make_tuple(11, 'a', '1', "223", "aaaabc", 2.3);
      CHECK(fmt4stl::is_container<decltype(t1)>::value == false);

      auto t1_str = fmt::format("{}", t1);
      CHECK(t1_str == "(11, 'a', '1', \"223\", \"aaaabc\", 2.3)");
    }

    SUBCASE(sc_no.c_str()) {
      int i1 = 11, i2 = 12, i3 = 13;
      std::map<int, std::string> mis{{1, "abc"}, {2, "cdb"}, {3, "def"}};
      std::map<int, const char *> mis2{{i1, "abc"}, {i2, "cdb"}, {i3, "def"}};
      auto p1 = std::make_pair(1, "abc");
      auto t1 =
          std::make_tuple(11, p1, 'a', mis, '1', "223", mis2, "aaaabc", 2.3);

      auto t1_str = fmt::format("{}", t1);
      CHECK(t1_str == "(11, (1: \"abc\"), 'a', [(1: \"abc\"), (2: \"cdb\"), "
                      "(3: \"def\")], '1', \"223\", [(11: \"abc\"), (12: "
                      "\"cdb\"), (13: \"def\")], \"aaaabc\", 2.3)");
    }
  }
}
