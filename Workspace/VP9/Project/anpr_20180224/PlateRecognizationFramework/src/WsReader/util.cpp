#include "util.h"

void print_hex_memory(void *mem, int pos, int n) {
    int i;
    unsigned char *p = (unsigned char *)mem;
    for (i=pos;i<n;i++) {
        printf("0x%02x ", p[i] & 0xff);
        if (((i-pos)%16==0) && (i!=pos))
            printf("\n");
    }
    printf("\n");
}

void exportVectorToFile(string url, vector<char> v) {
  std::ofstream outfile;
  outfile.open(url, std::ios_base::app);
  for (const auto &e : v) outfile << e;
}
