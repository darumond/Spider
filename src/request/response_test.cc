// #include <filesystem>
// #include <gtest/gtest.h>
//
// #include "response.hh"
//
// TEST(Response_GET_200, StatusLine)
//{
//     auto response = request::Response(200, "GET", "/", "");
//     std::string status_line = response.status_line();
//     EXPECT_EQ(status_line, "HTTP/1.1 200 OK\r\n");
// }
//
// TEST(Response_GET_200, Date)
//{
//     auto response = request::Response(200, "GET", "/", "");
//     std::string date = response.date();
//     std::clog << date;
// }
//
// TEST(Response_GET_200, Content_Length)
//{
//     auto response = request::Response(
//         200, "GET", "../../../tests/response_file/content_length_5.txt", "");
//     std::string specific_header = response.specific_headers();
//     EXPECT_EQ(response.getContentLength().value, 5);
// }
//
// TEST(Response_PUT_200, Content_Length)
//{
//     auto response = request::Response(
//         200, "PUT", "../../../tests/response_file/content_length_5.txt",
//         "lucky");
//     std::string specific_header = response.specific_headers();
//     std::clog << response.response();
//     EXPECT_EQ(response.getContentLength().value, 0);
// }
//
// TEST(Response_PUT_201, Content_Length)
//{
//     auto response = request::Response(
//         201, "PUT", "../../../tests/response_file/content_length_5.txt",
//         "lucki");
//     std::string specific_header = response.specific_headers();
//     std::clog << response.response();
//     EXPECT_EQ(response.getContentLength().value, 0);
// }
//
// TEST(Response_DELETE_200, Content_Length)
//{
//     auto response = request::Response(
//         200, "DELETE", "../../../tests/response_file/no_file.txt", "");
//     std::string specific_header = response.specific_headers();
//     std::clog << response.response();
//     EXPECT_EQ(response.getContentLength().value, 0);
// }
//
// TEST(Response_GET_400, Bad_Request)
//{
//     auto response = request::Response(
//         400, "GET", "../../../tests/response_file/content_length_5.txt", "");
//     std::string status_line = response.status_line();
//     std::clog << response.response();
//     EXPECT_EQ(status_line, "HTTP/1.1 400 Bad Request\r\n");
// }
//
// TEST(Response_GET_404, Not_Found)
//{
//     auto response = request::Response(
//         404, "GET", "../../../tests/response_file/content_length_5.txt", "");
//     std::string status_line = response.status_line();
//     std::clog << response.response();
//     EXPECT_EQ(status_line, "HTTP/1.1 404 Not Found\r\n");
// }
//
// TEST(Response_PUT_405, Method_Not_Allowed)
//{
//     auto response = request::Response(
//         405, "PUT", "../../../tests/response_file/content_length_5.txt", "");
//     std::string status_line = response.status_line();
//     std::string specific_header = response.allow_header();
//     std::clog << response.response();
//     EXPECT_EQ(specific_header, "Allow: GET, HEAD\r\n");
// }
// int main(int argc, char** argv)
//{
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }
