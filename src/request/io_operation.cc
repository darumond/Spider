#include "io_operation.hh"

void io_operation::put(const std::string& path, const std::string& body)
{
    std::ofstream requested_filed;
    requested_filed.open(path, std::ios::out | std::ios::trunc);
    requested_filed << body;
    requested_filed.close();
}