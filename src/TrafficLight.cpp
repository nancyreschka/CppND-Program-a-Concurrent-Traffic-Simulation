#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    
    // perform vector modification under the lock
    std::unique_lock<std::mutex> lock(_mutex);
    _cond.wait(lock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable

    // remove first vector element from queue
    T msg = std::move(_queue.front());
    _queue.pop_front();

    // return the received message
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    
    // perform vector modification under the lock
    std::lock_guard<std::mutex> lock(_mutex);

    // add vector to queue
    _queue.push_back(std::move(msg));
    std::cout << "   Message " << msg << " has been sent to the queue" << std::endl;
    _cond.notify_one(); // notify client after pushing new TrafficLight into vector
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight()
{    
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    TrafficLightPhase receivedPhase;
    
    while (true)
    {
        receivedPhase = _trafficLightPhaseQueue.receive();
        if(receivedPhase == TrafficLightPhase::green)
            break;
    }    
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    std::unique_lock<std::mutex> lock(_mutex);
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method
    // „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

double TrafficLight::getCycleDuration()
{
    // generate random number between 4000 and 6000 ms
    double randomNumber = (rand() % (6000 - 4000 + 1)) + 4000;
    return randomNumber; 
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.

    std::unique_lock<std::mutex> lock(_mutex);
    std::cout << "TrafficLight #" << _id << "::cycleThroughPhases: thread id = " << std::this_thread::get_id() << std::endl;
    lock.unlock();
    
    // duration of a single simulation cycle is between 4 and 6 seconds in ms
    double cycleDuration = getCycleDuration();
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;

    // init stop watch
    lastUpdate = std::chrono::system_clock::now();
    while(true)
    {
        // wait 1ms between two cycles
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();

        if(timeSinceLastUpdate >= cycleDuration)
        {
            lock.lock();
            // toggle traffic light between red and green
            if(_currentPhase == TrafficLightPhase::green) _currentPhase = TrafficLightPhase::red;
            else if(_currentPhase == TrafficLightPhase::red) _currentPhase = TrafficLightPhase::green;
            
            // send update message to queue
            TrafficLightPhase newPhase = _currentPhase;
            _trafficLightPhaseQueue.send(std::move(newPhase));
            lock.unlock();

            // reset stop watch for next cycle
            lastUpdate = std::chrono::system_clock::now();
            // update cycle duration for next cycle
            cycleDuration = getCycleDuration();
        }
    }
}