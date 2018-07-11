#include <memory>
#include <optional>

#include "Poco/ObjectPool.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Redis/Client.h"
#include "Poco/Redis/PoolableConnectionFactory.h"
#include "Poco/JSON/Parser.h"

namespace GeoSpaceServer {
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerResponse;
namespace Redis = Poco::Redis;

using JSONParser = Poco::JSON::Parser;
using RedisConnectionPool = Poco::ObjectPool<Redis::Client, Redis::Client::Ptr>;


auto errorResponse(std::string text, std::string errorCode, HTTPResponse::HTTPStatus statusCode, HTTPServerResponse& response, std::ostream& output);

class GeoLocationHandler
{
protected:
  JSONParser parser;
  Redis::Client::Ptr redisClient;

public:
  virtual void finishResponse(HTTPServerRequest& request,
                              HTTPServerResponse& response) = 0;

  virtual void operator()(HTTPServerRequest& request, HTTPServerResponse& response);

  inline GeoLocationHandler(RedisConnectionPool& rc)
  {
    redisClient = Redis::PooledConnection(rc);
  }
  inline ~GeoLocationHandler() {};
};

class GeoLocationCreateHandler : public GeoLocationHandler
{
public:
  void finishResponse(HTTPServerRequest& request,
                      HTTPServerResponse& response);

  GeoLocationCreateHandler(RedisConnectionPool& rc);

  inline ~GeoLocationCreateHandler() {};
};

class GeoLocationDeleteHandler : public GeoLocationHandler
{
public:
  void finishResponse(HTTPServerRequest& request,
                      HTTPServerResponse& response);

  GeoLocationDeleteHandler(RedisConnectionPool& rc);

  inline ~GeoLocationDeleteHandler() {};
};

class GeoLocationDistanceHandler : public GeoLocationHandler
{
public:
  void finishResponse(HTTPServerRequest& request,
                      HTTPServerResponse& response);

  GeoLocationDistanceHandler(RedisConnectionPool& rc);

  inline ~GeoLocationDistanceHandler() {};
};

}
