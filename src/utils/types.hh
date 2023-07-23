#pragma once

#define GET "GET"
#define HEAD "HEAD"
#define PUT "PUT"
#define DELETE "DELETE"

#define SP " "
#define CRLF "\r\n"
#define HTTP_VERSION "HTTP/1.1"

enum request_target
{
    ORIGIN_FORM,
    ABSOLUTE_FORM,
    AUTHORITY_FORM,
    ASTERISK_FORM,
    ERROR_FORM,
};

enum STATUS_CODE
{
    OK = 200,
    CREATED = 201,
    BAD_REQUEST = 400,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    CONFLICT = 409,
    UPGRADE_REQUIRED = 426,
    BAD_GATEWAY = 502
};