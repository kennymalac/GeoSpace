#include "include/handlers.hpp"

namespace GeoSpaceServer {
namespace JSON = Poco::JSON;

auto errorResponse(std::string text, std::string errorCode, HTTPResponse::HTTPStatus statusCode, HTTPServerResponse& response, std::ostream& output) {
  response.setStatus(statusCode);
  auto errorData = new JSON::Object;
  errorData->set("message", text);
  errorData->set("error_code", errorCode);
  errorData->stringify(output);
  return errorData;
}

void GeoLocationHandler::operator()(HTTPServerRequest& request,
                                    HTTPServerResponse& response) {
  auto& output = response.send();

  try {
    finishResponse(request, response);
    response.setContentType("text/json");
  }
  catch (Poco::InvalidAccessException e) {
    errorResponse(e.displayText(), "MISSING_FIELDS", HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST, response, output);
  }
  catch (Poco::NotImplementedException e) {
    errorResponse(e.displayText(), "INVALID_FIELDS", HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST, response, output);
  }
  catch (Poco::RangeException e) {
    errorResponse(e.displayText(), "INVALID_FIELDS", HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST, response, output);
  }
  catch (Poco::JSON::JSONException e) {
    errorResponse(e.displayText(), "INVALID_JSON", HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST, response, output);
  }
  catch (Poco::Redis::RedisException e) {
    std::cout << e.displayText() << "\n";
    errorResponse(e.displayText(), "REDIS", HTTPResponse::HTTPStatus::HTTP_INTERNAL_SERVER_ERROR, response, output);
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

GeoLocationCreateHandler::GeoLocationCreateHandler(std::shared_ptr<JSONParser> p, std::shared_ptr<RedisClient> rc)
  : GeoLocationHandler(p, rc)
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
  JSON::Object::Ptr data = parser->parse(request.stream()).extract<JSON::Object::Ptr>();

  auto [longitude, latitude] = getLocation(data);
}

GeoLocationDeleteHandler::GeoLocationDeleteHandler(std::shared_ptr<JSONParser> p, std::shared_ptr<RedisClient> rc)
  : GeoLocationHandler(p, rc)
{}

void GeoLocationDeleteHandler::finishResponse(HTTPServerRequest& request,
                                              HTTPServerResponse& response) {
  /**
     {
     "place_id": "1"
     }
  */
  JSON::Object::Ptr data = parser->parse(request.stream()).extract<JSON::Object::Ptr>();

  auto placeId = data->get("place_id");

  // delete this place
  // TODO
}


GeoLocationDistanceHandler::GeoLocationDistanceHandler(std::shared_ptr<JSONParser> p, std::shared_ptr<RedisClient> rc)
  : GeoLocationHandler(p, rc)
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
  JSON::Object::Ptr data = parser->parse(request.stream()).extract<JSON::Object::Ptr>();

  auto [longitude, latitude] = getLocation(data);

  // Input validation

}
}
