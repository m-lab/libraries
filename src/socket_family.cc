#include "mlab/socket_family.h"

#include <errno.h>
#include <memory.h>

#if defined(OS_LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID) || defined(OS_FREEBSD)
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#elif defined(OS_WINDOWS)
#include <WinSock2.h>
#include <ws2ipdef.h>
#include <WS2tcpip.h>
#else
#error Undefined platform
#endif

#include <string>

#include "log.h"

namespace mlab {
SocketFamily GetSocketFamilyForAddress(const std::string& addr) {
  struct addrinfo hint;
  struct addrinfo *info;
  memset(&hint, 0, sizeof(hint));
  hint.ai_family = AF_UNSPEC;
  hint.ai_flags = AI_NUMERICHOST;  // addr is a numerical network address

  if (getaddrinfo(addr.c_str(), 0, &hint, &info) == 0) {
    switch (info->ai_family) {
      case AF_INET: return SOCKETFAMILY_IPV4;
      case AF_INET6: return SOCKETFAMILY_IPV6;
      default: return SOCKETFAMILY_UNSPEC;
    }
  } else {
    LOG(ERROR, "getaddrinfo fails. %s [%d]", strerror(errno), errno);
    return SOCKETFAMILY_UNSPEC;
  }
}
}  // namespace mlab
