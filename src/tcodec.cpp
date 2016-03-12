#include "mbedcrypto/tcodec.hpp"

#include "mbedtls/base64.h"
///////////////////////////////////////////////////////////////////////////////
namespace mbedcrypto {
namespace {
///////////////////////////////////////////////////////////////////////////////

constexpr char
hex_lower(unsigned char b) noexcept {
    return "0123456789abcdef"[b & 0x0f];
}

///////////////////////////////////////////////////////////////////////////////
} // namespace anon
///////////////////////////////////////////////////////////////////////////////

buffer_t
hex::encode(const unsigned char* src, size_t length) {
    buffer_t buffer(length << 1, '\0');
    unsigned char* hexdata = reinterpret_cast<unsigned char*>(&buffer.front());

    for ( size_t i = 0;    i < length;    ++i ) {
        hexdata[i << 1]       = hex_lower(src[i] >> 4);
        hexdata[(i << 1) + 1] = hex_lower(src[i] & 0x0f);
    }

    return buffer;
}

buffer_t
hex::decode(const char* src, size_t length) {
    if ( length == 0 )
        length = std::strlen(src);

    if ( length == 0 ) // empty buffer
        return buffer_t{};

    if ( (length & 1) != 0 ) // size must be even
        throw exception("invalid size for hex string");

    buffer_t buffer(length >> 1, '\0');
    unsigned char* bindata = reinterpret_cast<unsigned char*>(&buffer.front());

    size_t j = 0, k = 0;
    for ( size_t i = 0;    i < length;    ++i, ++src ) {
        char s = *src;

        if      ( s >= '0'  &&  s <= '9' ) j = s - '0';
        else if ( s >= 'A'  &&  s <= 'F' ) j = s - '7';
        else if ( s >= 'a'  &&  s <= 'f' ) j = s - 'W';
        else
            throw exception("invalid character in hex string");

        k = ( ( i & 1 ) != 0 ) ? j : j << 4;
        bindata[i >> 1] = (unsigned char)( bindata[i >> 1] | k );
    }

    return buffer;
}

///////////////////////////////////////////////////////////////////////////////

size_t
base64::encode_size(const unsigned char* src, size_t srclen) noexcept {
    size_t olen = 0;
    mbedtls_base64_encode(
            nullptr, 0, // dest
            &olen,
            src, srclen
            );

    return olen;
}

size_t
base64::decode_size(const unsigned char* src, size_t srclen) noexcept {
    size_t olen = 0;
    mbedtls_base64_decode(
            nullptr, 0, // dest
            &olen,
            src, srclen
            );

    return olen;
}

size_t
base64::encode_size(const buffer_t& src) {
    return encode_size(reinterpret_cast<const unsigned char*>(src.data()),
            src.size()
            );
}

size_t
base64::decode_size(const buffer_t& src) {
    return decode_size(reinterpret_cast<const unsigned char*>(src.data()),
            src.size()
            );
}

int
base64::encode(const unsigned char* src, size_t srclen,
        unsigned char* dest, size_t& destlen) noexcept {
    size_t olen = 0;
    int ret = mbedtls_base64_encode(
            dest, destlen,
            &olen,
            src, srclen
            );
    destlen = olen;

    return ret;
}

int
base64::decode(const unsigned char* src, size_t srclen,
        unsigned char* dest, size_t& destlen) noexcept {
    size_t olen = 0;
    int ret = mbedtls_base64_decode(
            dest, destlen,
            &olen,
            src, srclen
            );
    destlen = olen;

    return ret;
}

void
base64::encode(const buffer_t& src, buffer_t& dest) {
    size_t requiredSize = encode_size(src);
    if ( dest.capacity() <= requiredSize )
        dest.reserve(requiredSize + 1);

    dest.resize(requiredSize);
    size_t dsize = requiredSize;
    int ret = encode(
            reinterpret_cast<const unsigned char*>(src.data()),
            src.size(),
            reinterpret_cast<unsigned char*>(&dest.front()),
            dsize
            );

    if ( ret != 0 )
        throw exception(ret, "failed to base64 encode");

    // adjust possible null byte at the end
    dest.resize(dsize);
}

void
base64::decode(const buffer_t& src, buffer_t& dest) {
    size_t requiredSize = decode_size(src);
    if ( dest.capacity() <= requiredSize )
        dest.reserve(requiredSize + 1);

    dest.resize(requiredSize);
    size_t dsize = requiredSize;
    int ret = decode(
            reinterpret_cast<const unsigned char*>(src.data()),
            src.size(),
            reinterpret_cast<unsigned char*>(&dest.front()),
            dsize
            );

    if ( ret != 0 )
        throw exception(ret, "failed to base64 decode");
}

buffer_t
base64::encode(const buffer_t& src) {
    buffer_t dest;
    encode(src, dest);
    return dest;
}

buffer_t
base64::decode(const buffer_t& src) {
    buffer_t dest;
    decode(src, dest);
    return dest;
}



///////////////////////////////////////////////////////////////////////////////
} // namespace mbedcrypto
///////////////////////////////////////////////////////////////////////////////