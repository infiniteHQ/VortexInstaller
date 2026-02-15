#pragma once

#include <mutex>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

class BackendClient {
 public:
  BackendClient() = default;
  ~BackendClient();

  void Start();
  void SendCommand(const std::string& cmd);
  void Poll();

 private:
  std::mutex io_mutex;

#ifdef _WIN32
  HANDLE hProcess = NULL;
  HANDLE hWrite = NULL;  // UI -> backend stdin
  HANDLE hRead = NULL;   // backend stdout -> UI
#else
  int write_fd = -1;  // UI -> backend
  int read_fd = -1;   // backend -> UI
#endif

  std::string readBuffer;
};
