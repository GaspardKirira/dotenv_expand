#include <dotenv_expand/dotenv_expand.hpp>

#include <iostream>
#include <unordered_map>

int main()
{
  std::unordered_map<std::string, std::string> env{
      {"HOST", "example.com"}
      // PORT intentionally missing
  };

  std::string url =
      dotenv_expand::expand("https://${HOST}:${PORT:-443}/api", env);

  std::cout << url << "\n";
  return 0;
}
