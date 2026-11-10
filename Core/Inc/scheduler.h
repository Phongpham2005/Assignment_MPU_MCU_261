/*
 * scheduler.h
 *
 *  Created on: Nov 10, 2026
 *      Author: Acer
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdint.h>

// Cấu trúc dữ liệu cho một Task
typedef struct {
    void   (*pTask)(void);   // Con trỏ tới hàm task
    uint32_t Delay;           // Số tick còn lại (tương đối so với task đứng trước)
    uint32_t Period;          // Chu kỳ lặp lại, 0 = one-shot task
    uint8_t  RunMe;           // Cờ báo task sẵn sàng chạy
    uint32_t TaskID;          // ID định danh task (tuỳ chọn)
} sTask;

// Các hàm giao tiếp của Scheduler
void     SCH_Init(void);
uint32_t SCH_Add_Task(void (*pFunc)(void), uint32_t DELAY, uint32_t PERIOD);
uint8_t  SCH_Delete_Task(uint32_t taskID);
void     SCH_Update(void);          // Gọi bên trong ngắt timer (ví dụ TIM2)
void     SCH_Dispatch_Tasks(void);  // Gọi liên tục trong vòng lặp while(1)

#endif /* SCHEDULER_H_ */
