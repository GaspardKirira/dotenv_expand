#include <dotenv_expand/dotenv_expand.hpp>

#include <iostream>

int main()
{
  // Example: use system environment
  // On Linux/macOS:
  // export MY_NAME=Gaspard
  //
  // On Windows (PowerShell):
  // setx MY_NAME Gaspard

  std::string result =
      dotenv_expand::expand_env("Hello ${MY_NAME:-Anonymous}");

  std::cout << result << "\n";
  return 0;
}
