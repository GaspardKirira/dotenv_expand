#include <dotenv_expand/dotenv_expand.hpp>

#include <iostream>
#include <unordered_map>

int main()
{
  std::unordered_map<std::string, std::string> env{
      {"HOST", "127.0.0.1"},
      {"PORT", "3000"},
      {"BASE", "http://${HOST}:${PORT}"},
      {"V1", "${BASE}/v1"},
      {"V2", "${BASE}/v2"},
  };

  dotenv_expand::expand_inplace(env);

  std::cout << "V1 = " << env["V1"] << "\n";
  std::cout << "V2 = " << env["V2"] << "\n";
  return 0;
}
