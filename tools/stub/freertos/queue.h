// Stub FreeRTOS queue - syntax-check only.
#pragma once
#include "FreeRTOS.h"
#include <stddef.h>
typedef void* QueueHandle_t;
inline QueueHandle_t xQueueCreate(int, size_t) { return (QueueHandle_t)1; }
inline BaseType_t xQueueSend(QueueHandle_t, const void*, int) { return pdTRUE; }
inline BaseType_t xQueueReceive(QueueHandle_t, void*, int) { return pdFALSE; }
