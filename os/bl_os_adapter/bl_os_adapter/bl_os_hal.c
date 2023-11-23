/****************************************************************************
 * components/platform/soc/bl602/bl602_os_adapter/bl602_os_adapter/bl602_os_hal.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/
#undef ARCH_RISCV
#include <rtthread.h>

#include <bl_os_hal.h>
#include <bl_os_adapter/bl_os_adapter.h>
#include <bl_os_adapter/bl_os_log.h>

#include <stdio.h>
#include <stdarg.h>

#include <blog.h>
#include <aos/yloop.h>


#include "bflb_irq.h"

#define CFG_COMPONENT_BLOG_ENABLE 0


/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
struct bl_taskhandle
{
    rt_thread_t tid;
    rt_sem_t sem;
    rt_list_t node;
};
/****************************************************************************
 * Private Types
 ****************************************************************************/

struct mq_adpt
{
};

struct irq_adpt
{
    void (*func)(void *arg); /* Interrupt callback function */
    void *arg;               /* Interrupt private data */
};

enum bl_os_timer_mode
{
    BL_OS_TIEMR_ONCE = 0,
    BL_OS_TIEMR_CYCLE
};

typedef enum bl_os_timer_mode bl_os_timer_mode_t;

// struct timer_adpt
// {
//     TimerHandle_t handle;
//     int32_t delay;
//     bl_os_timer_mode_t mode;
//     void (*func)(void *arg);
//     void *arg;
// };

static rt_list_t task_list;
/****************************************************************************
 * Public Data
 ****************************************************************************/

extern void *__attribute__((weak)) _wifi_log_flag;

bl_ops_funcs_t g_bl_ops_funcs =
{
    ._version = BL_OS_ADAPTER_VERSION,
    ._printf = printf,
    ._puts = bl_os_puts,
    ._assert = bl_os_assert_func,
    ._init = bl_os_api_init,
    ._enter_critical = bl_os_enter_critical,
    ._exit_critical = bl_os_exit_critical,
    ._msleep = bl_os_msleep,
    ._sleep = bl_os_sleep,
    ._event_group_create = bl_os_event_create,
    ._event_group_delete = bl_os_event_delete,
    ._event_group_send = bl_os_event_send,
    ._event_group_wait = bl_os_event_wait,
    ._event_register = bl_os_event_register,
    ._event_notify = bl_os_event_notify,
    ._task_create = bl_os_task_create,
    ._task_delete = bl_os_task_delete,
    ._task_get_current_task = bl_os_task_get_current_task,
    ._task_notify_create = bl_os_task_notify_create,
    ._task_notify = bl_os_task_notify,
    ._task_wait = bl_os_task_wait,
    ._irq_attach = bl_os_irq_attach,
    ._irq_enable = bl_os_irq_enable,
    ._irq_disable = bl_os_irq_disable,
    ._workqueue_create = NULL,
    ._workqueue_submit_hp = NULL,
    ._workqueue_submit_lp = NULL,
    ._timer_create = bl_os_timer_create,
    ._timer_delete = bl_os_timer_delete,
    ._timer_start_once = bl_os_timer_start_once,
    ._timer_start_periodic = bl_os_timer_start_periodic,
    ._sem_create = bl_os_sem_create,
    ._sem_delete = bl_os_sem_delete,
    ._sem_take = bl_os_sem_take,
    ._sem_give = bl_os_sem_give,
    ._mutex_create = bl_os_mutex_create,
    ._mutex_delete = bl_os_mutex_delete,
    ._mutex_lock = bl_os_mutex_lock,
    ._mutex_unlock = bl_os_mutex_unlock,
    ._queue_create = bl_os_mq_creat,
    ._queue_delete = bl_os_mq_delete,
    ._queue_send_wait = bl_os_mq_send_wait,
    ._queue_send = bl_os_mq_send,
    ._queue_recv = bl_os_mq_recv,
    ._malloc = bl_os_malloc,
    ._free = bl_os_free,
    ._zalloc = bl_os_zalloc,
    ._get_time_ms = bl_os_clock_gettime_ms,
    ._get_tick = bl_os_get_tick,
    ._log_write = bl_os_log_write,
    ._task_notify_isr = bl_os_task_notify_isr,
    ._yield_from_isr = bl_os_yield_from_isr,
    ._ms_to_tick = bl_os_ms_to_tick,
    ._set_timeout = NULL,
    ._check_timeout = NULL
};

