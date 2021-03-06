#include "TrafficLight.h"

#include <future>
#include <iostream>
#include <random>
/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive() {
  // receive new messages and pull them from the queue

  std::unique_lock<std::mutex> receive_lock(_msgQmutex);
  _condition_msgQ.wait(receive_lock, [this] { return !_queue.empty(); });
  T msg = std::move(_queue.front());
  _queue.pop_front();

  return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg) {


  // add a new message to the queue and afterwards send a notification.
  std::lock_guard<std::mutex> send_lock(_msgQmutex);
  _queue.push_back(std::move(msg));
  _condition_msgQ.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight() { _currentPhase = TrafficLightPhase::red; }

void TrafficLight::waitForGreen() {
  while (true) {
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    if (_msgQ.receive() == TrafficLightPhase::green) return;
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase() { return _currentPhase; }

void TrafficLight::simulate() {

  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {


  std::random_device rd;
  std::mt19937 random_engine(rd());
  std::uniform_real_distribution<> distribution(4000, 6000);
  double cycleDuration = distribution(random_engine);

  // init stop watch
  std::chrono::time_point<std::chrono::system_clock> lastUpdate =
      std::chrono::system_clock::now();
  while (true) {
      // to prevent 100% cpu usage
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    long timeSinceLastUpdate =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - lastUpdate)
            .count();
    if (timeSinceLastUpdate >= cycleDuration) {
      std::lock_guard lock(_mutex);
      // switch traffic light phase
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
