#pragma once
#include <string_view>
#include <cstdint>

#ifdef _MSC_VER
# define CORE_PRETTY_FUNCTION __FUNCSIG__
#else
# define CORE_PRETTY_FUNCTION __PRETTY_FUNCTION__
#endif

using StringID = uint32_t;

namespace core {
  
  namespace detail {
    
    template<typename>
    struct fnv1a;

    template<>
    struct fnv1a<std::uint32_t> {
      static constexpr auto offset = 2166136261U;
      static constexpr auto prime  = 16777619U;
    };

    template<>
    struct fnv1a<std::uint64_t> {
      static constexpr auto offset = 1099511628211ULL;
      static constexpr auto prime  = 14695981039346656037ULL;
    };
  }

  template<typename Type>
  constexpr auto nameof() -> std::string_view {
    const std::string_view func_name{ CORE_PRETTY_FUNCTION };
  
  #if defined __clang__ || defined __GNUC__

    constexpr auto prefix = '=';
    constexpr auto suffix = ']';

    auto begin = func_name.find_first_not_of(' ', func_name.find_first_of(prefix) + 1);
    auto end   = func_name.find_last_of(suffix);
    auto name  = func_name.substr(begin, end - begin);
    return name;

  #else // MSVC 

    constexpr auto prefix = '<';
    constexpr auto suffix = '>';

    if constexpr (!std::is_fundamental_v<Type>)
    {
      auto begin = func_name.find_first_of(prefix) + 1;

      auto space = func_name.find_first_of(' ', begin) + 1;
      auto end = func_name.find_last_of(suffix);

      if (space < end)
      {
        return func_name.substr(space, end - space);
      }

      auto name = func_name.substr(begin, end - begin);

      return name;
    }
    else
    {
      auto begin = func_name.find_first_not_of(' ', func_name.find_first_of(prefix) + 1);
      auto name = func_name.substr(begin, func_name.find_last_of(suffix) - begin);
      return name;
    }

  #endif
  }
  
  constexpr StringID type_sid(const std::string_view name)
  {
    constexpr auto prime = detail::fnv1a<StringID>::prime;
    
    StringID hash = detail::fnv1a<StringID>::offset;
    for (auto&& c : name)
      hash = (hash ^ static_cast<StringID>(c)) * prime;

    return hash;
  }

  template<typename T>
  constexpr StringID type_sid()
  {
    return type_sid(nameof<T>());
  }
  
  template<typename T>
  struct meta {
    using type = std::remove_cv_t<std::remove_pointer_t<std::decay_t<T>>>;
    
    static constexpr std::string_view name = nameof<type>();
    static constexpr StringID hash = type_sid<type>();

    constexpr meta() noexcept = default;
    
    constexpr meta(T&&) noexcept
    { }
    
    constexpr operator StringID() noexcept { return hash; } 
  };

  template<typename T> meta(T&&) -> meta<T>;

  namespace literals {
    constexpr StringID operator""_sid(const char* str, std::size_t size)
    {
      return type_sid({str, size});
    }
  } // namespace literals

}// namespace mq