extern void vprint(const char *fmt, va_list argp);
volatile bool sys_log_all_enable = true;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: bl_os_assert_func
 *
 * Description:
 *   Validation program developer's expected result
 *
 * Input Parameters:
 *   file  - configASSERT file
 *   line  - configASSERT line
 *   func  - configASSERT function
 *   expr  - configASSERT condition
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void bl_os_assert_func(const char *file, int line, const char *func, const char *expr)
{
    printf("Assert failed in %s, %s:%d (%s)", func, file, line, expr);

    RT_ASSERT(0);
}

/****************************************************************************
 * Name: bl_os_event_create
 *
 * Description:
 *   Create event group
 *
 * Input Parameters:
 *
 * Returned Value:
 *   Event group data pointer
 *
 ****************************************************************************/

BL_EventGroup_t bl_os_event_create(void)
{
    char name[RT_NAME_MAX];
    static uint32_t event_cnt = 0;

    memset(name, 0, RT_NAME_MAX);
    snprintf(name, RT_NAME_MAX, "event-%02d", event_cnt ++);

    rt_event_t event = rt_event_create(name, RT_IPC_FLAG_PRIO);
    if (event == NULL)
    {
        rt_kprintf("rt_event_create failed!\r\n");
        RT_ASSERT(0);
    }

    return event;
}

/****************************************************************************
 * Name: bl_os_event_delete
 *
 * Description:
 *   Delete event and free resource
 *
 * Input Parameters:
 *   event  - event data point
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void bl_os_event_delete(BL_EventGroup_t event)
{
    if (rt_event_delete(event) != RT_EOK)
    {
        rt_kprintf("rt_event_delete failed!\r\n");
    }
}

/****************************************************************************
 * Name: bl_os_event_send
 *
 * Description:
 *   Set event bits
 *
 * Input Parameters:
 *
 * Returned Value:
 *   Event value after setting
 *
 ****************************************************************************/

uint32_t bl_os_event_send(BL_EventGroup_t event, uint32_t bits)
{
    rt_err_t ret = RT_EOK;

    ret = rt_event_send(event, bits);
    if (ret != RT_EOK)
    {
        rt_kprintf("rt_event_send failed!\r\n");
    }

    return event->set;
}

/****************************************************************************
 * Name: bl_os_event_wait
 *
 * Description:
 *   Delete timer and free resource
 *
 * Input Parameters:
 *   event
 *   bits_to_wait_for
 *   clear_on_exit
 *   wait_for_all_bits
 *   block_time_tick
 *
 * Returned Value:
 *   Current event value
 *
 ****************************************************************************/
uint32_t bl_os_event_wait(BL_EventGroup_t event, uint32_t bits_to_wait_for, int clear_on_exit,
                          int wait_for_all_bits, uint32_t block_time_tick)
{
    rt_uint32_t recved;
    rt_int32_t wait_ticks;

    if(block_time_tick == BL_OS_WAITING_FOREVER)
        wait_ticks = RT_WAITING_FOREVER;
    else
        wait_ticks = block_time_tick;

    rt_uint8_t option = 0;

    if (wait_for_all_bits != BL_OS_FALSE)
    {
        option |= RT_EVENT_FLAG_AND;
    }
    else
    {
        option |= RT_EVENT_FLAG_OR;
    }

    if (clear_on_exit != BL_OS_FALSE)
    {
        option |= RT_EVENT_FLAG_CLEAR;
    }
    
    if (rt_event_recv(event, bits_to_wait_for, option, wait_ticks, &recved) != RT_EOK)
    {
        rt_kprintf("rt_event_recv failed!\r\n");
        recved = event->set;
    }

    return recved;
}

