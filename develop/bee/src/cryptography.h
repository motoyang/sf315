#pragma once

#include <unordered_map>

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

// #include "tls.h"
// #include "factory.hpp"

// --

namespace secure {
  using namespace Botan;

  bool initialize();
}

// --

// extern const std::unordered_map<NamedGroup, secure::BigInt> g_dh_supported;
// extern const std::unordered_map<NamedGroup, std::string> g_ecdh_supported;

// --

