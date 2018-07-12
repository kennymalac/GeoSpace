#include <string>

#include "Poco/Redis/Array.h"
#include "include/handlers.hpp"

namespace GeoSpaceServer {
namespace JSON = Poco::JSON;
namespace Redis = Poco::Redis;

std::string GeoLocationHandler::redisKey = "West-Hyperborea";

auto errorResponse(std::string text, std::string errorCode, HTTPResponse::HTTPStatus statusCode, HTTPServerResponse& response) {
  response.setStatusAndReason(statusCode);
  auto& output = response.send();

  JSON::Object::Ptr errorData = new JSON::Object;
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
    std::cout << e.displayText() << "\n";
    errorResponse(e.displayText(), "MISSING_FIELDS", HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST, response);
  }
  catch (Poco::NotImplementedException e) {
    std::cout << e.displayText() << "\n";
    errorResponse(e.displayText(), "INVALID_FIELDS", HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST, response);
  }
  catch (Poco::RangeException e) {
    std::cout << e.displayText() << "\n";
    errorResponse(e.displayText(), "INVALID_FIELDS", HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST, response);
  }
  catch (Poco::JSON::JSONException e) {
    std::cout << e.displayText() << "\n";
    errorResponse(e.displayText(), "INVALID_JSON", HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST, response);
  }
  catch (Poco::Redis::RedisException e) {
    std::cout << e.displayText() << "\n";
    errorResponse(e.displayText(), "REDIS", HTTPResponse::HTTPStatus::HTTP_INTERNAL_SERVER_ERROR, response);
  }
  catch (const std::exception &e) {
    std::cout << e.what();
    errorResponse(e.what(), "UNKNOWN_ERROR", HTTPResponse::HTTPStatus::HTTP_INTERNAL_SERVER_ERROR, response);
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
     "place_id": 1,
     "latitude": 10.0,
     "longitude": 10.0
     }
  */
  // std::string body(std::istreambuf_iterator<char>(request.stream()), {});
  // std::cout << body;
  JSON::Object::Ptr data = parser.parse(request.stream()).extract<JSON::Object::Ptr>();

  auto [longitude, latitude] = getLocation(data);
  int placeId;
  data->get("place_id").convert(placeId);

  Redis::Array command;
  command << "GEOADD" << GeoLocationHandler::redisKey <<
    std::to_string(longitude) <<
    std::to_string(latitude) <<
    std::to_string(placeId);

  std::cout << command.toString() << "\n";

  auto placesAddedAmount = redisClient->execute<signed long>(command);
  std::cout << std::to_string(placesAddedAmount) << "\n";

  Redis::Array getCommand;
  getCommand << "GEOPOS" << GeoLocationHandler::redisKey << std::to_string(placeId);
  std::cout << "result: " << redisClient->execute<Redis::Array>(getCommand).toString() << "\n";

  response.setStatusAndReason(HTTPResponse::HTTPStatus::HTTP_CREATED);
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

  int placeId;
  data->get("place_id").convert(placeId);

  // delete this place
  Redis::Array deleteCommand;
  deleteCommand << "ZREM" << GeoLocationHandler::redisKey << std::to_string(placeId);
  redisClient->execute<signed long>(deleteCommand);

  response.setStatusAndReason(HTTPResponse::HTTPStatus::HTTP_OK);
  response.send();
}


GeoLocationDistanceRadiusHandler::GeoLocationDistanceRadiusHandler(RedisConnectionPool& rc)
  : GeoLocationHandler(rc)
{}

void GeoLocationDistanceRadiusHandler::finishResponse(HTTPServerRequest& request,
                                                      HTTPServerResponse& response) {
  /**
     INPUT:
     {
     "place_id": 1,
     "distance": 10.0,
     "unit": "km"
     }
     OUTPUT:
     {
     results: [2]
     }
  */
  JSON::Object::Ptr data = parser.parse(request.stream()).extract<JSON::Object::Ptr>();

  double distance;
  data->get("distance").convert(distance);
  std::string unit;
  data->get("unit").convert(unit);
  int placeId;
  data->get("place_id").convert(placeId);

  Redis::Array radiusCommand;
  radiusCommand << "GEORADIUSBYMEMBER" << GeoLocationHandler::redisKey << std::to_string(placeId) << std::to_string(distance) << unit;
  auto results = redisClient->execute<Redis::Array>(radiusCommand);

  response.setStatusAndReason(HTTPResponse::HTTPStatus::HTTP_OK);

  JSON::Array::Ptr serialized = new JSON::Array;
  JSON::Object::Ptr resp = new JSON::Object;

  // std::cout << results.get<Redis::BulkString>(0) << "\n";

  for (size_t i=0; i<results.size()-1; i++) {
    auto value = results.get<Redis::BulkString>(i);
    if (!value.isNull()) {
      serialized->add(std::stoi(value.value()));
    }
  }

  resp->set("results", serialized);

  auto& output = response.send();
  resp->stringify(output);
}
}
