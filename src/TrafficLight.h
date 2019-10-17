#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Vehicle;


template <class T>
class MessageQueue
{
public:
    // send messages via a queue
    void send(T &&msg);
    // receive messages from queue
    T receive();

private:
    std::mutex _mutex;
    std::condition_variable _cond;
    // message queue that stores message objects
    std::deque<T> _queue;
};

enum TrafficLightPhase
{
    red = 0, 
    green = 1
};

class TrafficLight : TrafficObject
{
public:
    // constructor / desctructor
    TrafficLight();

    ~TrafficLight();

    // getters / setters
    TrafficLightPhase getCurrentPhase();
    
    // typical behaviour methods
    void waitForGreen();
    void simulate();

private:
    // typical behaviour methods
    // change traffic light phase
    void cycleThroughPhases();
    // generates a random cycle duration between 4 and 6 second
    // number is returned in ms
    double getCycleDuration();
    
    std::condition_variable _condition;
    std::mutex _mutex;
    // traffic light phases that are exchanged with another thread
    MessageQueue<TrafficLightPhase> _trafficLightPhaseQueue;

    // states the current traffic light phase
    TrafficLightPhase _currentPhase;    
};

#endif