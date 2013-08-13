#include "mlab/mlab.h"

#include <stdio.h>

int main() {
  mlab_initialize("c_test", "0.1");

  struct Hostname* hostname = mlab_ns_lookup_hostname_for_tool("npad");

  while (hostname != 0) {
    printf("Found: %s: %s\n", hostname->hostname, hostname->ip_address);
    hostname = hostname->next;
  }

  // TODO(dominich): More tests.

  mlab_shutdown();
  return 0;
}
