#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include "TrafficObject.h"
#include <condition_variable>
#include <deque>
#include <mutex>

// forward declarations to avoid include cycle
class Vehicle;

// FP.3
template <class T> class MessageQueue {
public:
  void send(T &&msg);
  T receive();

private:
  std::deque<T> _queue;
  std::condition_variable _cond;
  std::mutex _mtx;
};

// FP.1
enum TrafficLightPhase { red, green };

class TrafficLight : public TrafficObject {
public:
  // constructor / desctructor
  TrafficLight();

  // getters / setters
  TrafficLightPhase getCurrentPhase();

  // typical behaviour methods
  void waitForGreen();
  void simulate();

private:
  // typical behaviour methods
  void cycleThroughPhases();

  // FP.4b
  MessageQueue<TrafficLightPhase> _msgQueue;
  TrafficLightPhase _currentPhase;
};

#endif