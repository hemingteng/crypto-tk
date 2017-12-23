//
// libsse_crypto - An abstraction layer for high level cryptographic features.
// Copyright (C) 2015-2017 Raphael Bost
//
// This file is part of libsse_crypto.
//
// libsse_crypto is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// libsse_crypto is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with libsse_crypto.  If not, see <http://www.gnu.org/licenses/>.
//

#pragma once

#include "hash.hpp"
#include "hmac.hpp"
#include "key.hpp"
#include "random.hpp"

#include <cstdint>
#include <cstring>

#include <algorithm>
#include <array>
#include <string>

namespace sse {

namespace crypto {


/// @class Prf
/// @brief Pseudorandom function.
///
/// The Prf templates realizes a pseudorandom function (PRF) using HMac-H, where
/// H is the hash function defined in hash.hpp (Blake2b).
///
/// It is templated according
/// to the output length. The rationale behind templating according the output
/// length is to avoid key-reuse across different calls to HMac with different
/// output length. If the the output length (NBYTES) is larger than the HMac's
/// digest, the output will be generated by blocks, using HMac in a counter
/// mode.
///
/// @tparam NBYTES  The output size (in bytes)
///

template<uint16_t NBYTES>
class Prf
{
public:
    /// @brief PRF key size (in bytes)
    static constexpr uint8_t kKeySize = 32;

    static_assert(kKeySize <= Hash::kBlockSize,
                  "The PRF key is too large for the hash block size");

    ///
    /// @brief Constructor
    ///
    /// Creates a Prf object with a new randomly generated key.
    ///
    Prf() : base_()
    {
    }

    ///
    /// @brief Constructor
    ///
    /// Creates a Prf object from a kKeySize (32) bytes key.
    /// After a call to the constructor, the input key is
    /// held by the Prf object, and cannot be re-used.
    ///
    /// @param key  The key used to initialize the PRF.
    ///             Upon return, k is empty
    ///
    explicit Prf(Key<kKeySize>&& key) : base_(std::move(key))
    {
    }

    /// @brief Destructor.
    ~Prf() // NOLINT // using = default causes a linker error on Travis
    {
    }

    ///
    /// @brief Evaluate the PRF
    ///
    /// Evaluates the PRF on the input buffer and places the result in an array.
    ///
    ///
    /// @param in       The input buffer. Must be non NULL.
    /// @param length   The size of the input buffer in bytes.
    ///
    /// @return         An std::array of NBYTES bytes containing the result of
    ///                 the evaluation
    ///
    /// @exception std::invalid_argument       in is NULL
    ///
    std::array<uint8_t, NBYTES> prf(const unsigned char* in,
                                    const size_t         length) const;

    ///
    /// @brief Evaluate the PRF
    ///
    /// Evaluates the PRF on the input string and places the result in an array.
    ///
    ///
    /// @param s        The input string.
    ///
    /// @return         An std::array of NBYTES bytes containing the result of
    ///                 the evaluation
    ///
    std::array<uint8_t, NBYTES> prf(const std::string& s) const;

    ///
    /// @brief Evaluate the PRF
    ///
    /// Evaluates the PRF on the input array and places the result in an array.
    ///
    ///
    /// @param in       The input array.
    ///
    /// @tparam L       The input array length.
    ///
    /// @return         An std::array of NBYTES bytes containing the result of
    ///                 the evaluation
    ///
    template<size_t L>
    std::array<uint8_t, NBYTES> prf(const std::array<uint8_t, L>& in) const;

    ///
    /// @brief Derive a key using the PRF
    ///
    /// Creates and returns a new Key object, of size NBYTES, by calling the PRF
    /// on the input buffer. The input acts as a salt for the key derivation.
    ///
    ///
    /// @param in       The input buffer.
    /// @param length   The size of the input buffer in bytes.
    ///
    /// @return         A new NBYTES bytes key.
    ///
    /// @exception std::invalid_argument       in is NULL
    ///
    Key<NBYTES> derive_key(const unsigned char* in, const size_t length) const;

    ///
    /// @brief Derive a key using the PRF
    ///
    /// Creates and returns a new Key object, of size NBYTES, by calling the PRF
    /// on the input string. The input acts as a salt for the key derivation.
    ///
    ///
    /// @param s        The input string.
    ///
    /// @return         A new NBYTES bytes key.
    ///
    Key<NBYTES> derive_key(const std::string& s) const;

    ///
    /// @brief Derive a key using the PRF
    ///
    /// Creates and returns a new Key object, of size NBYTES, by calling the PRF
    /// on the input array. The input acts as a salt for the key derivation.
    ///
    ///
    /// @param in       The input array.
    ///
    /// @tparam L       The input array length.
    ///
    /// @return         A new NBYTES bytes key.
    ///
    template<size_t L>
    Key<NBYTES> derive_key(const std::array<uint8_t, L>& in) const;

private:
    /// @internal
    /// @brief Inner implementation of the PRF
    using PrfBase = HMac<Hash, kKeySize>;

