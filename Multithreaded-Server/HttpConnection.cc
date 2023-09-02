/*
 * Copyright ©2023 Chris Thachuk.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Spring Quarter 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <stdint.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <string>
#include <vector>
#include <iostream>

#include "./HttpRequest.h"
#include "./HttpUtils.h"
#include "./HttpConnection.h"

#define BUFFSIZE 1024

using std::map;
using std::string;
using std::vector;

using boost::algorithm::split;
using boost::algorithm::trim;
using boost::algorithm::is_any_of;
using boost::algorithm::token_compress_on;
using boost::algorithm::to_lower;

namespace hw4 {

static const char* kHeaderEnd = "\r\n\r\n";
static const int kHeaderEndLen = 4;

bool HttpConnection::GetNextRequest(HttpRequest* const request) {
  // Use WrappedRead from HttpUtils.cc to read bytes from the files into
  // private buffer_ variable. Keep reading until:
  // 1. The connection drops
  // 2. You see a "\r\n\r\n" indicating the end of the request header.
  //
  // Hint: Try and read in a large amount of bytes each time you call
  // WrappedRead.
  //
  // After reading complete request header, use ParseRequest() to parse into
  // an HttpRequest and save to the output parameter request.
  //
  // Important note: Clients may send back-to-back requests on the same socket.
  // This means WrappedRead may also end up reading more than one request.
  // Make sure to save anything you read after "\r\n\r\n" in buffer_ for the
  // next time the caller invokes GetNextRequest()!

  // STEP 1:

  // Attempt to get the next user request which indicated when
  // we find thee kHeaderEnd sequence
  size_t header_end_pos = buffer_.find(kHeaderEnd);
  if (header_end_pos == string::npos) {
    char buf[BUFFSIZE];
    int read_byte;

    while (1) {
      read_byte = WrappedRead(fd_, reinterpret_cast<unsigned char*>(buf),
                                  BUFFSIZE);
      if (read_byte == 0) {
        break;
      }
      if (read_byte == -1) {
        return false;
      }
        buffer_ += string(buf, read_byte);
        header_end_pos = buffer_.find(kHeaderEnd);
        if (header_end_pos != string::npos) {
          break;
        }
    }
  }

  if (header_end_pos == string::npos) {
    return false;
  }

  // Parse the request and update buffe_ accordingly
  *request = ParseRequest(buffer_.substr(0, header_end_pos));
  buffer_ = buffer_.substr((header_end_pos + kHeaderEndLen));
  return true;
}

bool HttpConnection::WriteResponse(const HttpResponse& response) const {
  string str = response.GenerateResponseString();
  int res = WrappedWrite(fd_,
                         reinterpret_cast<const unsigned char*>(str.c_str()),
                         str.length());
  if (res != static_cast<int>(str.length()))
    return false;
  return true;
}

HttpRequest HttpConnection::ParseRequest(const string& request) const {
  HttpRequest req("/");  // by default, get "/".

  // Plan for STEP 2:
  // 1. Split the request into different lines (split on "\r\n").
  // 2. Extract the URI from the first line and store it in req.URI.
  // 3. For the rest of the lines in the request, track the header name and
  //    value and store them in req.headers_ (e.g. HttpRequest::AddHeader).
  //
  // Hint: Take a look at HttpRequest.h for details about the HTTP header
  // format that you need to parse.
  //
  // You'll probably want to look up boost functions for:
  // - Splitting a string into lines on a "\r\n" delimiter
  // - Trimming whitespace from the end of a string
  // - Converting a string to lowercase.
  //
  // Note: If a header is malformed, skip that line.

  // STEP 2:
  string copy(request);
  trim(copy);
  vector<string> lines;
  split(lines, copy, is_any_of("\r\n"), token_compress_on);
  vector<string> first_components;
  string first_line = lines[0];

  split(first_components, first_line, is_any_of(" "), token_compress_on);

  // set the uri now that we have split the first line into components
  req.set_uri(first_components[1]);

  // For the rest of the lines get the header and values associated
  for (size_t i = 1; i < lines.size(); i++) {
    vector<string> components;
    split(components, lines[i], is_any_of(": "), token_compress_on);
    to_lower(components[0]);
    req.AddHeader(components[0], components[1]);
  }
  return req;
}

}  // namespace hw4