/****************************************************************************
 * Name: bl_os_event_register
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_event_register(int type, void *cb, void *arg)
{
    aos_register_event_filter(type, cb, arg);
    return 0;
}

/****************************************************************************
 * Name: bl_os_event_notify
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_event_notify(int evt, int val)
{
    return aos_post_event(EV_WIFI, evt, val);
}

/****************************************************************************
 * Name: bl_os_task_create
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_task_create(const char *name,
                      void *entry,
                      uint32_t stack_depth,
                      void *param,
                      uint32_t prio,
                      BL_TaskHandle_t task_handle)
{
    char sem_name[RT_NAME_MAX];
    static uint32_t sem_cnt = 0;
    static bool init = false;

    if (init == false)
    {
        rt_list_init(&task_list);

        init = true;
    }

    struct bl_taskhandle *handle = (struct bl_taskhandle *)rt_malloc(sizeof(struct bl_taskhandle));
    if (handle == RT_NULL)
    {
        rt_kprintf("malloc BL_TaskHandle failed!\r\n");
        return -1;
    }

    memset(handle, 0, sizeof(struct bl_taskhandle));

    rt_thread_t tid = rt_thread_create(name, entry, param, stack_depth, prio, 20);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    else
    {
        rt_kprintf("rt_thread_create failed!\r\n");
        rt_free(handle);

        return -1;
    }

    memset(sem_name, 0, RT_NAME_MAX);
    snprintf(sem_name, RT_NAME_MAX, "tsem-%02d", sem_cnt ++);

    rt_sem_t sem = rt_sem_create(sem_name, 0, RT_IPC_FLAG_PRIO);
    if (sem == RT_NULL)
    {
        rt_kprintf("rt_sem_create failed!\r\n");
        rt_thread_delete(tid);
        rt_free(handle);
        return -1;
    }
    handle->tid = tid;
    handle->sem = sem;

    rt_list_insert_after(&task_list, &handle->node);
    
    return 0;
}

/****************************************************************************
 * Name: bl_os_task_delete
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_task_delete(BL_TaskHandle_t task_handle)
{
    rt_err_t ret = RT_EOK;
    rt_thread_t tid;
    struct bl_taskhandle *p = (struct bl_taskhandle *)task_handle;

    if (p == RT_NULL)
    {
        tid = rt_thread_self();
    }
    else
    {
        tid = p->tid;
        ret = rt_thread_delete(tid);
        if (ret != RT_EOK)
        {
            rt_kprintf("rt_thread_delete failed!\r\n");
        }
    }

    ret = rt_sem_delete(p->sem);
    if (ret != RT_EOK)
    {
        rt_kprintf("rt_sem_delete failed!\r\n");
    }

    rt_free(p);
}

/****************************************************************************
 * Name: bl_os_task_get_current_task
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

BL_TaskHandle_t bl_os_task_get_current_task(void)
{
    rt_thread_t tid = rt_thread_self();
    rt_list_t *list;
    struct bl_taskhandle *handle;

    for (list = (&task_list)->next; list != &task_list; list = list->next)
    {
        handle = (struct bl_taskhandle *)rt_list_entry(list, struct bl_taskhandle, node);
        if (tid == handle->tid)
        {
            return (BL_TaskHandle_t)handle;
        }
    }
    
    return NULL;
}

/****************************************************************************
 * Name: bl_os_task_notify_create
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

BL_TaskHandle_t bl_os_task_notify_create(void)
{
    return bl_os_task_get_current_task();
}

/****************************************************************************
 * Name: bl_os_task_notify
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_task_notify(BL_TaskHandle_t task_handle)
{
    RT_ASSERT(task_handle);

    rt_err_t ret = RT_EOK;
    rt_list_t *list;
    struct bl_taskhandle *handle;
    struct bl_taskhandle *p = (struct bl_taskhandle *)task_handle;
    for (list = (&task_list)->next; list != &task_list; list = list->next)
    {
        handle = (BL_TaskHandle_t)rt_list_entry(list, struct bl_taskhandle, node);
        if (p->tid == handle->tid)
        {
            ret = rt_sem_release(p->sem);
            if (ret != RT_EOK)
            {
                rt_kprintf("rt_sem_release failed! ret: %d\r\n", ret);
            }
            break;
        }
    }
}

/****************************************************************************
 * Name: bl_os_task_wait
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_task_wait(BL_TaskHandle_t task_handle, uint32_t ticks)
{
    rt_err_t ret = RT_EOK;
    rt_list_t *list;
    struct bl_taskhandle *handle;
    struct bl_taskhandle *p = (struct bl_taskhandle *)task_handle;

    rt_int32_t wait_ticks;
    if(ticks == BL_OS_WAITING_FOREVER)
        wait_ticks = RT_WAITING_FOREVER;
    else
        wait_ticks = ticks;

    for (list = (&task_list)->next; list != &task_list; list = list->next)
    {
        handle = (BL_TaskHandle_t)rt_list_entry(list, struct bl_taskhandle, node);
        if (p->tid == handle->tid)
        {
            ret = rt_sem_take(p->sem, wait_ticks);
            if (ret != RT_EOK)
            {
                rt_kprintf("rt_sem_task failed! ret: %d\r\n", ret);
            }
            break;
        }
    }
}

/****************************************************************************
 * Name: bl_os_api_init
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_api_init(void)
{
    return RT_TRUE;
}

/****************************************************************************
 * Name: bl_os_lock_gaint
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_lock_gaint(void)
{
}

/****************************************************************************
 * Name: bl_os_unlock_gaint
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_unlock_gaint(void)
{
}

/****************************************************************************
 * Name: bl_os_enter_critical
 *
 * Description:
 *   Enter critical state
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   CPU PS value
 *
 ****************************************************************************/

