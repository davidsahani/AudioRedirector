#include "MAConvert.hpp"
#include <array>

/// Generic mapping between an enum type and its string representation.
template <typename Enum>
struct Mapping {
    Enum value;
    const char* str;
};

/// Convert enum → string
template <typename Enum, size_t N>
constexpr const char* to_string(Enum value, const std::array<Mapping<Enum>, N>& table, const char* defaultStr) {
    for (auto&& m : table) {
        if (m.value == value) return m.str;
    }
    return defaultStr;
}

/// Convert string → enum
template <typename Enum, size_t N>
std::optional<Enum> to_enum(std::string_view str, const std::array<Mapping<Enum>, N>& table) {
    auto it = std::find_if(table.begin(), table.end(),
        [&](const auto& m){ return str == m.str; });
    return it != table.end() ? std::optional<Enum>{it->value} : std::nullopt;
}

// ------------------------------------------------------------------------
// Mappings (enum ↔ string)
// ------------------------------------------------------------------------

static constexpr std::array<Mapping<ma_format>, 6> kFormatMappings{{
    { ma_format_unknown, "Unknown" },
    { ma_format_u8,      "u8 (Unsigned 8-bit)" },
    { ma_format_s16,     "s16 (Signed 16-bit)" },
    { ma_format_s24,     "s24 (Signed 24-bit)" },
    { ma_format_s32,     "s32 (Signed 32-bit)" },
    { ma_format_f32,     "f32 (32-bit Float)" }
}};

// ------------------------------------------------------------------------
// Conversion functions (enum ↔ string)
// ------------------------------------------------------------------------

const char* ma::convert::to_string(ma_format format) {
    return to_string(format, kFormatMappings, "Invalid Format");
}

std::optional<ma_format> ma::convert::to_format(std::string_view str) {
    return to_enum(str, kFormatMappings);
}

const char* ma::convert::to_string(ma_result result) {
    switch (result) {
        case MA_SUCCESS:                        return "Success";
        case MA_ERROR:                          return "Unspecified Error";  /* A generic error. */
        case MA_INVALID_ARGS:                   return "Invalid Arguments";
        case MA_INVALID_OPERATION:              return "Invalid Operation";
        case MA_OUT_OF_MEMORY:                  return "Out of Memory";
        case MA_OUT_OF_RANGE:                   return "Out of Range";
        case MA_ACCESS_DENIED:                  return "Access Denied";
        case MA_DOES_NOT_EXIST:                 return "Does Not Exist";
        case MA_ALREADY_EXISTS:                 return "Already Exists";
        case MA_TOO_MANY_OPEN_FILES:            return "Too Many Open Files";
        case MA_INVALID_FILE:                   return "Invalid File";
        case MA_TOO_BIG:                        return "Too Big";
        case MA_PATH_TOO_LONG:                  return "Path Too Long";
        case MA_NAME_TOO_LONG:                  return "Name Too Long";
        case MA_NOT_DIRECTORY:                  return "Not Directory";
        case MA_IS_DIRECTORY:                   return "Is Directory";
        case MA_DIRECTORY_NOT_EMPTY:            return "Directory Not Empty";
        case MA_AT_END:                         return "At End";
        case MA_NO_SPACE:                       return "No Space";
        case MA_BUSY:                           return "Busy";
        case MA_IO_ERROR:                       return "IO Error";
        case MA_INTERRUPT:                      return "Interrupt";
        case MA_UNAVAILABLE:                    return "Unavailable";
        case MA_ALREADY_IN_USE:                 return "Already In Use";
        case MA_BAD_ADDRESS:                    return "Bad Address";
        case MA_BAD_SEEK:                       return "Bad Seek";
        case MA_BAD_PIPE:                       return "Bad Pipe";
        case MA_DEADLOCK:                       return "Deadlock";
        case MA_TOO_MANY_LINKS:                 return "Too Many Links";
        case MA_NOT_IMPLEMENTED:                return "Not Implemented";
        case MA_NO_MESSAGE:                     return "No Message";
        case MA_BAD_MESSAGE:                    return "Bad Message";
        case MA_NO_DATA_AVAILABLE:              return "No Data Available";
        case MA_INVALID_DATA:                   return "Invalid Data";
        case MA_TIMEOUT:                        return "Timeout";
        case MA_NO_NETWORK:                     return "No Network";
        case MA_NOT_UNIQUE:                     return "Not Unique";
        case MA_NOT_SOCKET:                     return "Not Socket";
        case MA_NO_ADDRESS:                     return "No Address";
        case MA_BAD_PROTOCOL:                   return "Bad Protocol";
        case MA_PROTOCOL_UNAVAILABLE:           return "Protocol Unavailable";
        case MA_PROTOCOL_NOT_SUPPORTED:         return "Protocol Not Supported";
        case MA_PROTOCOL_FAMILY_NOT_SUPPORTED:  return "Protocol Family Not Supported";
        case MA_ADDRESS_FAMILY_NOT_SUPPORTED:   return "Address Family Not Supported";
        case MA_SOCKET_NOT_SUPPORTED:           return "Socket Not Supported";
        case MA_CONNECTION_RESET:               return "Connection Reset";
        case MA_ALREADY_CONNECTED:              return "Already Connected";
        case MA_NOT_CONNECTED:                  return "Not Connected";
        case MA_CONNECTION_REFUSED:             return "Connection Refused";
        case MA_NO_HOST:                        return "No Host";
        case MA_IN_PROGRESS:                    return "In Progress";
        case MA_CANCELLED:                      return "Cancelled";
        case MA_MEMORY_ALREADY_MAPPED:          return "Memory Already Mapped";

        /* General non-standard errors. */
        case MA_CRC_MISMATCH:                   return "CRC Mismatch";

        /* General miniaudio-specific errors. */
        case MA_FORMAT_NOT_SUPPORTED:           return "Format Not Supported";
        case MA_DEVICE_TYPE_NOT_SUPPORTED:      return "Device Type Not Supported";
        case MA_SHARE_MODE_NOT_SUPPORTED:       return "Share Mode Not Supported";
        case MA_NO_BACKEND:                     return "No Backend";
        case MA_NO_DEVICE:                      return "No Device";
        case MA_API_NOT_FOUND:                  return "API Not Found";
        case MA_INVALID_DEVICE_CONFIG:          return "Invalid Device Config";
        case MA_LOOP:                           return "Loop";
        case MA_BACKEND_NOT_ENABLED:            return "Backend Not Enabled";

        /* State errors. */
        case MA_DEVICE_NOT_INITIALIZED:         return "Device Not Initialized";
        case MA_DEVICE_ALREADY_INITIALIZED:     return "Device Already Initialized";
        case MA_DEVICE_NOT_STARTED:             return "Device Not Started";
        case MA_DEVICE_NOT_STOPPED:             return "Device Not Stopped";

        /* Operation errors. */
        case MA_FAILED_TO_INIT_BACKEND:         return "Failed To Init Backend";
        case MA_FAILED_TO_OPEN_BACKEND_DEVICE:  return "Failed To Open Backend Device";
        case MA_FAILED_TO_START_BACKEND_DEVICE: return "Failed To Start Backend Device";
        case MA_FAILED_TO_STOP_BACKEND_DEVICE:  return "Failed To Stop Backend Device";

        default: return "Unknown Error";    /* Missing case? */
    }
}
