#ifndef PAGE_STACK_H
#define PAGE_STACK_H

#include "../../GUI_App/ui.h"

#define MAX_DEPTH 6
// 页面结构体
typedef struct{
    void (*init)(void); // 页面初始化函数指针
    void (*deinit)(void); //
    lv_obj_t **page_obj;
} Page_t;

// 页面堆栈结构体
typedef struct{
    Page_t* pages[MAX_DEPTH];
    uint8_t top;
} PageStack_t;

Page_t* Page_Get_NowPage(void);
void Page_Back(void);
void Page_Back_Bottom(void);
void Page_Load(Page_t *newPage);
void Pages_init(void);


#endif // PAGE_STACK_H
