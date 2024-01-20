#include "json.h"
#include <format>
#include <fstream>

namespace json {
  class string_iterator {
    private:
      std::string_view text;
      size_t index;
    public:
      string_iterator(const std::string_view& text);

      bool available() const;
      char peek() const;
      char next();
  };

  json::value read_value(string_iterator& text);

  string_iterator::string_iterator(const std::string_view& text) :
    text(text), index(0) {}

  char string_iterator::peek() const {
    return text[index];
  }

  bool string_iterator::available() const {
    return index < text.size();
  }

  char string_iterator::next() {
    return available() ? text[index++] : text[index];
  }

  value::value() : type(json::value_type::undefined) {}

  value::value(json::value_type type) :
    type(type) {}

  value::value(json::value_type type, std::string text) :
    type(type), text(text) {}

  value::value(std::unordered_map<std::string, json::value> values) :
    type(json::value_type::object), dict(values) {}

  value::value(std::vector<json::value> values) :
    type(json::value_type::array), array(values) {}

  value::value(std::string str) :
    type(json::value_type::string), text(str) {}

  value::value(const char* str) : value(std::string{str}) {}

  value::value(bool value) :
    type(value ?
         json::value_type::true_literal :
         json::value_type::false_literal) {}

  value::value(int value) :
    type(json::value_type::integer), text(std::to_string(value)) {}

  value::value(double value) :
    type(json::value_type::floating), text(std::to_string(value)) {}

  std::vector<std::string> value::keys() {
    std::vector<std::string> result;

    if(is_object()) {
      for(auto& [key, value] : dict) {
        result.push_back(key);
      }
    }

    return result;
  }

  bool value::is_string() const {
    return type == json::value_type::string;
  }

  bool value::is_object() const {
    return type == json::value_type::object;
  }

  bool value::is_array() const {
    return type == json::value_type::array;
  }

  bool value::is_bool() const {
    return type == value_type::true_literal ||
      type == value_type::false_literal;
  }

  bool value::is_number() const {
    return type == json::value_type::integer ||
      type == json::value_type::floating;
  }

  bool value::is_integer() const {
    return type == json::value_type::integer;
  }

  bool value::is_float() const {
    return type == json::value_type::floating;
  }

  bool value::is_null() const {
    return type == json::value_type::null_literal;
  }

  size_t value::size() const {
    switch(type) {
      case json::value_type::array:
        return array.size();
      case json::value_type::string:
        return ((std::string)*this).size();
        break;
      default:
        return 0;
    }
  }

  std::string value::to_string() const {
    std::string result;

    using json::value_type;
    switch(type) {
      case value_type::object:
        result += "{ ";

        for(size_t i = 0; const auto& [key, val] : dict) {
          if(i++) result += ", ";
          result += std::format("\"{}\": {}", key, val.to_string());
        }

        result += " }";
        break;

      case value_type::array:
        result += "[";

        for(size_t i = 0; const auto& val : array) {
          result += ((i++) ? ", " : "") + val.to_string();
        }

        result += "]";
        break;

      case value_type::floating:
      case value_type::integer:
        result += text;
        break;

      case value_type::string:
        result += std::format("\"{}\"", text);
        break;

      case value_type::true_literal:
        result += "true";
        break;

      case value_type::false_literal:
        result += "false";
        break;

      case value_type::null_literal:
        result += "null";
        break;

      case value_type::undefined:
        result += text;

      default:
        break;
    }

    return result;
  }

  value::operator int() const {
    switch(type) {
      case value_type::integer:
      case value_type::floating:
        return std::stoi(to_string());
      case value_type::false_literal: return 0;
      case value_type::true_literal: return 1;
      default: break;
    }

    return 0;
  }

  value::operator double() const {
    switch(type) {
      case value_type::integer:
      case value_type::floating:
        return std::stod(to_string());
      default:
        return 0;
    }
  }

  value::operator std::string() const {
    if(type == json::value_type::string) return text;
    return to_string();
  }

  value::operator bool() const {
    using json::value_type;
    return !(type == value_type::false_literal || type == value_type::undefined);
  }

  #ifdef NO_EXCEPTIONS
  bool value::error() const {
    return type == value_type::undefined;
  }
  #endif

  value value::operator[](const std::string& str) const {
    auto index = dict.find(str);
    if(index != dict.end()) {
      return index->second;
    }

    return value();
  }

  value value::operator[](size_t i) const {
    if(i < array.size()) {
      return array[i];
    }

    const char* msg = "array index out of bounds";
    #ifndef NO_EXCEPTIONS
    throw json::exception(msg);
    #else
    return json::error(msg);
    #endif
  }

  std::ostream& operator<<(std::ostream& stream, const json::value& value) {
    return stream << value.to_string();
  }

  bool operator==(const json::value& value, double num) {
    if(value.is_number()) {
      return (double)value == num;
    }

    return false;
  }

