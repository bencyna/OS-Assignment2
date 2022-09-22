// a1766116, Cyna, Benjamin
// a1768686, Kumar, Sharath
// a1809221, Zhou, Minghan
// a1808020, Zhang, Xingyuan 
// group 4

#include <iostream>
#include <fstream>
#include <deque>
#include <vector>
#include <functional>
#include <queue>

// std is a namespace: https://www.cplusplus.com/doc/oldtutorial/namespaces/
int TIME_ALLOWANCE = 10;  // allow to use up to this number of time slots at once
const int PRINT_LOG = 0; // print detailed execution trace

class Customer
{
public:
    std::string name;
    int priority;
    int arrival_time;
    int slots_remaining; // how many time slots are still needed
    int playing_since;

    Customer(std::string par_name, int par_priority, int par_arrival_time, int par_slots_remaining)
    {
        name = par_name;
        priority = par_priority;
        arrival_time = par_arrival_time;
        slots_remaining = par_slots_remaining;
        playing_since = -1;
    }
};

class Event
{
public:
    int event_time;
    int customer_id;  // each event involes exactly one customer
    int priority;

    Event(int par_event_time, int par_customer_id, int _priority)
    {
        event_time = par_event_time;
        customer_id = par_customer_id;
        priority = _priority;
    }
};

void initialize_system(
    std::ifstream &in_file,
    std::deque<Event> &arrival_events,
    std::vector<Customer> &customers)
{
    std::string name;
    int priority, arrival_time, slots_requested;

    // read input file line by line
    // https://stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c
    int customer_id = 0;
    while (in_file >> name >> priority >> arrival_time >> slots_requested)
    {
        Customer customer_from_file(name, priority, arrival_time, slots_requested);
        customers.push_back(customer_from_file);

        // new customer arrival event
        Event arrival_event(arrival_time, customer_id, priority);
        arrival_events.push_back(arrival_event);

        customer_id++;
    }
}

void print_state(
    std::ofstream &out_file,
    int current_time,
    int current_id,
    const std::deque<Event> &arrival_events,
    const std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> &p0_queue,
    const std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> &p1_queue)
{
    out_file << current_time << " " << current_id << '\n';
    if (PRINT_LOG == 0)
    {
        return;
    }
    std::cout << current_time << ", " << current_id << '\n';
    for (int i = 0; i < arrival_events.size(); i++)
    {
        std::cout << "\t" << arrival_events[i].event_time << ", " << arrival_events[i].customer_id << ", ";
    }
    std::cout << '\n';
    // for (int i = 0; i < p0_queue.size(); i++)
    // {
    //     std::cout << "\t" << p0_queue[i] << ", ";
    // }
    // std::cout << '\n';
    // for (int i = 0; i < p1_queue.size(); i++)
    // {
    //     std::cout << "\t" << p1_queue[i] << ", ";
    // }
    std::cout << '\n';
}

// process command line arguments
// https://www.geeksforgeeks.org/command-line-arguments-in-c-cpp/
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Provide input and output file names." << std::endl;
        return -1;
    }
    std::ifstream in_file(argv[1]);
    std::ofstream out_file(argv[2]);
    if ((!in_file) || (!out_file))
    {
        std::cerr << "Cannot open one of the files." << std::endl;
        return -1;
    }

    // deque: https://www.geeksforgeeks.org/deque-cpp-stl/
    // vector: https://www.geeksforgeeks.org/vector-in-cpp-stl/
    std::deque<Event> arrival_events; // new customer arrivals
    std::vector<Customer> customers; // information about each customer

    // read information from file, initialize events queue
    initialize_system(in_file, arrival_events, customers);

    int current_id = -1; // who is using the machine now, -1 means nobody
    int time_out = -1; // time when current customer will be preempted
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> init_queue; // waiting queue
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> p0_queue; // waiting queue
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> p1_queue; // waiting queue
    int priority_switch = 0;


    // step by step simulation of each time slot
    bool all_done = false;
    for (int current_time = 0; !all_done; current_time++)
    {
        priority_switch++;

        // welcome newly arrived customers
        while (!arrival_events.empty() && (current_time == arrival_events[0].event_time))
        {
            init_queue.push({customers[arrival_events[0].customer_id].slots_remaining, arrival_events[0].customer_id});
            arrival_events.pop_front();
        }
        // check if we need to take a customer off the machine
        if (current_id >= 0)
        {
            if (current_time == time_out)
            {
                int last_run = current_time - customers[current_id].playing_since;
                customers[current_id].slots_remaining -= last_run;
                if (customers[current_id].slots_remaining > 0)
                {
                    // customer is not done yet, waiting for the next chance to play
                    if(customers[current_id].priority == 0){
                        p0_queue.push({customers[current_id].slots_remaining, current_id});
                    }else{
                        p1_queue.push({customers[current_id].slots_remaining, current_id});
                    }

                }
                current_id = -1; // the machine is free now
            }
        }
        // if machine is empty, schedule a new customer
        if (current_id == -1)
        {
            if (!init_queue.empty()) // is anyone waiting?
            {
                //first = slots_remaining
                //second  = current_id
                auto a = init_queue.top();
                current_id = a.second;
                init_queue.pop();
                TIME_ALLOWANCE = 3;
                if (TIME_ALLOWANCE > customers[current_id].slots_remaining)
                {
                    time_out = current_time + customers[current_id].slots_remaining;
                }
                else
                {
                    time_out = current_time + TIME_ALLOWANCE;
                }
                customers[current_id].playing_since = current_time;
            } else if (!p0_queue.empty() && (priority_switch % 4) != 0) // is anyone waiting?
            {
                //first = slots_remaining
                //second  = current_id
                auto a = p0_queue.top();
                current_id = a.second;
                p0_queue.pop();
                TIME_ALLOWANCE = 12;

                if (TIME_ALLOWANCE > customers[current_id].slots_remaining)
                {
                    time_out = current_time + customers[current_id].slots_remaining;
                }
                else
                {
                    time_out = current_time + TIME_ALLOWANCE;
                }
                customers[current_id].playing_since = current_time;
            }else if(!p1_queue.empty()) // is anyone waiting?
            {
                //first = slots_remaining
                //second  = current_id
                auto a = p1_queue.top();
                current_id = a.second;
                p1_queue.pop();
                TIME_ALLOWANCE = 12;

                if (TIME_ALLOWANCE > customers[current_id].slots_remaining)
                {
                    time_out = current_time + customers[current_id].slots_remaining;
                }
                else
                {
                    time_out = current_time + TIME_ALLOWANCE;
                }
                customers[current_id].playing_since = current_time;
            }
            else if (!p0_queue.empty()) // is anyone waiting?
            {
                //first = slots_remaining
                //second  = current_id
                auto a = p0_queue.top();
                current_id = a.second;
                p0_queue.pop();
                TIME_ALLOWANCE = 12;

                if (TIME_ALLOWANCE > customers[current_id].slots_remaining)
                {
                    time_out = current_time + customers[current_id].slots_remaining;
                }
                else
                {
                    time_out = current_time + TIME_ALLOWANCE;
                }
                customers[current_id].playing_since = current_time;
            }
        }
        print_state(out_file, current_time, current_id, arrival_events, p0_queue, p1_queue);

        // exit loop when there are no new arrivals, no waiting and no playing customers
        all_done = (arrival_events.empty() && p0_queue.empty() && p1_queue.empty() && (current_id == -1));
    }

    return 0;
}
