#include "TrafficLight.h"
#include <future>
#include <iostream>
#include <random>

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait()
    // to wait for and receive new messages and pull them from the queue using move semantics.
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> uLock(_mtx);
    _cond.wait(uLock);
    T msg = std::move(_queue.back());
    _queue.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex>
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> uLock(_mtx);
    std::cout << "   Message " << msg << " has been sent to the queue" << std::endl;
    _queue.push_back(std::move(msg));
    _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight() { _currentPhase = TrafficLightPhase::red; }

void TrafficLight::waitForGreen() {
  // FP.5b : add the implementation of the method waitForGreen, in which an
  // infinite while-loop runs and repeatedly calls the receive function on the
  // message queue. Once it receives TrafficLightPhase::green, the method
  // returns.
  while (true)
  {
      if(_msgQueue.receive()==TrafficLightPhase::green) {
          break;
      }
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase() { return _currentPhase; }

void TrafficLight::simulate() {
  // FP.2b : Finally, the private method „cycleThroughPhases“ should be started
  // in a thread when the public method „simulate“ is called. To do this, use
  // the thread queue in the base class.
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {
  // FP.2a : Implement the function with an infinite loop that measures the time
  // between two loop cycles and toggles the current phase of the traffic light
  // between red and green and sends an update method to the message queue using
  // move semantics. The cycle duration should be a random value between 4 and 6
  // seconds. Also, the while-loop should use std::this_thread::sleep_for to
  // wait 1ms between two cycles.
  std::chrono::duration<double> elapsed_time{0};
  auto t1 = std::chrono::system_clock::now();
  int const max_duration = 6,
            min_duration = 4; // max and min duration for cycle duration in s
  double cycle_duration =
      min_duration +
      (double)rand() / ((double)(RAND_MAX / (max_duration - min_duration)));
  while (true) {
    auto t2 = std::chrono::system_clock::now();
    elapsed_time += t2 - t1;
    if (elapsed_time.count() >= cycle_duration) {
      TrafficLightPhase signal = getCurrentPhase() == TrafficLightPhase::green
                                     ? TrafficLightPhase::red
                                     : TrafficLightPhase::green;
      _msgQueue.send(std::move(signal));
      elapsed_time = std::chrono::duration<double>(
          0); // reset elapsed time in current cycle
      cycle_duration =
          min_duration +
          (double)rand() /
              ((double)(RAND_MAX /
                        (max_duration -
                         min_duration))); // determine new random cycle duration
                                          // between 4 and 6s
    }
    std::this_thread::sleep_for(
        std::chrono::milliseconds(1)); // prevent burning through CPU
    t1 = std::chrono::system_clock::now();
  }
}
