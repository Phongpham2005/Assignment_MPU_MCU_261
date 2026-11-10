/*
 * scheduler.c
 *
 *  Created on: Nov 10, 2026
 *      Author: Acer
 */

#include "scheduler.h"

// Định nghĩa số lượng task tối đa trong hệ thống (tùy chọn theo dự án)
#define SCH_MAX_TASKS 40

static sTask SCH_tasks_G[SCH_MAX_TASKS];
static uint8_t SCH_count = 0;

void SCH_Init(void) {
    SCH_count = 0;
    for (int i = 0; i < SCH_MAX_TASKS; i++) {
        SCH_tasks_G[i].pTask = 0;
        SCH_tasks_G[i].Delay = 0;
        SCH_tasks_G[i].Period = 0;
        SCH_tasks_G[i].RunMe = 0;
        SCH_tasks_G[i].TaskID = 0;
    }
}

// Hàm SCH_Update đạt độ phức tạp O(1) - Gọi trong ngắt Timer (10ms)
void SCH_Update(void) {
    if (SCH_count == 0) return;

    // Giảm Delay của task đầu tiên
    if (SCH_tasks_G[0].Delay > 0) {
        SCH_tasks_G[0].Delay--;
    }

    // Nếu Delay về 0, bật cờ RunMe cho task đầu tiên
    if (SCH_tasks_G[0].Delay == 0) {
        SCH_tasks_G[0].RunMe = 1;
    }
}

// Thêm task mới vào scheduler (Sắp xếp theo thời gian chạy tương đối - Linked-list logic trên mảng)
uint32_t SCH_Add_Task(void (*pFunc)(void), uint32_t DELAY, uint32_t PERIOD) {
    if (SCH_count >= SCH_MAX_TASKS) {
        return 0; // Tràn mảng task
    }

    uint32_t index = 0;
    uint32_t accumulated_delay = 0;

    // Tìm vị trí thích hợp để chèn task dựa trên khoảng Delay tương đối
    while (index < SCH_count) {
        accumulated_delay += SCH_tasks_G[index].Delay;
        if (DELAY < accumulated_delay) {
            accumulated_delay -= SCH_tasks_G[index].Delay;
            break;
        }
        index++;
    }

    // Dời các task phía sau xuống để nhường chỗ
    for (int i = SCH_count; i > index; i--) {
        SCH_tasks_G[i] = SCH_tasks_G[i - 1];
    }

    // Chèn task mới vào vị trí index
    SCH_tasks_G[index].pTask = pFunc;
    SCH_tasks_G[index].Period = PERIOD;
    SCH_tasks_G[index].RunMe = 0;

    if (index == 0) {
        SCH_tasks_G[index].Delay = DELAY;
        if (SCH_count > 0) {
            SCH_tasks_G[1].Delay -= DELAY; // Cập nhật lại delay tương đối cho task kế tiếp
        }
    } else {
        SCH_tasks_G[index].Delay = DELAY - accumulated_delay;
        if (index + 1 <= SCH_count) {
            SCH_tasks_G[index + 1].Delay -= SCH_tasks_G[index].Delay;
        }
    }

    SCH_count++;
    return (SCH_count - 1); // Trả về ID hoặc Index của task
}

// Thực thi các task có cờ RunMe = 1 (Gọi trong while(1))
void SCH_Dispatch_Tasks(void) {
    if (SCH_count == 0) return;

    if (SCH_tasks_G[0].RunMe > 0) {
        (*SCH_tasks_G[0].pTask)(); // Chạy task

        sTask tempTask = SCH_tasks_G[0];

        // Xóa task cũ ở đầu danh sách (dồn mảng lên)
        SCH_Delete_Task(0);

        // Nếu là task tuần hoàn (Period > 0), tự động thêm lại vào scheduler
        if (tempTask.Period > 0) {
            SCH_Add_Task(tempTask.pTask, tempTask.Period, tempTask.Period);
        }
    }
}

// Xóa task khỏi danh sách theo vị trí/ID
uint8_t SCH_Delete_Task(uint32_t taskID) {
    if (taskID >= SCH_count) return 0;

    // Nếu xóa task ở đầu và còn task phía sau, cộng dồn delay sang task kế tiếp
    if (taskID == 0 && SCH_count > 1) {
        SCH_tasks_G[1].Delay += SCH_tasks_G[0].Delay;
    }

    for (uint32_t i = taskID; i < SCH_count - 1; i++) {
        SCH_tasks_G[i] = SCH_tasks_G[i + 1];
    }

    SCH_count--;
    SCH_tasks_G[SCH_count].pTask = 0;
    SCH_tasks_G[SCH_count].Delay = 0;
    SCH_tasks_G[SCH_count].Period = 0;
    SCH_tasks_G[SCH_count].RunMe = 0;

    return 1;
}
