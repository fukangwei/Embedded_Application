#include "mysys.h"

void NVIC_Configuration ( void ) {
    NVIC_PriorityGroupConfig ( NVIC_PriorityGroup_2 );
}