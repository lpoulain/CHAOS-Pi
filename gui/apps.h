#ifndef __APPS_H
#define __APPS_H

void app_kernelheap_load(UserInterface *ui);
void app_kernelheap_wake_up(UserInterface *ui);
void app_filesystem_load(UserInterface *ui);
void app_filesystem_wake_up(UserInterface *ui);
void app_memory_load(UserInterface *ui);
void app_memory_wake_up(UserInterface *ui);

void app_memory_first_touch(int x, int y);
void app_memory_process_touch_event(enum TouchStatus status);

#endif
