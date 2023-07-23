#include "vhost.hh"

#include <boost/log/trivial.hpp>
#include <utility>

VHost::VHost(boost::asio::ip::address ip, int port, std::string serverName,
             std::string root, bool cudFiles, std::string defaultFile,
             bool is_default_vhost, std::string ssl_cert, std::string ssl_key,
             std::shared_ptr<boost::asio::ssl::context>& ssl_context,
             std::optional<Proxy_pass> proxyPass)
    : ip_(std::move(ip))
    , port_(port)
    , server_name_(std::move(serverName))
    , root_(std::move(root))
    , cud_files_(cudFiles)
    , default_file_(std::move(defaultFile))
    , is_default_vhost_(is_default_vhost)
    , ssl_cert_(std::move(ssl_cert))
    , ssl_key_(std::move(ssl_key))
    , proxy_pass_(std::move(proxyPass))
    , ssl_context_(std::shared_ptr<boost::asio::ssl::context>(ssl_context))
{
    is_crypted_ = true;
    ssl_context_->use_certificate_chain_file(ssl_cert_);
    ssl_context_->use_private_key_file(ssl_key_,
                                       boost::asio::ssl::context::pem);
    // ====================== SNI ======================
}

VHost::VHost(boost::asio::ip::address ip, int port, std::string serverName,
             std::string root, bool cudFiles, std::string defaultFile,
             bool is_default_vhost, std::string ssl_cert, std::string ssl_key,
             std::optional<Proxy_pass> proxyPass)
    : ip_(std::move(ip))
    , port_(port)
    , server_name_(std::move(serverName))
    , root_(std::move(root))
    , cud_files_(cudFiles)
    , default_file_(std::move(defaultFile))
    , is_default_vhost_(is_default_vhost)
    , ssl_cert_(std::move(ssl_cert))
    , ssl_key_(std::move(ssl_key))
    , is_crypted_(true)
    , proxy_pass_(std::move(proxyPass))
{}

VHost::VHost(boost::asio::ip::address ip, int port, std::string serverName,
             std::string root, bool cudFiles, std::string defaultFile,
             bool is_default_vhost, std::optional<Proxy_pass> proxyPass)
    : ip_(std::move(ip))
    , port_(port)
    , server_name_(std::move(serverName))
    , root_(std::move(root))
    , cud_files_(cudFiles)
    , default_file_(std::move(defaultFile))
    , is_default_vhost_(is_default_vhost)
    , proxy_pass_(std::move(proxyPass))
{
    is_crypted_ = false;
}

void VHost::print_vhost()
{
    std::clog << "{\n";
    std::clog << "  \"vhosts\": [\n";
    std::clog << "      {\n";

    std::clog << "          \"ip\": " << ip_ << "\n";
    std::clog << "          \"port\": " << port_ << "\n";
    std::clog << "          \"server_name\": " << server_name_ << "\n";
    std::clog << "          \"root\": " << root_ << "\n";
    std::clog << "          \"cud_files\": " << (cud_files_ ? "true" : "false")
              << "\n";
    std::clog << "          \"default_file\": " << default_file_ << "\n";
    std::clog << "          \"is_default_vhost\": "
              << (is_default_vhost_ ? "true" : "false") << "\n";
    std::clog << "          \"is_encrypted\": "
              << (is_crypted_ ? "true" : "false") << "\n";
    std::clog << "          \"ssl_cert\": " << ssl_cert_ << "\n";
    std::clog << "          \"ssl_cert\": " << ssl_key_ << "\n";

    std::clog << "      }\n";
    std::clog << " ]\n";
    std::clog << "}\n";
}

void VHost::display_connection()
{
    BOOST_LOG_TRIVIAL(info)
        << "The socket has received a client. IP: " << ip_.to_string()
        << ", PORT: " << port_;
}
const std::optional<Proxy_pass>& VHost::getProxyPass() const
{
    return proxy_pass_;
}