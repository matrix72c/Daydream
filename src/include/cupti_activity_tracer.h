#include <cuda.h>
#include <cupti.h>
#include <stdio.h>
#include <iostream>

// libunwind MACRO for local unwind optimization
#define UNW_LOCAL_ONLY
#include "libunwind.h"

void cupti_activity_tracer_init();
void cupti_activity_tracer_close();
void CUPTIAPI bufferCompleted(CUcontext ctx,
                              uint32_t streamId,
                              uint8_t* buffer,
                              size_t size,
                              size_t validSize);