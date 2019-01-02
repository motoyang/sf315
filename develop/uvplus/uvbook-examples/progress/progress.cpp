#include <iostream>

#include <uvp.hpp>

void printProgress(uvp::Async *handle) {
  double percentage = *((double *)handle->data());
  std::cerr << "Downloaded " << std::setprecision(2) << std::fixed << percentage
            << "%" << std::endl;
}

int main(int argc, char *ls[]) {
  uvp::initialize("./", "progress", 1, 3);
  auto l = std::make_unique<uvp::LoopObject>();
  int size = 10240;
  uvp::WorkReq req;
  req.data(&size);

  uvp::AsyncObject async(l.get(), printProgress);
  l->queueWork(&req,
               [&async](uvp::Work *req) {
                 int size = *((int *)req->data());
                 int downloaded = 0;
                 while (downloaded < size) {
                   double percentage = downloaded * 100.0 / size;
                   async.data((void *)&percentage);
                   async.send();
                   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                   downloaded += (200 + random()) %
                                 1000; // can only download max 1000bytes/sec,
                                       // but at least a 200;
                 }
               },
               [&async](uvp::Work *req, int status) {
                 std::cerr << "Download complete" << std::endl;
                 async.close(nullptr);
               });

  l->run(UV_RUN_DEFAULT);
  l->close();

  LOG_INFO << "quit with return code: " << 0;
  return 0;
}
