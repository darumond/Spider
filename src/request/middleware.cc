#include "request.hh"

// ================= REQUEST HANDLE LOGIC-PART =================

bool request::is_operation_authorised(request::Request& req,
                                      const std::string& path)
{
    req.setResourcePath(get_access_path(req.getCurrentVhost().getRoot(), path));
    if (std::filesystem::is_directory(req.getResourcePath()))
    {
        req.setIsDirectory(true);
        if (req.getMethod() == GET || req.getMethod() == HEAD)
            req.setResourcePath(req.getResourcePath() + "/"
                                + req.getCurrentVhost().getDefaultFile());
        // unauthorized interactions with PUT should lead to 409 (try to
        // access to folder)
        else if (req.getMethod() == PUT)
            return req.set_status_code(
                CONFLICT, false, "is_operation_authorised",
                "put method is not allowed on directory");
        // unauthorized interactions with DELETE should lead to 405 (try to
        // access to folder)
        else if (req.getMethod() == DELETE)
            return req.set_status_code(
                METHOD_NOT_ALLOWED, false, "is_operation_authorised",
                "delete method is not allowed on directory");
    }

    req.setIsDirectory(false);
    // the client does not have the necessary permissions to access the
    // requested resources
    misc::FileDescriptor fd;
    try
    {
        fd = sys::open(req.getResourcePath().c_str(), O_RDONLY);
    }
    catch (std::exception& e)
    {}

    if (errno == EACCES)
        return req.set_status_code(FORBIDDEN, false, "is_operation_authorised",
                                   "permission denied");
    // create file or directory
    else if (errno == ENOENT && req.getMethod() == PUT)
        return req.set_status_code(CREATED, true, "is_operation_authorised",
                                   "a new file has been created");
    // if we want to write to a file that is currently writing
    else if (req.getMethod() == PUT && sys::fcntl_wrapper(fd.fd_, 0) == -1)
        return req.set_status_code(
            CONFLICT, false, "is_operation_authorised",
            "try to access to a file that is used by another vhost");
    // if the file is not found during a GET request the status code
    // should be 404.
    else if (errno == ENOENT)
        return req.set_status_code(NOT_FOUND, false, "is_operation_authorised",
                                   "file " + req.getResourcePath()
                                       + " not found");

    return true;
}