uint32_t bl_os_enter_critical(void)
{
    rt_enter_critical();

    return 0;
}

/****************************************************************************
 * Name: bl_os_exit_critical
 *
 * Description:
 *   Exit from critical state
 *
 * Input Parameters:
 *   level - CPU PS value
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void bl_os_exit_critical(uint32_t level)
{
    rt_exit_critical();
}

/****************************************************************************
 * Name: bl_os_msleep
 *
 * Description:
 *   Sleep in milliseconds
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_msleep(long msec)
{
    rt_thread_mdelay(msec);

    return 0;
}

/****************************************************************************
 * Name: bl_os_sleep
 *
 * Description:
 *   Sleep in seconds
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_sleep(unsigned int seconds)
{
    rt_thread_mdelay(seconds * 1000);

    return 0;
}

/****************************************************************************
 * Name: bl_os_printf
 *
 * Description:
 *   Output format string and its arguments
 *
 * Input Parameters:
 *   format - format string
 *
 * Returned Value:
 *   0
 *
 ****************************************************************************/
#if 0
void bl_os_printf(const char *__fmt, ...)
{
    va_list args;
    rt_size_t length = 0;
    static char rt_log_buf[RT_CONSOLEBUF_SIZE];
    
    if (sys_log_all_enable) {
        /* args point to the first variable parameter */
        va_start(args, __fmt);

        /* You can add your code under here. */
        // vprint(__fmt, args);
        length = rt_vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, __fmt, args);
        if (length > RT_CONSOLEBUF_SIZE - 1)
        {
            length = RT_CONSOLEBUF_SIZE - 1;
        }
        extern rt_device_t _console_device;
        if (_console_device == RT_NULL)
        {
            rt_hw_console_output(rt_log_buf);
        }
        else
        {
            rt_device_write(_console_device, 0, rt_log_buf, length);
        }
        va_end(args);
    }
}
#endif
/****************************************************************************
 * Name: bl_os_puts
 *
 * Description:
 *   Output format string
 *
 * Input Parameters:
 *   s - string
 *
 * Returned Value:
 *   0
 *
 ****************************************************************************/

void bl_os_puts(const char *s)
{
    if (s != NULL) {
        puts(s);
    }
}

/****************************************************************************
 * Name: bl_os_malloc
 *
 * Description:
 *   Allocate a block of memory
 *
 * Input Parameters:
 *   size - memory size
 *
 * Returned Value:
 *   Memory pointer
 *
 ****************************************************************************/

void *bl_os_malloc(unsigned int size)
{
    return rt_malloc(size);
}

/****************************************************************************
 * Name: bl_os_free
 *
 * Description:
 *   Free a block of memory
 *
 * Input Parameters:
 *   ptr - memory block
 *
 * Returned Value:
 *   No
 *
 ****************************************************************************/

void bl_os_free(void *ptr)
{
    rt_free(ptr);
}

/****************************************************************************
 * Name: bl_os_zalloc
 *
 * Description:
 *   Allocate a block of memory
 *
 * Input Parameters:
 *   size - memory size
 *
 * Returned Value:
 *   Memory pointer
 *
 ****************************************************************************/

void *bl_os_zalloc(unsigned int size)
{
    return rt_calloc(1, size);
}