    PrfBase base_;
};

template<uint16_t NBYTES>
constexpr uint8_t Prf<NBYTES>::kKeySize;

// PRF instantiation
// For now, use HMAC-Hash where Hash is the hash function defined in hash.hpp
template<uint16_t NBYTES>
std::array<uint8_t, NBYTES> Prf<NBYTES>::prf(const unsigned char* in,
                                             const size_t         length) const
{
    if (in == nullptr) {
        throw std::invalid_argument("in is NULL");
    }

    static_assert(
        NBYTES != 0,
        "PRF output length invalid: length must be strictly larger than 0");

    std::array<uint8_t, NBYTES> result;

    if (NBYTES > PrfBase::kDigestSize) {
        unsigned char* tmp = new unsigned char[length + 1];
        memcpy(tmp, in, length);

        uint16_t pos = 0;
        uint8_t  i   = 0;
        for (; pos < NBYTES; pos += PrfBase::kDigestSize, i++) {
            // use a counter mode
            tmp[length] = i;

            // fill res
            if (static_cast<size_t>(NBYTES - pos) >= PrfBase::kDigestSize) {
                base_.hmac(
                    tmp, length + 1, result.data() + pos, PrfBase::kDigestSize);
            } else {
                base_.hmac(tmp,
                           length + 1,
                           result.data() + pos,
                           static_cast<size_t>(NBYTES - pos));
            }
        }

        sodium_memzero(tmp, length + 1);
        delete[] tmp;
    } else if (NBYTES <= Hash::kDigestSize) {
        // only need one output bloc of PrfBase.
        base_.hmac(in, length, result.data(), result.size());
    }


    return result;
}

// Convienience function to run the PRF over a C++ string
template<uint16_t NBYTES>
std::array<uint8_t, NBYTES> Prf<NBYTES>::prf(const std::string& s) const
{
    return prf(reinterpret_cast<const unsigned char*>(s.data()), s.length());
}

template<uint16_t NBYTES>
template<size_t L>
std::array<uint8_t, NBYTES> Prf<NBYTES>::prf(
    const std::array<uint8_t, L>& in) const
{
    return prf(reinterpret_cast<const unsigned char*>(in.data()), L);
}

// derive a key using the PRF

template<uint16_t NBYTES>
Key<NBYTES> Prf<NBYTES>::derive_key(const unsigned char* in,
                                    const size_t         length) const
{
    return Key<NBYTES>(prf(in, length).data());
}

template<uint16_t NBYTES>
Key<NBYTES> Prf<NBYTES>::derive_key(const std::string& s) const
{
    return Key<NBYTES>(prf(s).data());
}

template<uint16_t NBYTES>
template<size_t L>
Key<NBYTES> Prf<NBYTES>::derive_key(const std::array<uint8_t, L>& in) const
{
    return Key<NBYTES>(prf(in).data());
}


} // namespace crypto
} // namespace sse

// Explicitely instantiate some templates for the code coverage
#ifdef CHECK_TEMPLATE_INSTANTIATION
namespace sse {
namespace crypto {
extern template class Prf<1>;
extern template std::array<uint8_t, 1> Prf<1>::prf(
    const std::array<uint8_t, 20>& in) const;
extern template Key<1> Prf<1>::derive_key(
    const std::array<uint8_t, 20>& in) const;

extern template class Prf<10>;
extern template std::array<uint8_t, 10> Prf<10>::prf(
    const std::array<uint8_t, 40>& in) const;
extern template Key<10> Prf<10>::derive_key(
    const std::array<uint8_t, 40>& in) const;

extern template class Prf<20>;
extern template std::array<uint8_t, 20> Prf<20>::prf(
    const std::array<uint8_t, 50>& in) const;
extern template Key<20> Prf<20>::derive_key(
    const std::array<uint8_t, 50>& in) const;

extern template class Prf<128>;
extern template std::array<uint8_t, 128> Prf<128>::prf(
    const std::array<uint8_t, 100>& in) const;
extern template Key<128> Prf<128>::derive_key(
    const std::array<uint8_t, 100>& in) const;

extern template class Prf<1024>;
extern template std::array<uint8_t, 1024> Prf<1024>::prf(
    const std::array<uint8_t, 200>& in) const;
extern template Key<1024> Prf<1024>::derive_key(
    const std::array<uint8_t, 200>& in) const;

extern template class Prf<2000>;
} // namespace crypto
} // namespace sse
#endif
