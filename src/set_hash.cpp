#include "set_hash.hpp"

#include "hash.hpp"

#include "ecmh/binary_elliptic_curve/GLS254.hpp"
#include "ecmh/multiset_hash/ECMH.hpp"
#include "ecmh/array_view/array_view.hpp"

#include <iostream>
#include <iomanip>

using namespace jbms::multiset_hash;

namespace sse
{

namespace crypto
{

	/*
	 * HashWrapper
	 * 
	 * A wrapper around the Hash class to make it compatible with the ECMH code.
	 *
	 */
	
	class HashWrapper
	{
	public:
	    constexpr static size_t digest_bytes = sse::crypto::Hash::kDigestSize;
	    constexpr static size_t block_bytes = sse::crypto::Hash::kBlockSize;
		
	    static void hash(unsigned char *out, const unsigned char *in, size_t inlen)
		{
			sse::crypto::Hash::hash(in, inlen, out);
		}
		
	};

typedef jbms::multiset_hash::ECMH<jbms::binary_elliptic_curve::GLS254, sse::crypto::HashWrapper, false> MSH;

class SetHash::SetHashImpl
{
public:

	SetHashImpl();	
	SetHashImpl(const MSH::State &s);	
	SetHashImpl(const SetHash::SetHashImpl& o) = default;	
	SetHashImpl(const std::string &hex);
	~SetHashImpl();

	void add_element(const std::string &in);
	void add_set(const SetHashImpl *h);
	void remove_element(const std::string &in);
	void remove_set(const SetHashImpl *h);
	MSH::State invert_set();
	
	std::string hex() const;
	bool operator==(const SetHashImpl& h) const;
		
	

    // static MSH *ecmh__;
private:	
	// typedef jbms::multiset_hash::ECMH<jbms::binary_elliptic_curve::GLS254, sse::crypto::HashWrapper, false> MSH;
    // static const MSH ecmh_;
   	MSH::State state_;
	
	static const MSH& ecmh()
	{
		// if(ecmh__ == NULL)
		// 	ecmh__ = new MSH();
		static MSH ecmh__;
		return ecmh__;
	}
};

// SetHash::SetHashImpl::MSH const SetHash::SetHashImpl::ecmh_{};
// MSH *SetHash::SetHashImpl::ecmh__ = NULL;

SetHash::SetHash() : set_hash_imp_(new SetHashImpl())
{
}

SetHash::SetHash(const std::string &hex) :  set_hash_imp_(new SetHashImpl(hex))
{
}

SetHash::SetHash(const SetHash& o) : set_hash_imp_(new SetHashImpl(o.set_hash_imp_->hex()))
{
}

SetHash::SetHash(const SetHash&& o) : set_hash_imp_(std::move(o.set_hash_imp_))
{
}

SetHash::~SetHash() 
{ 
	delete set_hash_imp_;
}

void SetHash::add_element(const std::string &in)
{
	set_hash_imp_->add_element(in);
}

void SetHash::add_set(const SetHash &h)
{
	set_hash_imp_->add_set(h.set_hash_imp_);
}

void SetHash::remove_element(const std::string &in)
{
	set_hash_imp_->remove_element(in);
}

void SetHash::remove_set(const SetHash &h)
{
	set_hash_imp_->remove_set(h.set_hash_imp_);
}

SetHash SetHash::invert_set()
{
	SetHash h;
	*h.set_hash_imp_ = SetHash::SetHashImpl(set_hash_imp_->invert_set());
	return h;
}

std::string SetHash::hex() const
{
	return set_hash_imp_->hex();
}


std::ostream& operator<<(std::ostream& os, const SetHash& h)
{
	os << h.set_hash_imp_->hex();
	return os;
}

SetHash& SetHash::operator=(const SetHash& h)
{
	delete set_hash_imp_;
	set_hash_imp_ = new SetHashImpl(*h.set_hash_imp_);
	return *this;
}

bool SetHash::operator==(const SetHash& h) const
{
	return (*set_hash_imp_ == *h.set_hash_imp_);
}

bool SetHash::operator!=(const SetHash& h) const
{
	return !(*this == h);
}

/*
 * SetHashImpl
 *
 */


SetHash::SetHashImpl::SetHashImpl()
{
	state_ = initial_state(ecmh());
}

SetHash::SetHashImpl::SetHashImpl(const MSH::State &s) : state_(s)
{
}

SetHash::SetHashImpl::SetHashImpl(const std::string &hex)
{
	state_ = from_hex(ecmh(), hex);
}

SetHash::SetHashImpl::~SetHashImpl() 
{ 
}

void SetHash::SetHashImpl::add_element(const std::string &in)
{
    add(ecmh(), state_, in);
}

void SetHash::SetHashImpl::add_set(const SetHash::SetHashImpl *in)
{
    add_hash(ecmh(), state_, in->state_);
}

void SetHash::SetHashImpl::remove_element(const std::string &in)
{
    remove(ecmh(), state_, in);
}

void SetHash::SetHashImpl::remove_set(const SetHash::SetHashImpl *in)
{
    remove_hash(ecmh(), state_, in->state_);
}

MSH::State SetHash::SetHashImpl::invert_set()
{
	return invert(ecmh(), state_);
}

std::string SetHash::SetHashImpl::hex() const
{
	return to_hex(ecmh(), state_);
}

bool SetHash::SetHashImpl::operator==(const SetHash::SetHashImpl& h) const
{
	return equal(ecmh().curve(), state_, h.state_);
}

}
}