/****************************************************************************
 * Name: bl_os_mq_creat
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

BL_MessageQueue_t bl_os_mq_creat(uint32_t queue_len, uint32_t item_size)
{
    char name[RT_NAME_MAX];
    static uint32_t mq_cnt = 0;

    memset(name, 0, RT_NAME_MAX);
    snprintf(name, RT_NAME_MAX, "mq-%02d", mq_cnt ++);

    rt_mq_t mq = rt_mq_create(name, item_size, queue_len, RT_IPC_FLAG_PRIO);
    if (mq == RT_NULL)
    {
        rt_kprintf("rt_mq_create failed!\r\n");
    }

    return mq;
}

/****************************************************************************
 * Name: bl_os_mq_delete
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_mq_delete(BL_MessageQueue_t mq)
{
    if (rt_mq_delete(mq) != RT_EOK)
    {
        rt_kprintf("rt_mq_delete failed!\r\n");
    }
}

/****************************************************************************
 * Name: bl_os_mq_send_wait
 *
 * Description:
 *   Generic send message to queue within a certain period of time
 *
 * Input Parameters:
 *   queue - Message queue data pointer
 *   item  - Message data pointer
 *   ticks - Wait ticks
 *   prio  - Message priority
 *
 * Returned Value:
 *   True if success or false if fail
 *
 ****************************************************************************/

int bl_os_mq_send_wait(BL_MessageQueue_t queue, void *item, uint32_t len, uint32_t ticks, int prio)
{
    rt_int32_t wait_ticks;

    if (ticks == BL_OS_WAITING_FOREVER)
        wait_ticks = RT_WAITING_FOREVER;
    else
        wait_ticks = ticks;

    if (rt_mq_send_wait(queue, item, len, wait_ticks) != RT_EOK)
    {
        rt_kprintf("rt_mq_send failed!\r\n");
        return 1;
    }
    return 0;
}

/****************************************************************************
 * Name: bl_os_mq_send
 *
 * Description:
 *   Send message of low priority to queue within a certain period of time
 *
 * Input Parameters:
 *   queue - Message queue data pointer
 *   item  - Message data pointer
 *   ticks - Wait ticks
 *
 * Returned Value:
 *   True if success or false if fail
 *
 ****************************************************************************/

int bl_os_mq_send(BL_MessageQueue_t queue, void *item, uint32_t len)
{
    return bl_os_mq_send_wait(queue, item, len, 0, 0);
}

/****************************************************************************
 * Name: bl_os_mq_recv
 *
 * Description:
 *   Receive message from queue within a certain period of time
 *
 * Input Parameters:
 *   queue - Message queue data pointer
 *   item  - Message data pointer
 *   ticks - Wait ticks
 *
 * Returned Value:
 *   True if success or false if fail
 *
 ****************************************************************************/

int bl_os_mq_recv(BL_MessageQueue_t queue, void *item, uint32_t len, uint32_t ticks)
{
    rt_int32_t wait_ticks;

    if (ticks == BL_OS_WAITING_FOREVER)
        wait_ticks = RT_WAITING_FOREVER;
    else
        wait_ticks = ticks;

    if (rt_mq_recv(queue, item, len, wait_ticks) < 0)
    {
        rt_kprintf("rt_mq_rev failed!\r\n");
        return 1;
    }
    return 0;
}

