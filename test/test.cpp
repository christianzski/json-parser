#include <catch2/catch_test_macros.hpp>
#include <random>
#include <json.h>

TEST_CASE("RFC 8259 example 1", "[rfc8259]") {
	auto json = json::load("./files/rfc13-1.json");
	REQUIRE(json.is_object());
	REQUIRE(json["Image"].is_object());
	REQUIRE(json["Image"]["Width"] == 800);
	REQUIRE(json["Image"]["Height"] == 600);
	REQUIRE(json["Image"]["Title"] == "View from 15th Floor");
	REQUIRE(json["Image"]["Thumbnail"].is_object());
	REQUIRE(json["Image"]["Thumbnail"]["Url"] == "http://www.example.com/image/481989943");
	REQUIRE(json["Image"]["Thumbnail"]["Height"] == 125);
	REQUIRE(json["Image"]["Thumbnail"]["Width"] == 100);
	REQUIRE(json["Image"]["Animated"] == false);
	REQUIRE(json["Image"]["IDs"].is_array());
	REQUIRE(json["Image"]["IDs"][0] == 116);
	REQUIRE(json["Image"]["IDs"][1] == 943);
	REQUIRE(json["Image"]["IDs"][2] == 234);
	REQUIRE(json["Image"]["IDs"][3] == 38793);
}

TEST_CASE("RFC 8259 example 2", "[rfc8259]") {
	auto json = json::load("./files/rfc13-2.json");
	REQUIRE(json.is_array());
	REQUIRE(json[0].is_object());
	REQUIRE(json[0]["precision"] == "zip");
	REQUIRE(json[0]["Latitude"] == 37.7668);
	REQUIRE(json[0]["Longitude"] == -122.3959);
	REQUIRE(json[0]["Address"] == "");
	REQUIRE(json[0]["City"] == "SAN FRANCISCO");
	REQUIRE(json[0]["State"] == "CA");
	REQUIRE(json[0]["Zip"] == "94107");
	REQUIRE(json[0]["Country"] == "US");

	REQUIRE(json[1].is_object());
	REQUIRE(json[1]["precision"] == "zip");
	REQUIRE(json[1]["Latitude"] == 37.371991);
	REQUIRE(json[1]["Longitude"] == -122.026020);
	REQUIRE(json[1]["Address"] == "");
	REQUIRE(json[1]["City"] == "SUNNYVALE");
	REQUIRE(json[1]["State"] == "CA");
	REQUIRE(json[1]["Zip"] == "94085");
	REQUIRE(json[1]["Country"] == "US");
}

TEST_CASE("RFC 8259 value examples", "[rfc8259]") {
	REQUIRE(json::parse("\"Hello world!\"") == "Hello world!");
	REQUIRE(json::parse("42") == 42);
	REQUIRE(json::parse("true") == true);
}

TEST_CASE("UTF-8 parsing", "[strings]") {
	REQUIRE(json::parse("\"\\u0021\\u00A3\\u0418\\u07FF\\u1E55\\uFFFC\"") == "!£И߿ṕ￼");
}

TEST_CASE("Vectorize homogeneous arrays", "[numbers]") {
	auto json = json::parse("[ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ]");
	auto vector = json.to_vector<int>();
	int sum = std::accumulate(vector.begin(), vector.end(), 0);
	REQUIRE(sum == 55);
	REQUIRE((double)sum / vector.size() == 5.5);
}

TEST_CASE("Pi test", "[numbers]") {
	auto json = json::parse(R"(["3", ".", "1", "4", "1", "5", "9", "2", "6", "5", "3", "5"])");
	std::vector<std::string> digits = json.to_vector<std::string>();
	std::string pi = std::accumulate(digits.begin(), digits.end(), std::string{});

	REQUIRE(fabs(std::stod(pi) - M_PI) < 0.001);
}

TEST_CASE("Stringify", "[stringify]") {
	json::value json = json::array({ 1, 2, 3 });
	REQUIRE(json.to_string() == "[1, 2, 3]");

	json::value obj = { { "name", "bob" }, { "level", 42 } };
	REQUIRE((obj.to_string() == "{ \"name\": \"bob\", \"level\": 42 }" ||
			 obj.to_string() == "{ \"level\": 42, \"name\": \"bob\" }"));
}

