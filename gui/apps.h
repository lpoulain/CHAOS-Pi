#ifndef __APPS_H
#define __APPS_H

void app_kernelheap_init();
void app_filesystem_init();
void app_memory_init();
void app_memory_first_touch(int x, int y);
void app_memory_process_touch_event(enum TouchStatus status);

#endif
