#pragma once

#include <string>

#include "config.hh"
#include "types.hh"

// ======================= main utils =======================

bool invalid_server_configuration();
bool is_wildcard_ip(const boost::asio::ip::address& vhost_ip);
bool is_same_ip_type(const VHost& v1, const VHost& v2);
bool check_vhosts_validity();
bool is_default_vhost_unique(const VHost& v1, VHost& v2);

// ======================= request utils =======================

bool valid_method(const std::string& method);
std::string get_next_token(const std::string& request, size_t* cur_pos,
                           const std::string& delimiter);
std::string remove_bracket(const std::string& word);
void to_lower_case(std::string& word);
std::string remove_space_at_begin_and_at_the_end(const std::string& word);
bool contains_space(const std::string& word);
std::string get_access_path(const std::string& root, const std::string& path);
std::pair<std::string, std::string>
get_key_and_value(const std::string& header);