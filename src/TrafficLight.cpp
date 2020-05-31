#include "TrafficLight.h"

#include <future>
#include <iostream>
#include <random>
/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive() {
  // FP.5a : The method receive should use std::unique_lock<std::mutex> and
  // to wait for and receive new messages and pull them from the queue using
  // move semantics. The received object should then be returned by the receive
  // function.
  std::unique_lock<std::mutex> receive_lock(_msgQmutex);
  _condition_msgQ.wait(receive_lock, [this] { return !_queue.empty(); });
  T msg = std::move(_queue.front());
  _queue.pop_front();

  return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg) {
  // FP.4a : The method send should use the mechanisms
  // std::lock_guard<std::mutex>
  // as well as _condition.notify_one() to add a new message to the queue and
  // afterwards send a notification.
  std::lock_guard<std::mutex> send_lock(_msgQmutex);
  _queue.push_back(std::move(msg));
  _condition_msgQ.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight() { _currentPhase = TrafficLightPhase::red; }

void TrafficLight::waitForGreen() {
  // FP.5b : add the implementation of the method waitForGreen, in which an
  // infinite while-loop runs and repeatedly calls the receive function on the
  // message queue. Once it receives TrafficLightPhase::green, the method
  // returns.
  while (true) {
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    if (_msgQ.receive() == TrafficLightPhase::green) return;
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase() { return _currentPhase; }

void TrafficLight::simulate() {
  // FP.2b : Finally, the private method „cycleThroughPhases“ should bestarted
  // in a thread when the public method „simulate“ is called.To do this, use the
  // thread queue in the base class.
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

  std::random_device rd;
  std::mt19937 random_engine(rd());
  std::uniform_real_distribution<> distribution(4000, 6000);
  double cycleDuration = distribution(random_engine);

  // init stop watch
  std::chrono::time_point<std::chrono::system_clock> lastUpdate =
      std::chrono::system_clock::now();
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    long timeSinceLastUpdate =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - lastUpdate)
            .count();
    if (timeSinceLastUpdate >= cycleDuration) {
      std::lock_guard lock(_mutex);
      _currentPhase = _currentPhase == TrafficLightPhase::red
                          ? TrafficLightPhase::green
                          : TrafficLightPhase::red;

      TrafficLightPhase msg = _currentPhase;
      _msgQ.send(std::move(msg));

      lastUpdate = std::chrono::system_clock::now();
      cycleDuration = distribution(random_engine);
    }
  }
}
