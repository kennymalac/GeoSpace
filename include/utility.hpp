#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerRequest.h"

namespace GeoSpaceServer {
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;

template <typename Handler, class... Types>
auto inline contextRequestHandler(Types... args) {
  return [&args...](HTTPServerRequest& request,
                    HTTPServerResponse& response) {
           auto const handler = new Handler(std::forward<Types>(args)...);
           (*handler)(request, response);
           delete handler;
         };
  };
}
