#include <iostream>

#include "Poco/Util/Application.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/ObjectPool.h"

#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/SecureServerSocket.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Redis/Client.h"
#include "Poco/Redis/PoolableConnectionFactory.h"
#include "Poco/JSON/Parser.h"
#include "Poco/StreamCopier.h"
#include "Poco/SharedPtr.h"

// TODO figure this out
#include "PocoHttpRouterProject/include/HttpRouter.h"
#include "utility.hpp"
#include "handlers.hpp"

namespace GeoSpaceServer {
using Poco::Net::HTTPServer;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerParams;
using Poco::Net::ServerSocket;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::ThreadPool;
using Poco::Dynamic::Var;

namespace Redis = Poco::Redis;
using JSONParser = Poco::JSON::Parser;
using RedisConnectionPool = Poco::ObjectPool<Redis::Client, Redis::Client::Ptr>;


class MethodNotAllowedHandler : public Poco::Net::HTTPRequestHandler {
  void handleRequest(Poco::Net::HTTPServerRequest& request,
                     Poco::Net::HTTPServerResponse& response) {
    response.setStatus(HTTPResponse::HTTPStatus::HTTP_METHOD_NOT_ALLOWED);
    return;
  }
};

class GeospaceServer : public Poco::Util::ServerApplication {

protected:
  void initialize(Poco::Util::Application& self)
  {
    // load default configuration files, if present
    loadConfiguration();
    ServerApplication::initialize(self);
  }

  int main(const std::vector<std::string>& args) {
    auto port = static_cast<unsigned short>(config().getInt("GeospaceServer.port", 9980));
    std::string hostname = config().getString("GeospaceServer.redis.hostname", "127.0.0.1");

    auto redisPort = static_cast<unsigned short>(config().getInt("GeospaceServer.redis.port", 6379));

    // 

    // set-up a server socket
    ServerSocket service(port);
    // auto parser = std::make_shared<JSONParser>();
    // hostname, redisPort
    Poco::Net::SocketAddress redisAddress(hostname, redisPort);

    RedisConnectionPool redisConnectionPool(Poco::PoolableObjectFactory<Redis::Client, Redis::Client::Ptr>(redisAddress), 4, 4);

    auto* router = new HttpRouter();

    auto locationCreateHandler = contextRequestHandler<GeoLocationCreateHandler, RedisConnectionPool&>(redisConnectionPool);
    auto locationDeleteHandler = contextRequestHandler<GeoLocationDeleteHandler, RedisConnectionPool&>(redisConnectionPool);
    auto locationDistanceRadiusHandler = contextRequestHandler<GeoLocationDistanceRadiusHandler, RedisConnectionPool&>(redisConnectionPool);

    //router->AddRoute("/location/", list, "GET");
    router->AddRoute("/location/", locationCreateHandler, "PUT");
    router->AddRoute("/location/", locationDeleteHandler, "DELETE");
    router->AddRoute("/location/distance-radius", locationDistanceRadiusHandler, "POST");

    // TODO params
    HTTPServerParams* serverParams = new HTTPServerParams;
    // pParams->setMaxQueued(maxQueued);
    // pParams->setMaxThreads(maxThreads);

    // set-up a HTTPServer instance
    HTTPServer server(router, service, serverParams);

    // start the HTTPServer
    server.start();
    // wait for CTRL-C or kill
    waitForTerminationRequest();
    // Stop the HTTPServer
    server.stop();

    return Application::EXIT_OK;
  }
};
}

int main(int argc, char** argv) {
  GeoSpaceServer::GeospaceServer app;
  return app.run(argc, argv);
}

