#include "tls.h"
#include "versionsupported.h"

// --

constexpr static ProtocolVersion s_pvSupported[] = {PV_TLS_1_1, PV_TLS_1_2,
                                                    PV_TLS_1_3};
constexpr static ProtocolVersion s_selected = PV_TLS_1_3;

VersionSupported::VersionSupported() {}

void VersionSupported::supported(ClientSupportedVersions *csv) const {
  auto count = COUNT_OF(s_pvSupported);
  csv->len(sizeof(s_pvSupported));
  auto data = csv->data();

  for (auto i = 0; i < count; ++i) {
    data[i] = s_pvSupported[i];
  }
}

ProtocolVersion VersionSupported::selected() const { return s_selected; }
