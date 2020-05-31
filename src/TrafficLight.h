#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <condition_variable>
#include <deque>
#include <mutex>

#include "TrafficObject.h"

enum TrafficLightPhase {
  red,
  green,
};

// forward declarations to avoid include cycle
class Vehicle;

template <class T>
class MessageQueue {
 public:
  void send(T &&MessageQueue);
  T receive();

 private:
  std::deque<T> _queue;
  std::condition_variable _condition_msgQ;
  std::mutex _msgQmutex;
};

class TrafficLight : TrafficObject {
 public:
  // constructor / desctructor
  TrafficLight();
  // getters / setters
  TrafficLightPhase getCurrentPhase();
  // typical behaviour methods
  void waitForGreen();
  void simulate();

 private:
  void cycleThroughPhases();
  // typical behaviour methods
  TrafficLightPhase _currentPhase;
  std::condition_variable _condition;
  std::mutex _mutex;
  MessageQueue<TrafficLightPhase> _msgQ;
};

#endif