#include <dotenv_expand/dotenv_expand.hpp>

#include <iostream>
#include <unordered_map>

int main()
{
  std::unordered_map<std::string, std::string> env{
      {"X", "VALUE"},
  };

  std::string s =
      dotenv_expand::expand(R"(Price is 100\$ and var=${X})", env);

  std::cout << s << "\n";
  return 0;
}
