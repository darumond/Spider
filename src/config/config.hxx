#pragma once

inline bool Config::is_valid_config() const
{
    return valid_config_;
}
inline std::vector<std::shared_ptr<VHost>>& Config::get_vhost()
{
    return vhosts;
}