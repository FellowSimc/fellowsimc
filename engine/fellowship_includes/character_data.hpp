
#pragma once

#include "util/span.hpp"
#include "util/generic.hpp"
#include <string>
#include "util/timespan.hpp"
#include "util/string_view.hpp"

struct character_csv_lookup_t
{
  const char* token_name;
  double _value;
    
  const char* name_cstr() const
  {
    return token_name;
  }
  
  const double value() const
  {
    return _value;
  }

  const timespan_t millis() const
  {
    return timespan_t::from_millis( _value );
  }

  const timespan_t seconds() const
  {
    return timespan_t::from_seconds( _value );
  }

  static const character_csv_lookup_t* find( util::string_view name );
  static util::span<const character_csv_lookup_t> data();
  static const character_csv_lookup_t& nil();
};

struct character_csv_lookup_nil_t : public character_csv_lookup_t
{
  constexpr character_csv_lookup_nil_t() : character_csv_lookup_t()
  {
    token_name = "null";
    _value = 0.0;
  }
};

inline constexpr character_csv_lookup_nil_t character_csv_lookup_nil_v{};
inline const character_csv_lookup_t& character_csv_lookup_t::nil()
{
  return character_csv_lookup_nil_v;
}
