#include "TrafficLight.h"
#include <future>
#include <iostream>
#include <random>

/* Implementation of class "MessageQueue" */

template <typename T> T MessageQueue<T>::receive() {
  // FP.5a
  std::unique_lock<std::mutex> uLock(_mtx);
  _cond.wait(uLock, [this] { return !_queue.empty(); });
  T msg = std::move(_queue.back());
  _queue.clear(); // avoid piled up message queue for less frequented
                  // intersections
  return msg;
}

template <typename T> void MessageQueue<T>::send(T &&msg) {
  // FP.4a
  std::lock_guard<std::mutex> uLock(_mtx);
  std::cout << "   Message " << msg << " has been sent to the queue"
            << std::endl;
  _queue.push_back(std::move(msg));
  _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight() { _currentPhase = TrafficLightPhase::red; }

void TrafficLight::waitForGreen() {
  // FP.5b
  while (true) {
    auto signal = _msgQueue.receive();
    if (signal == TrafficLightPhase::green) {
      return;
    }
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase() { return _currentPhase; }

void TrafficLight::simulate() {
  // FP.2b
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {
  // FP.2a
  auto t1 = std::chrono::high_resolution_clock::now();
  // max and min for cycle duration in ms
  int const max_ms = 6000, min_ms = 4000;
  // random cycle duration between 4s and 6s
  int const cycle_ms = min_ms + rand() % (max_ms - min_ms + 1);
  while (true) {
    auto t2 = std::chrono::high_resolution_clock::now();
    double current_cycle_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    if (current_cycle_ms >= cycle_ms) {
      _currentPhase = _currentPhase == TrafficLightPhase::green
                          ? TrafficLightPhase::red
                          : TrafficLightPhase::green;
      _msgQueue.send(std::move(_currentPhase));
      // reset elapsed time in current cycle
      current_cycle_ms = 0;
      t1 = std::chrono::high_resolution_clock::now();
    }
    // prevent burning through CPU
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
