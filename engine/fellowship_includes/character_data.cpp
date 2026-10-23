
#include "character_data.hpp"
#include "generated/character_data.inc"

util::span<const character_csv_lookup_t> character_csv_lookup_t::data()
{
  return util::make_span( __character_csv_lookup ).subspan( 0 );
}

const character_csv_lookup_t* character_csv_lookup_t::find( util::string_view name )
{
  const auto __data = data();

  auto it = range::find( __data, name, &character_csv_lookup_t::name_cstr );
  if ( it != __data.end() )
    return &*it;

  return nullptr;
}