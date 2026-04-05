#ifndef ISR_H
#define ISR_H

#include "utils.h"

void isr_handler(u64 int_no);
void irq_handler(u64 int_no);

#endif