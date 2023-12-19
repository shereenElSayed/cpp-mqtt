#include <cstdio>
#include <time.h>

#include "cspot_mqtt.h"
#include "woofc.h"

int laptop_handler(WOOF *wf, unsigned long wf_seq_no, void *ptr)
{
    EL *el = (EL*)ptr;
    printf("In laptop_handler:: Element: %d\n", el->value);

}
