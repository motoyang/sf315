#pragma once

#include <memory>

// --

class VersionSupported {
  // struct Impl;
  // std::unique_ptr<Impl> _impl;

public:
  VersionSupported();
  // virtual ~VersionSupported() = default;

  // VersionSupported(const VersionSupported&) = delete;
  // VersionSupported(VersionSupported&&) = delete;
  // VersionSupported& operator=(const VersionSupported&) = delete;
  // VersionSupported& operator=(VersionSupported&&) = delete;

  void supported(ClientSupportedVersions* csv) const;
  ProtocolVersion selected() const;
};
