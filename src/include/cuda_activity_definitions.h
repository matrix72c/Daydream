#include <cuda.h>
#include <cupti.h>

static const char* get_memcopy_events_string(CUpti_ActivityMemcpyKind kind) {
    switch (kind) {
        case CUPTI_ACTIVITY_MEMCPY_KIND_HTOD:
            return "HtoD";
        case CUPTI_ACTIVITY_MEMCPY_KIND_DTOH:
            return "DtoH";
        case CUPTI_ACTIVITY_MEMCPY_KIND_HTOA:
            return "HtoA";
        case CUPTI_ACTIVITY_MEMCPY_KIND_ATOH:
            return "AtoH";
        case CUPTI_ACTIVITY_MEMCPY_KIND_ATOA:
            return "AtoA";
        case CUPTI_ACTIVITY_MEMCPY_KIND_ATOD:
            return "AtoD";
        case CUPTI_ACTIVITY_MEMCPY_KIND_DTOA:
            return "DtoA";
        case CUPTI_ACTIVITY_MEMCPY_KIND_DTOD:
            return "DtoD";
        case CUPTI_ACTIVITY_MEMCPY_KIND_HTOH:
            return "HtoH";
        default:
            break;
    }
    return "unknown";
}

static const char* get_sync_events_string(uint32_t kind) {
    switch (kind) {
        case CUPTI_ACTIVITY_SYNCHRONIZATION_TYPE_EVENT_SYNCHRONIZE:
            return "gpu_sync_events";
        case CUPTI_ACTIVITY_SYNCHRONIZATION_TYPE_STREAM_WAIT_EVENT:
            return "gpu_sync_stream_event_wait";
        case CUPTI_ACTIVITY_SYNCHRONIZATION_TYPE_STREAM_SYNCHRONIZE:
            return "gpu_sync_stream";
        case CUPTI_ACTIVITY_SYNCHRONIZATION_TYPE_CONTEXT_SYNCHRONIZE:
            return "gpu_sync_context";
        case CUPTI_ACTIVITY_SYNCHRONIZATION_TYPE_UNKNOWN:
        default:
            break;
    }
    return "unknown";
}

const char* get_activity_overhead_string(CUpti_ActivityOverheadKind kind) {
    switch (kind) {
        case CUPTI_ACTIVITY_OVERHEAD_DRIVER_COMPILER:
            return "compiler";
        case CUPTI_ACTIVITY_OVERHEAD_CUPTI_BUFFER_FLUSH:
            return "buffer_flush";
        case CUPTI_ACTIVITY_OVERHEAD_CUPTI_INSTRUMENTATION:
            return "instrumentation";
        case CUPTI_ACTIVITY_OVERHEAD_CUPTI_RESOURCE:
            return "resource";
        default:
            break;
    }

    return "unknown";
}

const char* get_activity_object_string(CUpti_ActivityObjectKind kind) {
    switch (kind) {
        case CUPTI_ACTIVITY_OBJECT_PROCESS:
            return "process";
        case CUPTI_ACTIVITY_OBJECT_THREAD:
            return "thread";
        case CUPTI_ACTIVITY_OBJECT_DEVICE:
            return "device";
        case CUPTI_ACTIVITY_OBJECT_CONTEXT:
            return "context";
        case CUPTI_ACTIVITY_OBJECT_STREAM:
            return "stream";
        default:
            break;
    }

    return "unknown";
}

uint32_t get_activity_object_id(CUpti_ActivityObjectKind kind,
                                CUpti_ActivityObjectKindId* id) {
    switch (kind) {
        case CUPTI_ACTIVITY_OBJECT_PROCESS:
            return id->pt.processId;
        case CUPTI_ACTIVITY_OBJECT_THREAD:
            return id->pt.threadId;
        case CUPTI_ACTIVITY_OBJECT_DEVICE:
            return id->dcs.deviceId;
        case CUPTI_ACTIVITY_OBJECT_CONTEXT:
            return id->dcs.contextId;
        case CUPTI_ACTIVITY_OBJECT_STREAM:
            return id->dcs.streamId;
        default:
            break;
    }

    return 0xffffffff;
}

static const char* get_compute_api_string(CUpti_ActivityComputeApiKind kind) {
    switch (kind) {
        case CUPTI_ACTIVITY_COMPUTE_API_CUDA:
            return "CUDA";
        case CUPTI_ACTIVITY_COMPUTE_API_CUDA_MPS:
            return "CUDA_MPS";
        default:
            break;
    }

    return "<unknown>";
}
