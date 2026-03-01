#ifndef DOTENV_EXPAND_DOTENV_EXPAND_HPP
#define DOTENV_EXPAND_DOTENV_EXPAND_HPP

/**
 * @file dotenv_expand.hpp
 * @brief Dotenv style variable expansion for strings and key value maps.
 *
 * `dotenv_expand` provides deterministic variable interpolation commonly used in
 * `.env` files and config strings.
 *
 * Supported patterns:
 * - Braced: `${VAR}`
 * - Unbraced: `$VAR`
 * - Default (bash like): `${VAR:-default}`
 *
 * Basic example:
 * @code
 * HOST=localhost
 * PORT=8080
 * API_URL=http://${HOST}:${PORT}
 * @endcode
 *
 * Expansion behavior:
 * - Variables are resolved from a provided map first.
 * - Optionally falls back to the process environment.
 * - Nested variables are expanded (bounded number of passes).
 * - Cycles are prevented by a convergence limit.
 *
 * Escaping:
 * - `\$` produces a literal `$` (no expansion).
 *
 * Windows note:
 * - Environment lookup uses a portable wrapper:
 *   - `_dupenv_s` on MSVC
 *   - `std::getenv` elsewhere
 *
 * Requirements: C++17+
 * Header-only. No external dependencies.
 */