  bool operator==(const json::value& value, int num) {
    if(value.is_number() || value.is_bool()) {
      return (int)value == num;
    }

    return false;
  }

  bool operator==(const json::value& value, const char* str) {
    if(value.is_string()) {
      return (std::string)value == str;
    }

    return false;
  }

  bool operator==(const json::value& value, std::string str) {
    if(value.is_string()) return (std::string)value == str;

    return false;
  }

  std::string read_unicode(string_iterator& text) {
    int value = 0, digits = 0;
    std::string code;

    while(digits < 4 && text.available()) {
      const char c = text.peek();
      switch(c) {
        case '0': case '1':
        case '2': case '3':
        case '4': case '5':
        case '6': case '7':
        case '8': case '9':
          digits++;
          value = (value << 4) | (c - '0');
          break;
        case 'a': case 'b':
        case 'c': case 'd':
        case 'e': case 'f':
          digits++;
          value = (value << 4) | (c - 'a' + 10);
          break;
        case 'A': case 'B':
        case 'C': case 'D':
        case 'E': case 'F':
          digits++;
          value = (value << 4) | (c - 'A' + 10);
          break;
        default: {
          #ifndef NO_EXCEPTIONS
          throw json::exception("parsing: invalid unicode character");
          #else
          return "";
          #endif
        } break;
      }

      if(digits < 4) text.next();
    }

    // Encode a code point into UTF-8 binary representation
    if(value >= 0x0000 && value <= 0x007F) {
      code += (char)value;
    } else if(value <= 0x07FF) {
      code += ((0b110 << 5) | ((value >> 6) & 0b11111));
      code += ((0b10 << 6) | ((value) & 0b111111));
    } else {
      code += ((0b1110 << 4) | ((value >> 12) & 0b1111));
      code += ((0b10 << 6) | ((value >> 6) & 0b111111));
      code += ((0b10 << 6) | (value & 0b111111));
    }

    return code;
  }

  json::value read_string(string_iterator& text) {
    text.next();

    std::string str;
    bool backslash = false, exit = false;

    while(text.available()) {
      const char c = text.peek();
      switch(c) {
        case '\\':
          if(!backslash) {
            backslash = true;
          }

          else str += '\\';
          break;
        case '"':
          if(!backslash) exit = true;
          break;
        case 'b':
          if(backslash) str += '\b';
          break;
        case 'f':
          if(backslash) str += '\f';
          break;
        case 'n':
          if(backslash) str += '\n';
          break;
        case 'r':
          if(backslash) str += '\r';
          break;
        case 't':
          if(backslash) str += '\t';
          break;
        case 'u':
          if(backslash) {
            text.next();
            std::string unicode = read_unicode(text);
            #ifdef NO_EXCEPTIONS
            if(unicode.empty()) {
              return json::error("parsing: invalid unicode character");
            }
            #endif

            str += unicode;
          }

          break;
      }

      if(exit) break;

      if(!backslash) str += c;
      else if(c != '\\') backslash = false;

      text.next();
    }

    text.next();

    return json::value(str);
  }

  bool is_digit(const char c) {
    switch(c) {
      case '0': case '1':
      case '2': case '3':
      case '4': case '5':
      case '6': case '7':
      case '8': case '9':
        return true;
      default:
        return false;
    }
  }

  std::string read_fractional(string_iterator& text) {
    std::string buffer;
    if(text.peek() == '.') {
      buffer += text.next();
      if(is_digit(text.peek())) {
        do {
          buffer += text.next();
        } while(is_digit(text.peek()));
      } else {
        #ifndef NO_EXCEPTIONS
        throw json::exception("parsing: decimal must be followed by digits");
        #else
        return "";
        #endif
      }
    }

    return buffer;
  }

  std::string read_exponent(string_iterator& text) {
    std::string buffer;
    if(text.peek() == 'e' || text.peek() == 'E') {
      buffer += text.next();
      if(text.peek() == '+') buffer += text.next();
      else if(text.peek() == '-') buffer += text.next();

      if(is_digit(text.peek())) {
        do {
          buffer += text.next();
        } while(is_digit(text.peek()));
      } else {
        #ifndef NO_EXCEPTIONS
        throw json::exception("parsing: exponent must be followed by digits");
        #else
        return "";
        #endif
      }
    }

    return buffer;
  }

  value::value(std::initializer_list<json::pair> list):
    type(json::value_type::object) {
    for(const auto& [key, value]: list) {
      dict[key] = value;
    }
  }

