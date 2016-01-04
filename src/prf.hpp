//
// libsse_crypto - An abstraction layer for high level cryptographic features.
// Copyright (C) 2015 Raphael Bost
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

#include "random.hpp"
#include "hmac.hpp"
#include "hash.hpp"

#include <cstdint>
#include <cstring>
#include <cassert>

#include <string>
#include <array>
#include <algorithm>

namespace sse
{

namespace crypto
{


/*****
 * Prf class
 *
 * A wrapper for cryptographic keys pseudo-random function.
 * PRFs are templatized according to the length output: 
 * one must not be able to use the same PRF 
 * with different output lenght
******/

template <uint8_t NBYTES> class Prf
{
public:
	typedef HMac<Hash> PrfBase;
	static constexpr uint8_t kKeySize = PrfBase::kKeySize;
		
	Prf() : base_()
	{
	}
	
	Prf(const void* k) : base_(k)
	{
	}

	Prf(const void* k, const uint8_t &len) : base_(k, len)
	{
	}
	
	Prf(const std::string& k) : base_(k)
	{
	}
	
	Prf(const std::string& k, const uint8_t &len) : base_(k, len)
	{
	}
	
	Prf(const std::array<uint8_t,kKeySize>& k) : base_(k)
	{	
	}

	Prf(const Prf<NBYTES>& prf) : base_(prf.base_)
	{		
	}

	// Destructor.
	~Prf() {}; 

	const std::array<uint8_t,kKeySize>& key() const
	{
		return base_.key();
	};
	
	const uint8_t* key_data() const
	{
		return base_.key_data();
	};
	
	std::array<uint8_t, NBYTES> prf(const unsigned char* in, const size_t &length) const;
	std::array<uint8_t, NBYTES> prf(const std::string &s) const;
	void prf(const unsigned char* in, const size_t &length, unsigned char* out) const;
	std::string prf_string(const std::string &s) const;
private:
	
	PrfBase base_;
};

// PRF instantiation
// For now, use HMAC-Hash where Hash is the hash function defined in hash.hpp
template <uint8_t NBYTES> std::array<uint8_t, NBYTES> Prf<NBYTES>::prf(const unsigned char* in, const size_t &length) const
{
	static_assert(NBYTES != 0, "PRF output length invalid: length must be strictly larger than 0");

	std::array<uint8_t, NBYTES> result;
	
	assert(NBYTES <= sse::crypto::Hash::kDigestSize);
	
	if(NBYTES <= sse::crypto::Hash::kDigestSize){
		// only need one output bloc of PrfBase.

        std::copy_n(base_.hmac(in, length).begin(), NBYTES, result.begin());
	}
	
	
	return result;
}

// Convienience function to run the PRF over a C++ string
template <uint8_t NBYTES> std::array<uint8_t, NBYTES> Prf<NBYTES>::prf(const std::string &s) const
{
	return prf((unsigned char*)s.data() , s.length());
}


// Convienience function to return the PRF result in a raw array
template <uint8_t NBYTES> void Prf<NBYTES>::prf(const unsigned char* in, const size_t &length, unsigned char* out) const
{
	base_.hmac(in, length, out);
}


template <uint8_t NBYTES> std::string Prf<NBYTES>::prf_string(const std::string &s) const
{
	std::array<uint8_t, NBYTES> tmp = prf(s);
	std::string result(tmp.begin(), tmp.end());
	
	return result;
}

} // namespace crypto
} // namespace sse