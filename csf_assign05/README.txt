README for Assignment 5

Names of team members, contributions: Su Huang, shuan148@jh.edu (Implemented all functions)

Synchronization report (for Milestone 2):

Shared data
- m_orders_map: map storing all active orders and access by order_new, item_update, order_update 
- m_next_order_id: counter that is incremented to assign unique order IDs 
- m_display_clients_set: set of Client pointers, modified by add_display_client and remove_display_client, read by broadcast

Synchronization strategy 
- Used a single pthread_mutex_t in the instance of the Server class to manage access to all shared data listed above 
- Used Guard class to lock mutex upon construction and automatically unlock it when the scope is exited 
- Every method that modifies m_orders_map is wrapped in a Guard 
- broacast method is called while m_mutex is already being held so that it can safely iterate over m_display_clients_set
- add_display_client and remove_display_client also use m_muted so the m_display_clients_set is consistent across states 

Preventing deadlocks 
- Server uses a single global mutex m_mutex 
- Never attempt to acquire a second mutex while holding the first so there's no circular waiting 
- order_update_internal avoids the above and is only called by methods that are already locking mutex 

Thread-safe communication 
- MessageQueue handles communication between threads 
- Uses an internal pthread_mutex_t and sem_t to separate thread-safety of MessageQueue and m_orders_map
- Ensures that blocking the queue doesn't block other order threads from accessing the server state 