/****************************************************************************
 * Name: bl_os_timer_create
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

BL_Timer_t bl_os_timer_create(void *func, void *argv)
{
    char name[RT_NAME_MAX];
    static uint32_t timer_cnt = 0;

    memset(name, 0, RT_NAME_MAX);
    snprintf(name, RT_NAME_MAX, "timer-%02d", timer_cnt ++);

    rt_timer_t timer = rt_timer_create(name, func, argv, rt_tick_from_millisecond(1000), RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_HARD_TIMER);
    if (timer == RT_NULL)
    {
        rt_kprintf("rt_timer_create failed!\r\n");
    }

    return timer;
}

/****************************************************************************
 * Name: bl_os_timer_delete
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_timer_delete(BL_Timer_t timerid, uint32_t ticks)
{
    RT_ASSERT(timerid);
    rt_err_t ret = RT_EOK;

    if (timerid->parent.flag & RT_TIMER_FLAG_ACTIVATED)
    {
        ret = rt_timer_stop(timerid);
        if (ret != RT_EOK)
        {
            rt_kprintf("rt_timer_stop failed! ret:%d\r\n", ret);
        }
    }
    
    ret = rt_timer_delete(timerid);
    if (ret != RT_EOK)
    {
        rt_kprintf("rt_timer_delete failed!\r\n");
        return 1;
    }

    return 0;
}

/****************************************************************************
 * Name: bl_os_timer_start_once
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_timer_start_once(BL_Timer_t timerid, long t_sec, long t_nsec)
{
    RT_ASSERT(timerid != RT_NULL);
    rt_err_t ret = RT_EOK;
    int32_t tick;

    tick = rt_tick_from_millisecond((t_sec * 1000) + ((t_nsec > 1e6) ? (t_nsec / 1e6) : 0));
    
    ret = rt_timer_control(timerid, RT_TIMER_CTRL_SET_TIME, &tick);
    if (ret != RT_EOK)
    {
        rt_kprintf("rt_timer_control failed!\r\n");
        return 1;
    }

    ret = rt_timer_control(timerid, RT_TIMER_CTRL_SET_ONESHOT, RT_NULL);
    if (ret != RT_EOK)
    {
        rt_kprintf("rt_timer_control failed!\r\n");
        return 1;
    }

    ret = rt_timer_start(timerid);
    if (ret != RT_EOK)
    {
        rt_kprintf("rt_timer_start failed!\r\n");
        return 1;
    }

    return 0;
}

/****************************************************************************
 * Name: bl_os_timer_start_periodic
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_timer_start_periodic(BL_Timer_t timerid, long t_sec, long t_nsec)
{
    RT_ASSERT(timerid != RT_NULL);

    rt_err_t ret = RT_EOK;
    int32_t tick;

    tick = rt_tick_from_millisecond((t_sec * 1000) + ((t_nsec > 1e6) ? (t_nsec / 1e6) : 0));
    ret = rt_timer_control(timerid, RT_TIMER_CTRL_SET_TIME, &tick);
    if (ret != RT_EOK)
    {
        rt_kprintf("rt_timer_control failed!\r\n");
        return 1;
    }

    ret = rt_timer_control(timerid, RT_TIMER_CTRL_SET_PERIODIC, RT_NULL);
    if (ret != RT_EOK)
    {
        rt_kprintf("rt_timer_control failed!\r\n");
        return 1;
    }

    ret = rt_timer_start(timerid);
    if (ret != RT_EOK)
    {
        rt_kprintf("rt_timer_start failed!\r\n");
        return 1;
    }

    return 0;
}

/****************************************************************************
 * Name: bl_os_workqueue_create
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void *bl_os_workqueue_create(void)
{
    return bl_os_task_get_current_task();
}

/****************************************************************************
 * Name: bl_os_workqueue_submit_hpwork
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_workqueue_submit_hpwork(void *work, void *worker, void *argv, long tick)
{
    bl_os_task_notify(work);

    return 0;
}

/****************************************************************************
 * Name: bl_os_workqueue_submit_lpwork
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_workqueue_submit_lpwork(void *work, void *worker, void *argv, long tick)
{
    bl_os_task_notify(work);

    return 0;
}

/****************************************************************************
 * Name: bl_os_clock_gettime_ms
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

uint64_t bl_os_clock_gettime_ms(void)
{
    uint64_t ms = 0;

    ms = (long long)rt_tick_get_millisecond();
    return ms;
}

/****************************************************************************
 * Name: bl_os_get_tick
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

uint32_t bl_os_get_tick()
{
    return (uint32_t)rt_tick_get();
}

/****************************************************************************
 * Name: bl_os_isr_adpt_cb
 *
 * Description:
 *   Wi-Fi interrupt adapter callback function
 *
 * Input Parameters:
 *   arg - interrupt adapter private data
 *
 * Returned Value:
 *   0 on success
 *
 ****************************************************************************/

static void bl_os_isr_adpt_cb(int irq, void *arg)
{
    struct irq_adpt *adapter = (struct irq_adpt *)arg;

    adapter->func(adapter->arg);
}

