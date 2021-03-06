// Copyright (c) 2017 Dr. Colin Hirsch and Daniel Frey
// Please see LICENSE for license or visit https://github.com/taocpp/json/

#ifndef TAOCPP_JSON_INCLUDE_EVENTS_MSGPACK_FROM_STRING_HPP
#define TAOCPP_JSON_INCLUDE_EVENTS_MSGPACK_FROM_STRING_HPP

#include "../../external/pegtl/parse.hpp"

#include "grammar.hpp"

namespace tao
{
   namespace json
   {
      namespace events
      {
         namespace msgpack
         {
            template< typename Consumer >
            void from_string( Consumer& consumer, const char* data, const std::size_t size, const char* source = nullptr, const std::size_t byte = 0, const std::size_t line = 1, const std::size_t byte_in_line = 0 )
            {
               json_pegtl::memory_input< json_pegtl::tracking_mode::LAZY, json_pegtl::eol::lf_crlf, const char* > in( data, data + size, source ? source : "tao::json::msgpack::from_string", byte, line, byte_in_line );
               json_pegtl::parse< msgpack::grammar >( in, consumer );
            }

            template< typename Consumer >
            void from_string( Consumer& consumer, const char* data, const std::size_t size, const std::string& source, const std::size_t byte = 0, const std::size_t line = 1, const std::size_t byte_in_line = 0 )
            {
               msgpack::from_string( consumer, data, size, source.c_str(), byte, line, byte_in_line );
            }

            template< typename Consumer, typename... Ts >
            void from_string( Consumer& consumer, const tao::string_view data, Ts&&... ts )
            {
               msgpack::from_string( consumer, data.data(), data.size(), std::forward< Ts >( ts )... );
            }

         }  // namespace msgpack

      }  // namespace events

   }  // namespace json

}  // namespace tao

#endif
