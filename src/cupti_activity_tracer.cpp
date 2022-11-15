#include "cupti_activity_tracer.h"
#include <fstream>
#include "cuda_activity_definitions.h"

#include <unistd.h>

#define CUPTI_CALL(call)                                                 \
    do {                                                                 \
        CUptiResult _status = call;                                      \
        if (_status != CUPTI_SUCCESS) {                                  \
            const char* errstr;                                          \
            cuptiGetResultString(_status, &errstr);                      \
            fprintf(stderr,                                              \
                    "%s:%d: error: function %s failed with error %s.\n", \
                    __FILE__, __LINE__, #call, errstr);                  \
            exit(-1);                                                    \
        }                                                                \
    } while (0)

#define BUF_SIZE (32 * 1024)
#define ALIGN_SIZE (8)
#define ALIGN_BUFFER(buffer, align)                                   \
    (((uintptr_t)(buffer) & ((align)-1))                              \
         ? ((buffer) + (align) - ((uintptr_t)(buffer) & ((align)-1))) \
         : (buffer))

// Timestamp at trace initialization time. Used to normalized other timestamps
static uint64_t start_timestamp;
std::ofstream fout;

void csv_print(std::string Type,
               unsigned long long Start,
               unsigned long long End,
               unsigned int Thread,
               unsigned int Process,
               unsigned int Stream,
               unsigned int Correlation,
               std::string Note) {
    fout << "\"" << Type << "\"," << Start << "," << End << "," << Thread
         << "," << Process << "," << Stream << "," << Correlation << ",\""
         << Note << "\"\n";
}

static void print_activity(CUpti_Activity* record) {
    switch (record->kind) {
        case CUPTI_ACTIVITY_KIND_MEMCPY: {
            CUpti_ActivityMemcpy5* memcpy = (CUpti_ActivityMemcpy5*)record;
            std::string note = std::string(get_memcopy_events_string(
                (CUpti_ActivityMemcpyKind)memcpy->copyKind));
            csv_print("MEMCPY",
                      (unsigned long long)(memcpy->start - start_timestamp),
                      (unsigned long long)(memcpy->end - start_timestamp), 0, 0,
                      memcpy->streamId, memcpy->correlationId, note);
            break;
        }
        case CUPTI_ACTIVITY_KIND_MEMSET: {
            CUpti_ActivityMemset4* memset = (CUpti_ActivityMemset4*)record;
            csv_print("MEMSET",
                      (unsigned long long)(memset->start - start_timestamp),
                      (unsigned long long)(memset->end - start_timestamp), 0, 0,
                      memset->streamId, memset->correlationId, "");
            break;
        }
        case CUPTI_ACTIVITY_KIND_KERNEL:
        case CUPTI_ACTIVITY_KIND_CONCURRENT_KERNEL: {
            const char* kindString =
                (record->kind == CUPTI_ACTIVITY_KIND_KERNEL) ? "KERNEL"
                                                             : "CONC KERNEL";
            CUpti_ActivityKernel7* kernel = (CUpti_ActivityKernel7*)record;
            std::string note =
                std::string(kindString) + "; " + std::string(kernel->name);
            csv_print("KERNEL",
                      (unsigned long long)(kernel->start - start_timestamp),
                      (unsigned long long)(kernel->end - start_timestamp), 0, 0,
                      kernel->streamId, kernel->correlationId, note);
            break;
        }
        case CUPTI_ACTIVITY_KIND_DRIVER: {
            CUpti_ActivityAPI* api = (CUpti_ActivityAPI*)record;
            csv_print("DRIVER",
                      (unsigned long long)(api->start - start_timestamp),
                      (unsigned long long)(api->end - start_timestamp),
                      api->threadId, api->processId, 0, api->correlationId, "");
            break;
        }
        case CUPTI_ACTIVITY_KIND_RUNTIME: {
            CUpti_ActivityAPI* api = (CUpti_ActivityAPI*)record;
            csv_print("RUNTIME",
                      (unsigned long long)(api->start - start_timestamp),
                      (unsigned long long)(api->end - start_timestamp),
                      api->processId, api->threadId, 0, api->correlationId, "");
            break;
        }
        case CUPTI_ACTIVITY_KIND_SYNCHRONIZATION: {
            CUpti_ActivitySynchronization* activity_sync =
                (CUpti_ActivitySynchronization*)record;
            std::string note =
                std::string(get_sync_events_string(activity_sync->type));
            csv_print(
                "SYNC",
                (unsigned long long)activity_sync->start - start_timestamp,
                (unsigned long long)activity_sync->end - start_timestamp, 0, 0,
                activity_sync->streamId, activity_sync->correlationId, note);
            break;
        }
        default:
            break;
    }
}

void CUPTIAPI bufferRequested(uint8_t** buffer,
                              size_t* size,
                              size_t* maxNumRecords) {
    uint8_t* bfr = (uint8_t*)malloc(BUF_SIZE + ALIGN_SIZE);
    if (bfr == NULL) {
        printf("Error: out of memory\n");
        exit(-1);
    }

    *size = BUF_SIZE;
    *buffer = ALIGN_BUFFER(bfr, ALIGN_SIZE);
    *maxNumRecords = 0;
}

void CUPTIAPI bufferCompleted(CUcontext ctx,
                              uint32_t streamId,
                              uint8_t* buffer,
                              size_t size,
                              size_t validSize) {
    CUptiResult status;
    CUpti_Activity* record = NULL;

    if (validSize > 0) {
        do {
            status = cuptiActivityGetNextRecord(buffer, validSize, &record);
            if (status == CUPTI_SUCCESS) {
                print_activity(record);
            } else if (status == CUPTI_ERROR_MAX_LIMIT_REACHED)
                break;
            else {
                CUPTI_CALL(status);
            }
        } while (1);
    }

    free(buffer);
}

void cupti_activity_tracer_init() {
    fout.open("cupti_activity_tracer.csv", std::ios::out | std::ios::trunc);
    fout << "Type,Start,End,Thread,Process,Stream,Correlation,Note\n";

    // enable activities
    CUPTI_CALL(cuptiActivityEnable(CUPTI_ACTIVITY_KIND_DEVICE));
    CUPTI_CALL(cuptiActivityEnable(CUPTI_ACTIVITY_KIND_CONTEXT));
    CUPTI_CALL(cuptiActivityEnable(CUPTI_ACTIVITY_KIND_DRIVER));
    CUPTI_CALL(cuptiActivityEnable(CUPTI_ACTIVITY_KIND_RUNTIME));
    CUPTI_CALL(cuptiActivityEnable(CUPTI_ACTIVITY_KIND_MEMCPY));
    CUPTI_CALL(cuptiActivityEnable(CUPTI_ACTIVITY_KIND_MEMSET));
    CUPTI_CALL(cuptiActivityEnable(CUPTI_ACTIVITY_KIND_NAME));
    CUPTI_CALL(cuptiActivityEnable(CUPTI_ACTIVITY_KIND_MARKER));
    CUPTI_CALL(cuptiActivityEnable(CUPTI_ACTIVITY_KIND_CONCURRENT_KERNEL));
    CUPTI_CALL(cuptiActivityEnable(CUPTI_ACTIVITY_KIND_SYNCHRONIZATION));
    CUPTI_CALL(
        cuptiActivityRegisterCallbacks(bufferRequested, bufferCompleted));

    size_t attrValue = 0, attrValueSize = sizeof(size_t);
    CUPTI_CALL(cuptiActivityGetAttribute(CUPTI_ACTIVITY_ATTR_DEVICE_BUFFER_SIZE,
                                         &attrValueSize, &attrValue));
    attrValue *= 2;
    CUPTI_CALL(cuptiActivitySetAttribute(CUPTI_ACTIVITY_ATTR_DEVICE_BUFFER_SIZE,
                                         &attrValueSize, &attrValue));

    CUPTI_CALL(
        cuptiActivityGetAttribute(CUPTI_ACTIVITY_ATTR_DEVICE_BUFFER_POOL_LIMIT,
                                  &attrValueSize, &attrValue));
    attrValue *= 2;
    CUPTI_CALL(
        cuptiActivitySetAttribute(CUPTI_ACTIVITY_ATTR_DEVICE_BUFFER_POOL_LIMIT,
                                  &attrValueSize, &attrValue));

    CUPTI_CALL(cuptiGetTimestamp(&start_timestamp));
}

void cupti_activity_tracer_close() {
    // Force flush any remaining activity buffers before termination of the
    // application
    CUPTI_CALL(cuptiActivityFlushAll(1));

    sleep(3);
    fout.close();
}
