#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "woofc.h"
#include "cspot_mqtt.h"


int main(int argc, char **argv)
{
    unsigned long err;
    char* Wname = argv[1];
    WooFInit();
    unsigned long ndx = WooFGetLatestSeqno(Wname);
    if (WooFInvalid(ndx)) {
            fprintf(stderr, "WooFGetLatestSeqno failed\n");
            fflush(stderr);
            exit(1);
    }
    printf("log size %lu\n", ndx);
    fflush(stdout);
    unsigned long i;
    EL el;
    for (i = 1; i <= ndx; i++) {
        if (WooFGet(Wname, &el, i) < 0) {
                printf("ERRRORR\n");
                fflush(stdout);
                fprintf(stderr, "WooFGet(%lu) failed\n", i);
                fflush(stderr);
                exit(1);
            }
        printf("%d: %d - start: %f\n",i, el.value, el.start_time);
    }

}
