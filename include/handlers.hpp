#include <memory>

#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Redis/Client.h"
#include "Poco/JSON/Parser.h"

namespace GeoSpaceServer {
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerResponse;

using RedisClient = Poco::Redis::Client;
using JSONParser = Poco::JSON::Parser;

auto errorResponse(std::string text, std::string errorCode, HTTPResponse::HTTPStatus statusCode, HTTPServerResponse& response, std::ostream& output);

class GeoLocationHandler
{
protected:
  std::shared_ptr<JSONParser> parser;
  std::shared_ptr<RedisClient> redisClient;

public:
  virtual void finishResponse(HTTPServerRequest& request,
                              HTTPServerResponse& response) = 0;

  virtual void operator()(HTTPServerRequest& request, HTTPServerResponse& response);

  inline GeoLocationHandler(std::shared_ptr<JSONParser> p, std::shared_ptr<RedisClient> rc)
    : parser(p), redisClient(rc)
  {}
  inline ~GeoLocationHandler() {};
};

class GeoLocationCreateHandler : public GeoLocationHandler
{
public:
  void finishResponse(HTTPServerRequest& request,
                      HTTPServerResponse& response);

  GeoLocationCreateHandler(std::shared_ptr<JSONParser> p, std::shared_ptr<RedisClient> rc);

  inline ~GeoLocationCreateHandler() {};
};

class GeoLocationDeleteHandler : public GeoLocationHandler
{
public:
  void finishResponse(HTTPServerRequest& request,
                      HTTPServerResponse& response);

  GeoLocationDeleteHandler(std::shared_ptr<JSONParser> p, std::shared_ptr<RedisClient> rc);

  inline ~GeoLocationDeleteHandler() {};
};

class GeoLocationDistanceHandler : public GeoLocationHandler
{
public:
  void finishResponse(HTTPServerRequest& request,
                      HTTPServerResponse& response);

  GeoLocationDistanceHandler(std::shared_ptr<JSONParser> p, std::shared_ptr<RedisClient> rc);

  inline ~GeoLocationDistanceHandler() {};
};

}
