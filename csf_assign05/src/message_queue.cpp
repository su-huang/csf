#include <iostream>
#include <ctime>
#include "guard.h"
#include "message_queue.h"

MessageQueue::MessageQueue() {
  pthread_mutex_init(&m_mutex, NULL); 
  sem_init(&m_sem, 0, 0);               // initialize count to 0     
}

MessageQueue::~MessageQueue() {
  pthread_mutex_destroy(&m_mutex); 
  sem_destroy(&m_sem); 
}

void MessageQueue::enqueue(std::shared_ptr<Message> msg) {
  // modifying shared queue so need critical section 
  Guard g(m_mutex); 
  
  m_queue.push_back(msg); 
  sem_post(&m_sem);     
}

std::shared_ptr<Message> MessageQueue::dequeue() {
  // initialize timespec_t value for one second in the future 
  std::timespec ts;
  std::timespec_get(&ts, TIME_UTC);
  ts.tv_sec += 1;

  // wait 1 second before acting on empty queue 
  if (sem_timedwait(&m_sem, &ts) != 0) {
    return std::shared_ptr<Message>(); 
  }

  // modifying shared queue so need critical section 
  Guard g(m_mutex); 

  // dequeue first message 
  auto msg = m_queue.front(); 
  m_queue.pop_front();
  return msg; 
}