TEST_CASE("Round-trip conversion", "[stringify]") {
	json::value json = {
 	  {"Image", {
		  { "Width", 800 },
		  { "Height", 600 },
		  { "Title", "View from 15th Floor" },
		  { "Thumbnail", {
			  { "Url", "http://www.example.com/image/481989943" },
			  { "Height", 125 },
			  { "Width", 100 },
		  }},
		  {"Animated", false},
		  {"IDs", json::array({116, 943, 234, 38793})}
	  }}
    };

	REQUIRE(json.is_object());
	REQUIRE(json["Image"].is_object());
	REQUIRE(json["Image"]["Width"] == 800);
	REQUIRE(json["Image"]["Height"] == 600);
	REQUIRE(json["Image"]["Title"] == "View from 15th Floor");
	REQUIRE(json["Image"]["Thumbnail"].is_object());
	REQUIRE(json["Image"]["Thumbnail"]["Url"] == "http://www.example.com/image/481989943");
	REQUIRE(json["Image"]["Thumbnail"]["Height"] == 125);
	REQUIRE(json["Image"]["Thumbnail"]["Width"] == 100);
	REQUIRE(json["Image"]["Animated"] == false);
	REQUIRE(json["Image"]["IDs"].is_array());
	REQUIRE(json["Image"]["IDs"][0] == 116);
	REQUIRE(json["Image"]["IDs"][1] == 943);
	REQUIRE(json["Image"]["IDs"][2] == 234);
	REQUIRE(json["Image"]["IDs"][3] == 38793);

	auto round_trip = json::parse(json.to_string());

	REQUIRE(round_trip.is_object());
	REQUIRE(round_trip["Image"].is_object());
	REQUIRE(round_trip["Image"]["Width"] == 800);
	REQUIRE(round_trip["Image"]["Height"] == 600);
	REQUIRE(round_trip["Image"]["Title"] == "View from 15th Floor");
	REQUIRE(round_trip["Image"]["Thumbnail"].is_object());
	REQUIRE(round_trip["Image"]["Thumbnail"]["Url"] == "http://www.example.com/image/481989943");
	REQUIRE(round_trip["Image"]["Thumbnail"]["Height"] == 125);
	REQUIRE(round_trip["Image"]["Thumbnail"]["Width"] == 100);
	REQUIRE(round_trip["Image"]["Animated"] == false);
	REQUIRE(round_trip["Image"]["IDs"].is_array());
	REQUIRE(round_trip["Image"]["IDs"][0] == 116);
	REQUIRE(round_trip["Image"]["IDs"][1] == 943);
	REQUIRE(round_trip["Image"]["IDs"][2] == 234);
	REQUIRE(round_trip["Image"]["IDs"][3] == 38793);
}

TEST_CASE("Error handling", "[errors]") {
	#ifndef NO_EXCEPTIONS
	try {
		auto json = json::parse("[]");
		json = json::parse("[1, 2, 3, 4, 5]");
		json = json::parse("[qwerty]");
		REQUIRE(false);
	} catch(const json::exception& e) {
		REQUIRE(true);
	}
	#else
	REQUIRE(json::parse("[qwerty]").error());
	REQUIRE(!json::parse("[]").error());
	REQUIRE(!json::parse("[1, 2, 3, 4, 5]").error());
	#endif
}

TEST_CASE("Checking types", "[types]") {
	auto json = json::parse(R"({
      "int": 42,
      "float": 42.42,
      "string": "Hello!",
      "array": [ 1, 2, 3, 4, 5 ],
      "object": { "key": "value" },
      "true": true,
      "false": false,
      "null": null,
    })");

	// Checking data types
	REQUIRE(json["int"].is_integer());
	REQUIRE(json["float"].is_float());
	REQUIRE(json["string"].is_string());
	REQUIRE(json["array"].is_array());
	REQUIRE(json["object"].is_object());
	REQUIRE(json["true"].is_bool());
	REQUIRE(json["false"].is_bool());
	REQUIRE(json["null"].is_null());

	REQUIRE(json["int"] == 42);
	REQUIRE(json["float"] == 42.42);
	REQUIRE(json["string"] == "Hello!");
	REQUIRE((json["array"].size() == 5 && json["array"][0] == 1));
	REQUIRE(json["object"]["key"] == "value");
	REQUIRE(json["true"] == true);
	REQUIRE(json["false"] == false);
}
