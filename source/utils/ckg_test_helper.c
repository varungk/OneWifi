/************************************************************************************
  If not stated otherwise in this file or this component's LICENSE file the
  following copyright and licenses apply:

  Copyright 2018 RDK Management

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 **************************************************************************/

#include <stdio.h>
#include <string.h>
#include "scheduler.h"
#include "vap_svc.h"

/* ─── Helper: user-defined callback for scheduler timer ─── */
static int ckg_timer_callback(void *arg)
{
    (void)arg;
    printf("ckg timer fired\n");
    return 0;
}

/* ─── Helper: alternate callback for parameter-forwarded bind ─── */
static int ckg_alt_callback(void *arg)
{
    (void)arg;
    return 1;
}

/* ─── Helper: accepts a fn-ptr param and binds it to struct field ─── */
static void ckg_bind_via_param(vap_svc_t *svc, vap_svc_event_fn_t handler)
{
    svc->event_fn = handler;
}

/* ─── Helper: dispatches via start_fn and stop_fn struct fn-ptr fields ─── */
static void ckg_dispatch_start_stop(vap_svc_t *svc)
{
    svc->start_fn(svc, 0, NULL);
    svc->stop_fn(svc, 0, NULL);
}

/* ─── Main test function exercising all CKG edge types ─── */
int ckg_test_edges(struct scheduler *sched, vap_svc_t *svc)
{
    int id = 0;

    /* CALLS (internal) + PASSES_CALLBACK (argument form) */
    scheduler_add_timer_task(sched, false, &id,
                             ckg_timer_callback,
                             NULL, 5000, 1, true);

    /* CALLS_EXTERNAL: libc memset */
    memset(&id, 0, sizeof(id));

    /* PASSES_CALLBACK (argument form, second instance) */
    scheduler_add_timer_task(sched, true, &id,
                             ckg_alt_callback,
                             NULL, 1000, 0, false);

    /* BINDS_CALLBACK (direct): assign fn-ptr to struct field */
    svc->start_fn = vap_svc_private_start;
    svc->stop_fn  = vap_svc_private_stop;

    /* DISPATCHES_VIA: call through start_fn and stop_fn via helper */
    ckg_dispatch_start_stop(svc);

    /* BINDS_CALLBACK (parameter-forwarded): fn-ptr passed through param */
    ckg_bind_via_param(svc, vap_svc_private_event);

    /* DISPATCHES_VIA: call through struct fn-ptr field */
    svc->update_fn(svc, 0, NULL, NULL);

    /* PASSES_CALLBACK (cast form) */
    void (*generic_fn)(void) = (void (*)(void))ckg_timer_callback;
    generic_fn();

    return 0;
}
