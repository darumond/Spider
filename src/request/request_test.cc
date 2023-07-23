// #include <gtest/gtest.h>
//
// #include "../config/config.hh"
// #include "request.hh"
//
//// The fixture for testing class Foo.
// class RequestTest : public ::testing::Test
//{
// protected:
//     VHost vhost_;
//
//     RequestTest()
//     {
//         // You can do set-up work for each test here.
//         Config server_config = Config::getInstance("config.json");
//         vhost_ = server_config.get_vhost()[0];
//     }
// };
//
// TEST_F(RequestTest, GET_ORIGIN_FORM_GOOD) // Good origin form request
//{
//     std::string request = "GET / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
//                           "curl/7.68.0\r\nAccept: */*\r\n\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 200);
// }
//
// TEST_F(RequestTest, GET_PYTHON) // Good origin form request
//{
//     std::string request = "GET / HTTP/1.1\r\nHost: "
//                           "localhost:8000\r\nUser-Agent:
//                           python-requests/2.31.0"
//                           "\r\nAccept-Encoding: gzip, deflate"
//                           "\r\nAccept: */*\r\nConnection: close\r\n\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 200);
// }
//
// TEST_F(RequestTest, GET_ABSOLUTE_FORM_GOOD) // Good absolute form request
//{
//     std::string request =
//         "GET http://localhost/ HTTP/1.1\r\nHost: example.com\r\n\r\n";
//
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 200);
// }
//
// TEST_F(RequestTest,
//        GET_ABSOLUTE_FORM_NO_HOST_VALUE) // No host value in absolute form
//        (200)
//{
//     std::string request = "GET http://localhost/ HTTP/1.1\r\nHost:\r\n\r\n";
//
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 200);
// }
//
// TEST_F(RequestTest, GET_ORIGIN_FORM_BAD_HOST) // Wrong host for origin form
//                                               // (400)
//{
//     std::string request = "GET / HTTP/1.1\r\nHost: tonpere\r\nUser-Agent: "
//                           "curl/7.68.0\r\nAccept: */*\r\n\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 400);
// }
// TEST_F(RequestTest, GET_ORIGIN_FORM_BAD_FILE) // File not found (404)
//{
//     std::string request =
//         "GET /tonpere.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 404);
// }
//
// TEST_F(RequestTest, GET_BAD_METHOD) // Bad Method (405)
//{
//     std::string request = "TRUC / HTTP/1.1\r\nHost: localhost\r\n\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 405);
// }
//
// TEST_F(RequestTest, GET_BAD_VERSION) // Bad Version (505)
//{
//     std::string request = "GET / HTTP/1.0\r\nHost: localhost\r\n\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 426);
// }
//
// TEST_F(RequestTest, GET_NO_HOST) // No host, Bad request (400)
//{
//     std::string request = "GET / HTTP/1.1\r\nNoHost: localhost\r\n\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 400);
// }
//
// TEST_F(
//     RequestTest,
//     GET_NO_CRLF_BETWEEN_HEADERS) // No CRLF between Headers, Bad request
//     (400)
//{
//     std::string request = "GET / HTTP/1.1\r\nHost: localhost\r\nAccept: */* "
//                           "User-Agent curl/7.68.0\r\n\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 400);
// }
//
// TEST_F(RequestTest,
//        GET_SPACE_AFTER_FIELD) // Space after field, Bad request (400)
//{
//     std::string request =
//         "GET / HTTP/1.1\r\nHost : localhost \r\nAccept: */*\r\n\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 400);
// }
//
// TEST_F(RequestTest, GET_NO_EMPTY_LINE) // No CRLF at the end, Bad request
// (400)
//{
//     std::string request = "GET / HTTP/1.1\r\nHost: localhost\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 400);
// }
//
// TEST_F(RequestTest, DOUBLE_HOST) // Double host, Bad request (400)
//{
//     std::string request =
//         "GET / HTTP/1.1\r\nHost: localhost\r\nHost: localhost\r\n\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 400);
// }
//
// TEST_F(RequestTest, DOUBLE_HEADER) // Double header, Bad request (400)
//{
//     std::string request =
//         "GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: "
//         "10\r\nContent-Length: 9\r\n\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 400);
// }
//
// TEST_F(RequestTest, HEADER_SIMPLE_WORD) // Simple Word Header, Bad request
// (400)
//{
//     std::string request = "GET / HTTP/1.1\r\nHost\r\n\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 400);
// }
//
// TEST_F(RequestTest, HOST_GOOD_IP) // Simple Word Header, Bad request (400)
//{
//     std::string request = "GET / HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 200);
// }
//
// TEST_F(RequestTest,
//        HOST_GOOD_IP_GOOD_PORT) // Simple Word Header, Bad request (400)
//{
//     std::string request = "GET / HTTP/1.1\r\nHost: 127.0.0.1:8000\r\n\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 200);
// }
// TEST_F(RequestTest,
//        HOST_GOOD_SERVERNAME_GOOD_PORT) // Simple Word Header, Bad request
//        (400)
//{
//     std::string request = "GET / HTTP/1.1\r\nHost: localhost:8000\r\n\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 200);
// }
//
// TEST_F(RequestTest,
//        HOST_BAD_SERVERNAME_GOOD_PORT) // Simple Word Header, Bad request
//        (400)
//{
//     std::string request = "GET / HTTP/1.1\r\nHost: localhost2:8000\r\n\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 400);
// }
//
// TEST_F(RequestTest,
//        HOST_BAD_IP_GOOD_PORT) // Bad host good port, Bad request (400)
//{
//     std::string request = "GET / HTTP/1.1\r\nHost: 127.0.0.3:8000\r\n\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 400);
// }
//
// TEST_F(RequestTest,
//        HOST_TRICKY_TEST) // Tricky test added one more colon, Bad request
//        (400)
//{
//     std::string request =
//         "GET / HTTP/1.1\r\nHost: localhost:8000:random\r\n\r\n";
//     auto test = request::Request(request, vhost_);
//     EXPECT_EQ(test.getStatusCode(), 400);
// }
//
// int main(int argc, char** argv)
//{
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }