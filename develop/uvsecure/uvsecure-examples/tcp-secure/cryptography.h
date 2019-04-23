#pragma once

#include <list>

#include <botan/hex.h>

#include <botan/rng.h>
#include <botan/auto_rng.h>

#include <botan/hash.h>
#include <botan/hkdf.h>
#include <botan/cipher_mode.h>

#include <botan/dl_group.h>
#include <botan/dh.h>
#include <botan/ec_group.h>
#include <botan/ecdh.h>
#include <botan/pubkey.h>

// --

namespace secure {
  using namespace Botan;

  // bool initialize();
}

// --

using ssvector = secure::secure_vector<uint8_t>;
using ssvlist = std::list<ssvector>;

// --

