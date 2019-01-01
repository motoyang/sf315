#include <iostream>

#include <uvp.hpp>

typedef void (*init_plugin_function)();

int main(int argc, char *argv[]) {
  if (argc == 1) {
    std::cerr << "Usage: " << argv[0] << " [plugin1] [plugin2] ..."
              << std::endl;
    return 0;
  }
  uvp::initialize("./", "plugin", 1, 3);

  uvp::Lib lib;
  for (int i = 1; i < argc; ++i) {
    std::cout << "loading " << argv[i] << std::endl;
    if (lib.open(argv[i])) {
      continue;
    }
    init_plugin_function init_plug;
    if (lib.dlsym("initialize", (void**)&init_plug)) {
      continue;
    }
    init_plug();
  }

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}
