/**
 * Copyright © 2017 INFN Torino - INDIGO-DataCloud
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "RequestQueue.h"
#include "Fass.h"
#include "PriorityManager.h"

void ReorderedQueue::request_execute(xmlrpc_c::paramList const& paramList,
                                 RequestAttributes& att) {

    PriorityManager *pm = Fass::instance().priority_manager();
    string queue = pm->get_queue();
    success_response(queue, att);
    return;
}