  json::value read_number(string_iterator& text) {
    std::string str;
    json::value_type type = json::value_type::integer;

    if(text.peek() == '-') {
      // Optional minus sign for numbers
      str += text.next();
    }

    if(text.peek() == '0') {
      str += text.next();
    } else if(is_digit(text.peek())) {
      do {
        str += text.next();
      } while(is_digit(text.peek()));
    } else {
      const char* msg = "parsing: invalid number";
      #ifndef NO_EXCEPTIONS
      throw json::exception(msg);
      #else
      return json::error(msg);
      #endif
    }

    if(text.peek() == '.') {
      const std::string fractional = read_fractional(text);

      #ifdef NO_EXCEPTIONS
      if(fractional.empty()) {
        return json::error("parsing: decimal must be followed by digits");
      }
      #endif

      type = json::value_type::floating;
      str += fractional;
    }

    switch(text.peek()) {
      case 'e':
      case 'E': {
        const std::string exponent = read_exponent(text);

        #ifdef NO_EXCEPTIONS
        if(exponent.empty()) {
          return json::error("parsing: exponent must be followed by digits");
        }
        #endif

        str += read_exponent(text);
        type = json::value_type::floating;
      } break;
    }

    return json::value(type, str);
  }

  json::value read_array(string_iterator& text) {
    bool exit = false, ready = true;

    std::vector<json::value> values;

    if(text.peek() == '[') {
      text.next();

      while(text.available()) {
        switch(text.peek()) {
          case ',':
            ready = true;
            text.next();
          case ' ': case '\r':
          case '\n': case '\t':
            break;
          case ']':
            exit = true;
            break;
          default:
            if(!ready) {
              const char* msg = "parsing: invalid array";
              #ifndef NO_EXCEPTIONS
              throw json::exception(msg);
              #else
              return json::error(msg);
              #endif
            }

            break;
        }

        if(ready && !exit) {
          values.push_back(read_value(text));

          #ifdef NO_EXCEPTIONS
          if(values.back().error()) return values.back();
          #endif

          ready = false;
        } else {
          text.next();
          if(exit) break;
        }
      }

      return json::value(values);
    }

    return json::value();
  }

  json::value read_object(string_iterator& text) {
    text.next();

    std::unordered_map<std::string, json::value> values;

    bool exit = false;

    std::string key;

    while(text.available()) {
      switch(text.peek()) {
        case '\"':
          key = (std::string)read_string(text);
          break;
        case ':':
          text.next();
          values[key] = read_value(text);

          #ifdef NO_EXCEPTIONS
          if(values[key].error()) return values[key];
          #endif

          key.clear();
          break;
        case ',':
          text.next();
          if(!key.empty()) {
            const char* msg = "parsing: object key does not have value";
            #ifndef NO_EXCEPTIONS
            throw json::exception(msg);
            #else
            return json::error(msg);
            #endif
          }
          break;
        case '}':
          exit = true;
          break;
        case ' ': case '\r':
        case '\n': case '\t':
          text.next();
          break;
        default:
          const char* msg = "parsing: invalid object";
          #ifndef NO_EXCEPTIONS
          throw json::exception(msg);
          #else
          return json::error(msg);
          #endif
          break;
      }

      if(exit) break;
    }

    text.next();
    return json::value(values);
  }

  json::value read_value(string_iterator& text) {
    std::string buffer;
    bool exit = false;

    while(text.available()) {
      switch(text.peek()) {
        case '{':
          return read_object(text);
        case '[':
          return read_array(text);
        case '"':
          return read_string(text);
        case '-':
        case '0': case '1':
        case '2': case '3':
        case '4': case '5':
        case '6': case '7':
        case '8': case '9':
          return read_number(text);
        case ' ': case '\n':
        case '\r': case '\t':
          break;
        case ',':
          exit = true;
          break;
        default:
          buffer += text.peek();
          break;
      }

      text.next();

      if(exit) break;
    }

    std::string_view view = buffer;
    view.remove_prefix(std::min(buffer.size(),
                                buffer.find_first_not_of(" \t\n\r")));
    view.remove_suffix(std::min(buffer.size(),
                                buffer.size() - buffer.find_last_not_of(" \t\n\r") - 1));

    if(view == "true") {
      return json::value(json::value_type::true_literal, "true");
    } else if(view == "false") {
      return json::value(json::value_type::false_literal, "false");
    } else if(view == "null") {
      return json::value(json::value_type::null_literal, "null");
    }

    const char* msg = "parsing: unrecognized literal";
    #ifndef NO_EXCEPTIONS
    throw json::exception(msg);
    #else
    return json::error(msg);
    #endif
  }

  json::value array(std::vector<json::value> list) {
    return json::value(list);
  }

  json::value error(const std::string& msg) {
    return json::value(json::value_type::undefined, msg);
  }

  json::value parse(std::string_view text) {
    string_iterator string{text};

    return read_value(string);
  }

  json::value load(const std::filesystem::path& filename) {
    std::fstream file(filename);

    std::string contents{
      std::istreambuf_iterator<char>(file),
      std::istreambuf_iterator<char>()
    };

    return json::parse(contents);
  }
}
