#include <algorithm>
#include "tls.h"
#include "versionsupported.h"

// --

constexpr static ProtocolVersion s_pvSupported[] = { // PV_TLS_1_1, PV_TLS_1_2,
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

bool VersionSupported::selected(ProtocolVersion *pv,
                                ClientSupportedVersions *csv) const {
  bool r = true;
  auto begin_data = csv->data();
  auto end_data = begin_data + csv->len() / sizeof(*begin_data);
  auto found = std::find(begin_data, end_data, s_selected);
  if (found == end_data) {
    r = false;
  }
  *pv = s_selected;
  return r;
}
