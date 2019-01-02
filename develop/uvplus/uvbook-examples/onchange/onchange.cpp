#include <iostream>
#include <memory>
#include <list>

#include <uvp.hpp>

int main(int argc, char *argv[]) {
  if (argc <= 2) {
    std::cerr << "Usage: " << argv[0] << " <command> <file1> [file2 ...]"
              << std::endl;
    return 1;
  }

  uvp::initialize("./", "onchange", 1, 3);
  auto l = std::make_unique<uvp::LoopObject>();
  std::string command = argv[1];

  auto runCommand = [&command](uvp::FsEvent *fsEvent, const char *filename, int events,
                        int status) {
    std::string path;
    fsEvent->getPath(path);
    std::cout << "Change detected in " << path;
    if (events & UV_RENAME)
      std::cout << " renamed ";
    if (events & UV_CHANGE)
      std::cout << " changed ";
    std::cout << (filename ? filename : "") << std::endl;

    system(command.c_str());
  };

  std::list<std::unique_ptr<uvp::FsEvent>> events;
  while (argc-- > 2) {
    std::cout << "Adding watch on " << argv[argc] << std::endl;
    auto p = std::make_unique<uvp::FsEventObject>(l.get());
    p->start(runCommand, argv[argc], UV_FS_EVENT_RECURSIVE);
    events.push_back(std::move(p));
  }

  l->run(UV_RUN_DEFAULT);
  l->close();

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}
