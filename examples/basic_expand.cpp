#include <dotenv_expand/dotenv_expand.hpp>

#include <iostream>
#include <unordered_map>

int main()
{
  std::unordered_map<std::string, std::string> env{
      {"HOST", "localhost"},
      {"PORT", "8080"},
      {"API_URL", "http://${HOST}:${PORT}"},
  };

  dotenv_expand::expand_inplace(env);

  std::cout << "API_URL = " << env["API_URL"] << "\n";
  return 0;
}