/****************************************************************************
 * Name: bl_os_irq_attach
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_irq_attach(int32_t n, void *f, void *arg)
{
    struct irq_adpt *adapter;

    blog_info("bl_os_irq_attach irq attach: n=%ld f=%p arg=%p\n", n, f, arg);

    adapter = (struct irq_adpt *)rt_malloc(sizeof(struct irq_adpt));
    RT_ASSERT(adapter);

    memset((void *)adapter, 0, sizeof(struct irq_adpt));

    adapter->func = f;
    adapter->arg = arg;

    bflb_irq_attach(n, bl_os_isr_adpt_cb, adapter);
    bflb_irq_enable(n);	
}

/****************************************************************************
 * Name: bl_os_irq_enable
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_irq_enable(int32_t n)
{
	bflb_irq_enable(n);
}

/****************************************************************************
 * Name: bl_os_irq_disable
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_irq_disable(int32_t n)
{
	bflb_irq_disable(n);
}

/****************************************************************************
 * Name: bl_os_mutex_create
 *
 * Description:
 *   Create mutex
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   Mutex data pointer
 *
 ****************************************************************************/

BL_Mutex_t bl_os_mutex_create(void)
{
    char name[RT_NAME_MAX];
    static uint32_t mutex_cnt = 0;

    memset(name, 0, RT_NAME_MAX);
    snprintf(name, RT_NAME_MAX, "mutex-%02d", mutex_cnt ++);

    rt_mutex_t mutex = rt_mutex_create(name, RT_IPC_FLAG_PRIO);
    if (mutex == RT_NULL)
    {
        rt_kprintf("rt_mutex_create failed!\r\n");
    }

    return mutex;
}

/****************************************************************************
 * Name: bl_os_mutex_delete
 *
 * Description:
 *   Delete mutex
 *
 * Input Parameters:
 *   mutex_data - mutex data pointer
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void bl_os_mutex_delete(BL_Mutex_t mutex_data)
{
    if (rt_mutex_delete(mutex_data) != RT_EOK)
    {
        rt_kprintf("rt_mutex_delete failed!\r\n");
    }
}

/****************************************************************************
 * Name: bl_os_mutex_lock
 *
 * Description:
 *   Lock mutex
 *
 * Input Parameters:
 *   mutex_data - mutex data pointer
 *
 * Returned Value:
 *   True if success or false if fail
 *
 ****************************************************************************/

int32_t bl_os_mutex_lock(BL_Mutex_t mutex_data)
{
    rt_err_t ret = RT_EOK;
    ret = rt_mutex_take(mutex_data, RT_WAITING_FOREVER);
    if (ret != RT_EOK)
    {
        rt_kprintf("rt_mutex_take failed!\r\n");
        return 1;
    }
    
    return 0;
}

/****************************************************************************
 * Name: bl_os_mutex_unlock
 *
 * Description:
 *   Lock mutex
 *
 * Input Parameters:
 *   mutex_data - mutex data pointer
 *
 * Returned Value:
 *   True if success or false if fail
 *
 ****************************************************************************/

int32_t bl_os_mutex_unlock(BL_Mutex_t mutex_data)
{
    rt_err_t ret = RT_EOK;

    ret = rt_mutex_release(mutex_data);
    if (ret != RT_EOK)
    {
        rt_kprintf("rt_mutex_release failed!\r\n");
        return 1;        
    }

    return 0;
}

/****************************************************************************
 * Name: bl_os_sem_create
 *
 * Description:
 *   Create and initialize semaphore
 *
 * Input Parameters:
 *   max  - No mean
 *   init - semaphore initialization value
 *
 * Returned Value:
 *   Semaphore data pointer
 *
 ****************************************************************************/

BL_Sem_t bl_os_sem_create(uint32_t init)
{
    char name[RT_NAME_MAX];
    static uint32_t sem_cnt = 0;

    memset(name, 0, RT_NAME_MAX);
    snprintf(name, RT_NAME_MAX, "sem-%02d", sem_cnt ++);

    rt_sem_t sem = rt_sem_create(name, 0, RT_IPC_FLAG_PRIO);
    if (sem == RT_NULL)
    {
        rt_kprintf("rt_sem_create failed!\r\n");
    }

    return sem;

}

/****************************************************************************
 * Name: bl_os_sem_delete
 *
 * Description:
 *   Delete semaphore
 *
 * Input Parameters:
 *   semphr - Semaphore data pointer
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void bl_os_sem_delete(BL_Sem_t semphr)
{
    if (rt_sem_delete(semphr) != RT_EOK)
    {
        rt_kprintf("rt_sem_delete failed!\r\n");
    }
}

/****************************************************************************
 * Name: bl_os_sem_take
 *
 * Description:
 *   Wait semaphore within a certain period of time
 *
 * Input Parameters:
 *   semphr - Semaphore data pointer
 *   ticks  - Wait system ticks
 *
 * Returned Value:
 *   True if success or false if fail
 *
 ****************************************************************************/

