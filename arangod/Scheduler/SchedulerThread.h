////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2016 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Dr. Frank Celler
/// @author Martin Schoenert
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGOD_SCHEDULER_SCHEDULER_THREAD_H
#define ARANGOD_SCHEDULER_SCHEDULER_THREAD_H 1

#include "Basics/Common.h"

#include <boost/lockfree/queue.hpp>

#include "Basics/Mutex.h"
#include "Basics/Thread.h"
#include "Scheduler/Task.h"
#include "Scheduler/TaskManager.h"

#include <deque>

namespace arangodb {
namespace rest {
class Scheduler;

/////////////////////////////////////////////////////////////////////////////
/// @brief job scheduler thread
/////////////////////////////////////////////////////////////////////////////

class SchedulerThread : public Thread, private TaskManager {
  SchedulerThread(SchedulerThread const&) = delete;
  SchedulerThread& operator=(SchedulerThread const&) = delete;

 public:
  SchedulerThread(Scheduler*, EventLoop, bool defaultLoop);
  ~SchedulerThread() { shutdown(); }

 public:
  void beginShutdown() override final;

 public:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief registers a task
  //////////////////////////////////////////////////////////////////////////////

  bool registerTask(Scheduler*, Task*);

  //////////////////////////////////////////////////////////////////////////////
  /// @brief unregisters a task
  //////////////////////////////////////////////////////////////////////////////

  void unregisterTask(Task*);

  //////////////////////////////////////////////////////////////////////////////
  /// @brief destroys a task
  //////////////////////////////////////////////////////////////////////////////

  void destroyTask(Task*);

  //////////////////////////////////////////////////////////////////////////////
  /// @brief sends data to a task
  //////////////////////////////////////////////////////////////////////////////

  void signalTask(std::unique_ptr<TaskData>&);

 protected:
  void run() override final;

  void addStatus(arangodb::velocypack::Builder* b) override final;

 private:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief what to do with the task
  //////////////////////////////////////////////////////////////////////////////

  enum work_e { INVALID, CLEANUP, DESTROY, SETUP };

  //////////////////////////////////////////////////////////////////////////////
  /// @brief what to do with the thread
  //////////////////////////////////////////////////////////////////////////////

  class Work {
   public:
    Work() : work(INVALID), scheduler(nullptr), task(nullptr) {}

    Work(work_e w, Scheduler* scheduler, Task* task)
        : work(w), scheduler(scheduler), task(task) {}

    work_e work;
    Scheduler* scheduler;
    Task* task;
  };

 private:
  //////////////////////////////////////////////////////////////////////////////
  /// @brief underlying scheduler
  //////////////////////////////////////////////////////////////////////////////

  Scheduler* _scheduler;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief if true, this is the default loop
  //////////////////////////////////////////////////////////////////////////////

  bool _defaultLoop;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief event loop
  //////////////////////////////////////////////////////////////////////////////

  EventLoop _loop;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief queue lock
  ////////////////////////////////////////////////////////////////////////////////

  Mutex _queueLock;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief work queue
  //////////////////////////////////////////////////////////////////////////////

  std::deque<Work> _queue;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief statistics
  ///
  /// This is for statistics only. Never use this number for decisions.
  //////////////////////////////////////////////////////////////////////////////

  std::atomic<int> _numberTasks;

  //////////////////////////////////////////////////////////////////////////////
  /// @brief stored task data
  //////////////////////////////////////////////////////////////////////////////

  boost::lockfree::queue<TaskData*> _taskData;
};
}
}

#endif
