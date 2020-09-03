//
//  http_scheduler.c
//  http
//
//  Created by JunYing Yeh on 2014/7/24.
//
//

#include "http_loc.h"

static HTTP_Worker_T * http_scheduler_get_nice_worker();

void http_scheduler_new_connection(HTTP_Connection_T *http_connection)
{
    HTTP_Worker_T *worker;

    ASSERT(http_connection != NULL);

    worker = http_scheduler_get_nice_worker();

    http_worker_ctrl(worker, HTTP_WORKER_NEW_CONNECTION, http_connection);
}

static HTTP_Worker_T * http_scheduler_get_nice_worker()
{
    HTTP_Worker_T       *ret = NULL;

#if (HTTP_CFG_TOTAL_WORKER == 1)
    {
        ret = HTTP_OM_GetWorkerByIndex(0);
    }
#elif (HTTP_CFG_TOTAL_WORKER == 2)
    {
        ret = HTTP_OM_GetWorkerByIndex(1);
        if (ret->kind == HTTP_WORKER_MASTER)
        {
            ret = HTTP_OM_GetWorkerByIndex(0);
        }
    }
#else
    {
        int i;
        HTTP_Worker_T   *min_conn_worker = NULL;

        for (i = 0; i < HTTP_CFG_TOTAL_WORKER; ++ i)
        {
            HTTP_Worker_T *temp_worker = HTTP_OM_GetWorkerByIndex(i);

            if (min_conn_worker == NULL)
            {
                if (temp_worker->kind != HTTP_WORKER_MASTER)
                {
                    min_conn_worker = temp_worker;
                }
            }
            else
            {
                if (temp_worker->kind != HTTP_WORKER_MASTER &&
                    temp_worker->connections.counter < min_conn_worker->connections.counter)
                {
                    min_conn_worker = temp_worker;
                }
            }
        }

        if (min_conn_worker)
        {
            ret = min_conn_worker;
        }
        else
        {
            ret = HTTP_OM_GetMasterWorker();
        }
    }
#endif

    ASSERT(ret != NULL);
    return ret;
}