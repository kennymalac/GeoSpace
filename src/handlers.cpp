#include <string>

#include "Poco/Redis/Array.h"
#include "include/handlers.hpp"

namespace GeoSpaceServer {
namespace JSON = Poco::JSON;
namespace Redis = Poco::Redis;

auto errorResponse(std::string text, std::string errorCode, HTTPResponse::HTTPStatus statusCode, HTTPServerResponse& response) {
  response.setStatusAndReason(statusCode);
  auto& output = response.send();

  auto errorData = new JSON::Object;
  errorData->set("message", text);
  errorData->set("error_code", errorCode);
  errorData->stringify(output);
  return errorData;
}

void GeoLocationHandler::operator()(HTTPServerRequest& request,
                                    HTTPServerResponse& response) {
  try {
    response.setContentType("text/json");
    finishResponse(request, response);
  }
  catch (Poco::InvalidAccessException e) {
    errorResponse(e.displayText(), "MISSING_FIELDS", HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST, response);
  }
  catch (Poco::NotImplementedException e) {
    errorResponse(e.displayText(), "INVALID_FIELDS", HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST, response);
  }
  catch (Poco::RangeException e) {
    errorResponse(e.displayText(), "INVALID_FIELDS", HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST, response);
  }
  catch (Poco::JSON::JSONException e) {
    errorResponse(e.displayText(), "INVALID_JSON", HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST, response);
  }
  catch (Poco::Redis::RedisException e) {
    std::cout << e.displayText() << "\n";
    errorResponse(e.displayText(), "REDIS", HTTPResponse::HTTPStatus::HTTP_INTERNAL_SERVER_ERROR, response);
  }
}


// This is an ugly workaround because C++17 doesn't have templated lambdas
// template <typename Handler, class ... Types>
// auto contextRequestHandler(Types ... args) {
// }

auto getLocation(JSON::Object::Ptr data) {
  double longitude;
  data->get("longitude").convert(longitude);
  double latitude;
  data->get("latitude").convert(latitude);

  return std::make_tuple(longitude, latitude);
}

GeoLocationCreateHandler::GeoLocationCreateHandler(RedisConnectionPool& rc)
  : GeoLocationHandler(rc)
{}

void GeoLocationCreateHandler::finishResponse(HTTPServerRequest& request,
                                              HTTPServerResponse& response) {
  /**
     {
     "user_id": 1,
     "place_id": 1,
     "latitude": 10.0,
     "longitude": 10.0
     }
  */
  // std::string body(std::istreambuf_iterator<char>(request.stream()), {});
  // std::cout << body;
  JSON::Object::Ptr data = parser.parse(request.stream()).extract<JSON::Object::Ptr>();

  auto [longitude, latitude] = getLocation(data);
  int userId;
  data->get("user_id").convert(userId);
  int placeId;
  data->get("place_id").convert(placeId);

  Redis::Array command;
  command << "GEOADD" << "USER-" + std::to_string(userId) <<
    std::to_string(longitude) <<
    std::to_string(latitude) <<
    "PLACE-" + std::to_string(placeId) + "";

  std::cout << command.toString() << "\n";

  auto placesAddedAmount = redisClient->execute<signed long>(command);
  std::cout << std::to_string(placesAddedAmount) << "\n";

  Redis::Array getCommand;
  getCommand << "GEOPOS" << "USER-" + std::to_string(userId) << "PLACE-" + std::to_string(placeId);
  std::cout << "result: " << redisClient->execute<Redis::Array>(getCommand).toString() << "\n";

  response.setStatus(HTTPResponse::HTTPStatus::HTTP_CREATED);
  response.send();
}

GeoLocationDeleteHandler::GeoLocationDeleteHandler(RedisConnectionPool& rc)
  : GeoLocationHandler(rc)
{}

void GeoLocationDeleteHandler::finishResponse(HTTPServerRequest& request,
                                              HTTPServerResponse& response) {
  /**
     {
     "place_id": "1"
     }
  */
  JSON::Object::Ptr data = parser.parse(request.stream()).extract<JSON::Object::Ptr>();

  auto placeId = data->get("place_id");


  // delete this place
  // TODO
}


GeoLocationDistanceHandler::GeoLocationDistanceHandler(RedisConnectionPool& rc)
  : GeoLocationHandler(rc)
{}

void GeoLocationDistanceHandler::finishResponse(HTTPServerRequest& request,
                                              HTTPServerResponse& response) {
  /**
     INPUT:
     {
     "place_id": 1,
     "other_place_id": 2,
     "unit": "km"
     }
     OUTPUT:
     {
     "distance": 10.12,
     "unit": "km"
     }
  */
  JSON::Object::Ptr data = parser.parse(request.stream()).extract<JSON::Object::Ptr>();

  auto [longitude, latitude] = getLocation(data);

  // Input validation

}
}
