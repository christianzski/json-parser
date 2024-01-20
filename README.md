# json-parser
A WIP C++ JSON parser/processor intended to be compliant with ECMA 404 2nd edition and RFC 8259. This JSON processor is built with a priority of readability and usability over performance.

# Building
## Requirements
- C++20 compliant compiler (GCC/Clang)
- CMake (version 3.x)

## Compilation

Define `NO_EXCEPTIONS` on build configuration if you do not wish to have exceptions thrown. By default, errors during parsing will throw exceptions of the form `json::exception`. If `NO_EXCEPTIONS` is defined, an error flag will be set instead.

## Building without exceptions
``` bash
mkdir build
cd build
cmake -DNO_EXCEPTIONS=1 ..

make
```

## Building with exceptions

``` bash
mkdir build
cd build
cmake ..

make
```

# Usage
When NO_EXCEPTIONS is defined, the JSON processor will silently fail on parsing errors, and will set an error flag. If you wish for exceptions to be thrown, you can enable them by defining `NO_EXCEPTIONS` in the preprocessor:

## Parsing example (w/ exceptions)

```cpp
#include <json.h>

int main() {
  try {
    // Exception will be thrown; single quotes are not allowed by the standard
    json::value body = json::parse("{'name': 'value'}");
    std::cout << body["name"] << "\n";
  } catch(json::exception& e) {
    std::cerr << "Failed to parse JSON!\n";
  }
}
```

## Parsing example (without exceptions)

```cpp
#include <json.h>

int main() {
  json::value body = json::parse("[ 1, 2, 3 ]");
  if(!body.error()) {
    // Conversion to vector is valid for homogeneous arrays
    auto vector = json::to_vector<int>(body);
    int sum = std::accumulate(body.begin(), body.end(), 0);
  } else {
    std::cout << body << "\n";
  }
}
```

## Parsing files
A load function is included to load and parse a local JSON file in one step: 
```cpp
#include <json.h>

int main() {
  json::value body = json::load("./document.json");
  if(body.is_object()) {
    for(const auto& [key, val] : body) {
      std::cout << key << " = " << val << "\n";
    }
  }
}
```

## Array access and vectorization
Accessing an array is straightforward with the subscript operator. If you have a homogeneous JSON array, you can also vectorize the data in one step:

``` cpp
#include <json.h>

int main() {
  auto json = json::parse("[ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ]");
  
  // You can index the JSON value directly
  assert(json[0] == 1);
  
  // Alternatively, you can convert to a vector and use the STL as needed
  auto vector = json.to_vector<int>();
  int sum = std::accumulate(vector.begin(), vector.end(), 0);
  assert(sum == 55);
}
```

## Constructing JSON
Creating JSON and converting it to text is straightforward. Due to language ambiguities, a JSON array must be directly constructed with `json::array`:

``` cpp
#include <json.h>

int main() {
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
  
  assert(json["Image"].is_object());
  assert(json["Image"]["IDs"][0] == 116);
  std::string text = json.to_string();
}
```

# References
* https://ecma-international.org/publications-and-standards/standards/ecma-404/
    - ECMA-404 - The JSON data interchange syntax
    
* https://datatracker.ietf.org/doc/html/rfc8259
    - RFC 8259 - The JavaScript Object Notation (JSON) Data Interchange Format