int32_t bl_os_sem_take(BL_Sem_t semphr, uint32_t ticks)
{
    rt_err_t ret = RT_EOK;
    rt_int32_t wait_ticks;

    if(ticks == BL_OS_WAITING_FOREVER)
        wait_ticks = RT_WAITING_FOREVER;
    else
        wait_ticks = ticks;

    ret = rt_sem_take(semphr, wait_ticks);
    if (ret != RT_EOK)
    {
        rt_kprintf("rt_sem_take failed!\r\n");
        return 1;
    }

    return 0;
}

/****************************************************************************
 * Name: bl_os_sem_give
 *
 * Description:
 *   Post semaphore
 *
 * Input Parameters:
 *   semphr - Semaphore data pointer
 *
 * Returned Value:
 *   True if success or false if fail
 *
 ****************************************************************************/

int32_t bl_os_sem_give(BL_Sem_t semphr)
{
    rt_err_t ret = RT_EOK;

    ret = rt_sem_release(semphr);
    if (ret!= RT_EOK)
    {
        rt_kprintf("rt_sem_release failed!\r\n");
        return 1;
    }

    return 0;
}

/****************************************************************************
 * Name: bl_os_log_writev
 *
 * Description:
 *   Output log with by format string and its arguments
 *
 * Input Parameters:
 *   level  - log level, no mean here
 *   tag    - log TAG, no mean here
 *   file   - file name
 *   line   - assert line
 *   format - format string
 *   args   - arguments list
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

static void bl_os_log_writev(uint32_t level,
                             const char *tag,
                             const char *file,
                             int line,
                             const char *format,
                             va_list args)
{
#if (CFG_COMPONENT_BLOG_ENABLE == 1)
    if ((level >= REFC_LEVEL(__COMPONENT_NAME_DEQUOTED__)) &&
        (level >= REFF_LEVEL(__COMPONENT_FILE_NAME_DEQUOTED__)))
    {
        bl_os_printf("[%10u][%s: %s:%4d] ",
                     bl_os_get_tick(),
                     tag,
                     file,
                     line);

        if (sys_log_all_enable) {
            vprint(format, args);
        }
    }
#endif
}

/****************************************************************************
 * Name: bl_os_log_write
 *
 * Description:
 *   Output log with by format string and its arguments
 *
 * Input Parameters:
 *   level  - log level, no mean here
 *   file   - file name
 *   line   - assert line
 *   tag    - log TAG, no mean here
 *   format - format string
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void bl_os_log_write(uint32_t level, const char *tag, const char *file, int line, const char *format, ...)
{
    va_list list;
    va_start(list, format);

    switch (level) {
        case LOG_LEVEL_ERROR:
        {
            bl_os_log_writev(BLOG_LEVEL_ERROR, "\x1b[31mERROR \x1b[0m", file, line, format, list);
            break;
        }
        case LOG_LEVEL_WARN:
        {
            bl_os_log_writev(BLOG_LEVEL_WARN, "\x1b[33mWARN  \x1b[0m", file, line, format, list);
            break;
        }
        case LOG_LEVEL_INFO:
        {
            bl_os_log_writev(BLOG_LEVEL_INFO, "\x1b[32mINFO  \x1b[0m", file, line, format, list);
            break;
        }
        case LOG_LEVEL_DEBUG:
        {
            bl_os_log_writev(BLOG_LEVEL_DEBUG, "DEBUG ", file, line, format, list);
            break;
        }
    }

    va_end(list);
}

/****************************************************************************
 * Name: bl_os_task_notify_isr
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

int bl_os_task_notify_isr(BL_TaskHandle_t task_handle)
{
    RT_ASSERT(task_handle);

    bl_os_task_notify(task_handle);

    return 0;
}

/****************************************************************************
 * Name: bl_os_yield_from_isr
 *
 * Description:
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

void bl_os_yield_from_isr(int xYield)
{
    rt_thread_yield();
}

/****************************************************************************
 * Name: bl_os_ms_to_tick
 *
 * Description:
 *  
 * Input Parameters:
 *
 * Returned Value:
 *
 ****************************************************************************/

unsigned int bl_os_ms_to_tick(unsigned int ms)
{
    return rt_tick_from_millisecond(ms);
}
