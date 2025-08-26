#include "base.hpp"

#ifndef _WIN32
  #include <netdb.h>
  #include <unistd.h>
#endif

bool VortexInstallerNet::CheckNet() {
#ifdef _WIN32
    int rc = std::system("ping -n 1 -w 2000 1.1.1.1 >NUL 2>&1");
    if (rc != 0)
        rc = std::system("ping -n 1 -w 2000 8.8.8.8 >NUL 2>&1");
    return rc == 0;
#else
    addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo("google.com", "80", &hints, &res);

    if (res)
        freeaddrinfo(res);

    return (status == 0);
#endif
}