#include <cctype>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace dotenv_expand
{
  /**
   * @brief Expansion options.
   */
  struct Options
  {
    /**
     * @brief When true, unknown variables may be resolved from the process env.
     */
    bool allow_process_env = true;

    /**
     * @brief Maximum number of full expansion passes (prevents cycles).
     */
    int max_passes = 16;
  };

  namespace detail
  {
    inline bool is_var_start(unsigned char c)
    {
      return (std::isalpha(c) != 0) || c == '_';
    }

    inline bool is_var_char(unsigned char c)
    {
      return (std::isalnum(c) != 0) || c == '_';
    }

    /**
     * @brief Portable process environment lookup.
     *
     * Returns empty string when not found.
     *
     * On MSVC, uses `_dupenv_s` to avoid issues with `getenv` security warnings
     * and to ensure the returned buffer ownership is clear.
     */
    inline std::string get_process_env_value(const std::string &key)
    {
#if defined(_MSC_VER)
      // MSVC: use _dupenv_s
      char *buf = nullptr;
      std::size_t len = 0;
      if (_dupenv_s(&buf, &len, key.c_str()) != 0 || buf == nullptr)
        return std::string();

      std::string value(buf);
      std::free(buf);
      return value;
#else
      // POSIX / MinGW / Clang / GCC
      if (const char *v = std::getenv(key.c_str()))
        return std::string(v);
      return std::string();
#endif
    }

    template <typename LookupFn>
    inline std::string expand_once(std::string_view input, LookupFn lookup)
    {
      std::string out;
      out.reserve(input.size());

      auto append_sv = [&](std::string_view sv)
      {
        out.append(sv.data(), sv.size());
      };

      const std::size_t n = input.size();
      std::size_t i = 0;

      while (i < n)
      {
        const char ch = input[i];

        // Escape: \$ => literal $
        if (ch == '\\' && (i + 1 < n) && input[i + 1] == '$')
        {
          out.push_back('$');
          i += 2;
          continue;
        }

        if (ch != '$')
        {
          out.push_back(ch);
          ++i;
          continue;
        }

        // '$' found
        if (i + 1 >= n)
        {
          out.push_back('$');
          ++i;
          continue;
        }

        // Braced: ${...}
        if (input[i + 1] == '{')
        {
          const std::size_t j = i + 2; // after "${"
          const std::size_t close = input.find('}', j);
          if (close == std::string_view::npos)
          {
            // no closing brace: treat '$' as literal
            out.push_back('$');
            ++i;
            continue;
          }

          std::string_view inner = input.substr(j, close - j);

          // Support default syntax: VAR:-default
          std::string_view var_name = inner;
          std::string_view def_value;

          const std::size_t def_pos = inner.find(":-");
          if (def_pos != std::string_view::npos)
          {
            var_name = inner.substr(0, def_pos);
            def_value = inner.substr(def_pos + 2);
          }

          auto ltrim = [](std::string_view s)
          {
            std::size_t k = 0;
            while (k < s.size() && std::isspace(static_cast<unsigned char>(s[k])) != 0)
              ++k;
            return s.substr(k);
          };
          auto rtrim = [](std::string_view s)
          {
            std::size_t k = s.size();
            while (k > 0 && std::isspace(static_cast<unsigned char>(s[k - 1])) != 0)
              --k;
            return s.substr(0, k);
          };

          var_name = rtrim(ltrim(var_name));
          def_value = rtrim(ltrim(def_value));

          // Empty var_name => keep literal chunk
          if (var_name.empty())
          {
            append_sv(input.substr(i, close - i + 1));
            i = close + 1;
            continue;
          }

          const std::string key(var_name);
          std::string resolved = lookup(key);

          if (resolved.empty() && !def_value.empty())
            resolved.assign(def_value.data(), def_value.size());

          out += resolved;
          i = close + 1;
          continue;
        }

        // Unbraced: $VAR
        const unsigned char next = static_cast<unsigned char>(input[i + 1]);
        if (!is_var_start(next))
        {
          // Not a var: literal '$'
          out.push_back('$');
          ++i;
          continue;
        }

        std::size_t j = i + 1;
        while (j < n && is_var_char(static_cast<unsigned char>(input[j])))
          ++j;

        const std::string key(input.substr(i + 1, j - (i + 1)));
        out += lookup(key);
        i = j;
      }

      return out;
    }

    template <typename LookupFn>
    inline std::string expand_with_passes(std::string_view input, LookupFn lookup, const Options &opt)
    {
      std::string current(input);

      for (int pass = 0; pass < opt.max_passes; ++pass)
      {
        std::string next = expand_once(current, lookup);
        if (next == current)
          return current;
        current.swap(next);
      }

      throw std::runtime_error("dotenv_expand: expansion did not converge (cycle suspected)");
    }
  } // namespace detail

  /**
   * @brief Expand variables in a string using a provided map and optional process env fallback.
   *
   * Resolution order:
   * 1) `vars` map
   * 2) process env (if enabled via options)
   *
   * Unknown variables expand to an empty string (dotenv like behavior).
   *
   * @param input Input string.
   * @param vars Key value variables used for expansion.
   * @param opt Expansion options.
   * @return Expanded string.
   * @throws std::runtime_error if expansion does not converge within max passes.
   */
  inline std::string expand(
      std::string_view input,
      const std::unordered_map<std::string, std::string> &vars,
      const Options &opt = {})
  {
    auto lookup = [&](const std::string &key) -> std::string
    {
      auto it = vars.find(key);
      if (it != vars.end())
        return it->second;

      if (opt.allow_process_env)
        return detail::get_process_env_value(key);

      return std::string();
    };

    return detail::expand_with_passes(input, lookup, opt);
  }

  /**
   * @brief Expand variables in a string using only the process environment.
   *
   * Unknown variables expand to an empty string.
   *
   * @param input Input string.
   * @param opt Expansion options.
   * @return Expanded string.
   * @throws std::runtime_error if expansion does not converge within max passes.
   */
  inline std::string expand_env(std::string_view input, Options opt = {})
  {
    opt.allow_process_env = true;

    auto lookup = [&](const std::string &key) -> std::string
    {
      return detail::get_process_env_value(key);
    };

    return detail::expand_with_passes(input, lookup, opt);
  }

  /**
   * @brief Expand variables for all values in a map (in place), using the map itself as the source.
   *
   * This is the common dotenv-expand pattern:
   * - Load a `.env` into a map
   * - Expand each value referencing other keys
   *
   * Resolution order for each key:
   * 1) current map
   * 2) process env (optional)
   *
   * @param vars Map to expand in place.
   * @param opt Expansion options.
   * @throws std::runtime_error if a value does not converge within max passes.
   */
  inline void expand_inplace(
      std::unordered_map<std::string, std::string> &vars,
      const Options &opt = {})
  {
    for (auto &kv : vars)
    {
      auto lookup = [&](const std::string &key) -> std::string
      {
        auto it = vars.find(key);
        if (it != vars.end())
          return it->second;

        if (opt.allow_process_env)
          return detail::get_process_env_value(key);

        return std::string();
      };

      kv.second = detail::expand_with_passes(kv.second, lookup, opt);
    }
  }

} // namespace dotenv_expand

#endif // DOTENV_EXPAND_DOTENV_EXPAND_HPP
