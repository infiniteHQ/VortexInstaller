#include "backend_client.hpp"

#include <stdexcept>

#include "../api/api.hpp"
#ifndef _WIN32
#include <fcntl.h>
#include <sys/wait.h>
void BackendClient::Start() {
  int in_pipe[2];   // UI -> backend
  int out_pipe[2];  // backend -> UI

  if (pipe(in_pipe) != 0 || pipe(out_pipe) != 0)
    throw std::runtime_error("pipe() failed");

  pid_t pid = fork();

  if (pid < 0)
    throw std::runtime_error("fork() failed");

  if (pid == 0) {
    // --- Child (backend) ---
    dup2(in_pipe[0], STDIN_FILENO);
    dup2(out_pipe[1], STDOUT_FILENO);
    close(in_pipe[1]);

    close(out_pipe[0]);
    execlp(
        "pkexec",
        "pkexec",
        VortexInstaller::GetPath("./vortex_installer_backend").c_str(),

        nullptr);
    _exit(1);
  }

  // --- Parent (UI) ---
  close(in_pipe[0]);
  close(out_pipe[1]);
  write_fd = in_pipe[1];

  read_fd = out_pipe[0];  // non-blocking read
  fcntl(read_fd, F_SETFL, O_NONBLOCK);
}

#endif
#ifdef _WIN32
void BackendClient::Start() {
  SECURITY_ATTRIBUTES sa{ sizeof(sa), nullptr, TRUE };

  HANDLE childStdinRead, childStdinWrite;
  HANDLE childStdoutRead, childStdoutWrite;

  CreatePipe(&childStdinRead, &childStdinWrite, &sa, 0);
  CreatePipe(&childStdoutRead, &childStdoutWrite, &sa, 0);

  SetHandleInformation(childStdinWrite, HANDLE_FLAG_INHERIT, 0);
  SetHandleInformation(childStdoutRead, HANDLE_FLAG_INHERIT, 0);

  STARTUPINFOA si{};
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdInput = childStdinRead;
  si.hStdOutput = childStdoutWrite;
  si.hStdError = childStdoutWrite;

  PROCESS_INFORMATION pi{};

  if (!CreateProcessA(
          nullptr, (LPSTR) "vortex_installer_backend.exe", nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi))
    throw std::runtime_error("CreateProcess failed");

  CloseHandle(childStdinRead);
  CloseHandle(childStdoutWrite);

  hProcess = pi.hProcess;
  hWrite = childStdinWrite;
  hRead = childStdoutRead;
}
#endif
void BackendClient::SendCommand(const std::string& cmd) {
  std::lock_guard<std::mutex> lock(io_mutex);

  nlohmann::json j{ { "type", "command" }, { "name", cmd } };

  std::string msg = j.dump() + "\n";

#ifdef _WIN32
  DWORD written;
  WriteFile(hWrite, msg.c_str(), (DWORD)msg.size(), &written, nullptr);
#else
  write(write_fd, msg.c_str(), msg.size());
#endif
}

void BackendClient::Refresh() {
  std::lock_guard<std::mutex> lock(io_mutex);

  nlohmann::json j{ { "type", "refresh" } };

  std::string msg = j.dump() + "\n";

#ifdef _WIN32
  DWORD written;
  WriteFile(hWrite, msg.c_str(), (DWORD)msg.size(), &written, nullptr);
#else
  write(write_fd, msg.c_str(), msg.size());
#endif
}

void BackendClient::SendPatch() {
  std::lock_guard<std::mutex> lock(io_mutex);

  nlohmann::json j;
  j["type"] = "patch";
  j["data"] = VortexInstaller::GetContext()->ToJson();

  std::string msg = j.dump() + "\n";

#ifdef _WIN32
  DWORD written;
  WriteFile(hWrite, msg.c_str(), (DWORD)msg.size(), &written, nullptr);
#else
  write(write_fd, msg.c_str(), msg.size());
#endif
}

void BackendClient::Poll() {
  char buf[1024];

#ifdef _WIN32
  DWORD available = 0;
  if (!PeekNamedPipe(hRead, nullptr, 0, nullptr, &available, nullptr) || available == 0)
    return;

  DWORD readBytes = 0;
  ReadFile(hRead, buf, sizeof(buf), &readBytes, nullptr);
  if (readBytes == 0)
    return;

  readBuffer.append(buf, readBytes);
#else
  ssize_t n = read(read_fd, buf, sizeof(buf));
  if (n <= 0)
    return;

  readBuffer.append(buf, n);
#endif

  size_t pos;
  while ((pos = readBuffer.find('\n')) != std::string::npos) {
    std::string line = readBuffer.substr(0, pos);
    readBuffer.erase(0, pos + 1);

    auto msg = nlohmann::json::parse(line, nullptr, false);
    if (!msg.is_object())
      continue;

    if (msg["type"] == "refresh") {
      VortexInstaller::GetContext()->PatchFromJson(msg["data"]);
    } else if (msg["type"] == "approved") {
      VortexInstaller::GetContext()->g_PollkitApproved = true;
    }
  }
}
BackendClient::~BackendClient() {
#ifdef _WIN32
  if (hProcess)
    CloseHandle(hProcess);
  if (hRead)
    CloseHandle(hRead);
  if (hWrite)
    CloseHandle(hWrite);
#else
  if (read_fd >= 0)
    close(read_fd);
  if (write_fd >= 0)
    close(write_fd);
#endif
}
