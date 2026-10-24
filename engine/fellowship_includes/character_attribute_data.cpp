
#include "character_attribute_data.hpp"
#include "generated/character_attribute_data.inc"

util::span<const character_attribute_data_t> character_attribute_data_t::data()
{
  return util::make_span( __character_attribute_data ).subspan( 0 );
}

const character_attribute_data_t* character_attribute_data_t::find( util::string_view name )
{
  const auto __data = data();

  auto it = range::find( __data, name, &character_attribute_data_t::name_cstr );
  if ( it != __data.end() )
    return &*it;

  return nullptr;
}