#ifndef HIGH_PRECISION_TIMER_H
#define HIGH_PRECISION_TIMER_H

#ifdef _WIN32
#include <Windows.h>
#else
#include <time.h>
#endif

// Cross platform high precision timer (Windows / Unix)
class High_Precision_Timer
{
#ifdef _WIN32
  HANDLE timer_event; // Event handle for synchronization

  // Static callback for multimedia timer
  static void CALLBACK timer_proc(UINT u_timer_id, UINT u_msg, DWORD_PTR dw_user, DWORD_PTR dw1, DWORD_PTR dw2)
  {
    // Access the event handle through the instance pointer
    High_Precision_Timer* instance = reinterpret_cast<High_Precision_Timer*>(dw_user);
    SetEvent(instance->timer_event); // Signal the event to unblock nano_sleep
  }
#endif

public:
  // High-resolution sleep function without busy-waiting
#ifdef _WIN32
  void sleep(DWORD milliseconds);
#else
  void sleep(long milliseconds);
#endif

#ifdef _WIN32
  High_Precision_Timer();
  ~High_Precision_Timer();
#endif
};

#ifdef _WIN32
// High-resolution sleep function without busy-waiting
inline void High_Precision_Timer::sleep(DWORD milliseconds)
{
  if (!this->timer_event)
  {
    return;
  }

  // Reset the event
  ResetEvent(this->timer_event);

  // Start a one-shot timer with the specified delay
  MMRESULT timer_id = timeSetEvent(
    milliseconds, 1, timer_proc, reinterpret_cast<DWORD_PTR>(this), TIME_ONESHOT
  );

  if (timer_id == 0)
  {
    return;
  }

  // Wait for the timer event to be signaled
  WaitForSingleObject(this->timer_event, INFINITE);

  // Clean up the timer
  timeKillEvent(timer_id);
}

#else // Unix
inline void High_Precision_Timer::sleep(long milliseconds)
{
  struct timespec req, rem;
  req.tv_sec = milliseconds / 1000;                // Convert milliseconds to seconds
  req.tv_nsec = (milliseconds % 1000) * 1'000'000; // Convert remaining milliseconds to nanoseconds
  nanosleep(&req, &rem);
}
#endif

#ifdef _WIN32
inline High_Precision_Timer::High_Precision_Timer()
{
  // Set timer resolution to 1 ms for high accuracy
  timeBeginPeriod(1);

  // Create an event for synchronization
  this->timer_event = CreateEvent(NULL, FALSE, FALSE, NULL); // Auto-reset event

  if (!this->timer_event)
  {
  }
}

inline High_Precision_Timer::~High_Precision_Timer()
{
  // Close the event handle and restore the timer resolution
  if (this->timer_event)
  {
    CloseHandle(this->timer_event);
  }

  timeEndPeriod(1);
}
#endif

#endif // HIGH_PRECISION_TIMER_H
