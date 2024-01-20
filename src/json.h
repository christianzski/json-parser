#include <iostream>

#include <vector>
#include <unordered_map>

#include <filesystem>

namespace json {
  enum class value_type {
    object, array, integer, floating, string,
    true_literal, false_literal, null_literal,
    undefined
  };

  #ifndef NO_EXCEPTIONS
  class exception : public std::runtime_error {
    public:
      explicit exception(const std::string& message): std::runtime_error(message) {}
  };
  #endif

  class pair;
  class value {
    json::value_type type;
    std::string text;

    std::unordered_map<std::string, json::value> dict;
    std::vector<json::value> array;

    public:
      value();

      explicit value(json::value_type type);
      explicit value(std::unordered_map<std::string, json::value> values);
      explicit value(std::vector<json::value> values);
      value(bool);
      value(int);
      value(double);
      value(const char*);
      value(std::string str);

      value(std::initializer_list<json::pair>);

      value(json::value_type type, std::string text);

      std::vector<std::string> keys();

      bool is_object() const;
      bool is_array() const;
      bool is_bool() const;
      bool is_string() const;
      bool is_number() const;
      bool is_integer() const;
      bool is_float() const;
      bool is_null() const;

      size_t size() const;

      std::string to_string() const;

      value operator[](const std::string&) const;
      value operator[](size_t) const;

      explicit operator std::string() const;
      explicit operator int() const;
      explicit operator bool() const;
      explicit operator double() const;

      #ifdef NO_EXCEPTIONS
      bool error() const;
      #endif

      template<typename T>
      std::vector<T> to_vector() const {
        std::vector<T> result;

        if(is_array()) {
          for(size_t i = 0; i < size(); ++i) {
            result.push_back((T)array[i]);
          }
        }

        return result;
      }
  };

  class pair {
    public:
      std::string key;
      json::value value;
      pair(const std::string& key, const json::value& value):
      key(key), value(value) {}
  };

  json::value array(std::vector<json::value>);

  #ifdef NO_EXCEPTIONS
  json::value error(const std::string& msg);
  #endif

  std::ostream& operator<<(std::ostream& stream, const json::value& value);
  bool operator==(const json::value& value, int num);
  bool operator==(const json::value& value, double num);
  bool operator==(const json::value& value, const char* str);
  bool operator==(const json::value& value, std::string str);

  [[nodiscard]] json::value parse(std::string_view text);
  [[nodiscard]] json::value load(const std::filesystem::path& filename);
}
