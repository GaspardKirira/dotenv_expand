#include <dotenv_expand/dotenv_expand.hpp>

#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>

namespace
{
  using dotenv_expand::Options;

  static void test_basic_braced()
  {
    std::unordered_map<std::string, std::string> env{
        {"HOST", "localhost"},
        {"PORT", "8080"},
    };

    const std::string in = "http://${HOST}:${PORT}";
    const std::string out = dotenv_expand::expand(in, env);

    assert(out == "http://localhost:8080");
  }

  static void test_basic_unbraced()
  {
    std::unordered_map<std::string, std::string> env{
        {"PASSWORD", "s1mpl3"},
        {"DB_PASS", "$PASSWORD"},
    };

    dotenv_expand::expand_inplace(env);
    assert(env["DB_PASS"] == "s1mpl3");
  }

  static void test_nested()
  {
    std::unordered_map<std::string, std::string> env{
        {"HOST", "127.0.0.1"},
        {"PORT", "3000"},
        {"API", "http://${HOST}:${PORT}"},
        {"API_V1", "${API}/v1"},
    };

    dotenv_expand::expand_inplace(env);
    assert(env["API"] == "http://127.0.0.1:3000");
    assert(env["API_V1"] == "http://127.0.0.1:3000/v1");
  }

  static void test_default_value()
  {
    std::unordered_map<std::string, std::string> env{
        {"HOST", "example.com"},
        // PORT intentionally missing
    };

    const std::string in = "https://${HOST}:${PORT:-443}/api";
    const std::string out = dotenv_expand::expand(in, env);

    assert(out == "https://example.com:443/api");
  }

  static void test_unknown_is_empty()
  {
    std::unordered_map<std::string, std::string> env{
        {"A", "x"},
    };

    const std::string out = dotenv_expand::expand("v=${MISSING}", env);
    assert(out == "v=");
  }

  static void test_escape_dollar()
  {
    std::unordered_map<std::string, std::string> env{
        {"X", "nope"},
    };

    const std::string out = dotenv_expand::expand(R"(price=123\$ USD, x=${X})", env);
    assert(out == "price=123$ USD, x=nope");
  }

  static void test_cycle_detection()
  {
    std::unordered_map<std::string, std::string> env{
        {"A", "${B}"},
        {"B", "${A}"},
    };

    Options opt;
    opt.max_passes = 8;

    bool threw = false;
    try
    {
      dotenv_expand::expand_inplace(env, opt);
    }
    catch (const std::runtime_error &)
    {
      threw = true;
    }

    assert(threw && "cycle should be detected by convergence limit");
  }
} // namespace

int main()
{
  test_basic_braced();
  test_basic_unbraced();
  test_nested();
  test_default_value();
  test_unknown_is_empty();
  test_escape_dollar();
  test_cycle_detection();

  std::cout << "[dotenv_expand] all tests passed\n";
  return 0;
}
