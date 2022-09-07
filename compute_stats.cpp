/*
created by Andrey Kan
andrey.kan@adelaide.edu.au
2021
*/
#include <iostream>
#include <fstream>
#include <vector>


class Customer
{
public:
    int priority;
    int arrival_time;
    int slots_remaining; // how many time slots are still needed
    int response_time;

    Customer(int par_priority, int par_arrival_time, int par_slots_remaining)
    {
        priority = par_priority;
        arrival_time = par_arrival_time;
        slots_remaining = par_slots_remaining;
        response_time = -1;
    }
};

class Stats
{
    int total_wait_0; // for high priority customers
    int total_wait_1; // for regular priority customers
    int total_wait; // for all customers
    int longest_response; // max response time across all customers
    int n_switches;

public:
    Stats();
    bool compute_scheduling_stats(std::ifstream& results_file, std::vector<Customer> &customers);
    void print();
};

void read_customer_info(std::ifstream &in_file, std::vector<Customer> &customers)
{
    std::string name;
    int priority, arrival_time, slots_requested;

    // read input file line by line
    // https://stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c
    int customer_id = 0;
    while (in_file >> name >> priority >> arrival_time >> slots_requested)
    {
        // structure initialization
        // https://en.cppreference.com/w/c/language/struct_initialization
        Customer customer_from_file(priority, arrival_time, slots_requested);
        customers.push_back(customer_from_file);
        customer_id++;
    }
}

Stats::Stats()
{
    total_wait_0 = -1;
    total_wait_1 = -1;
    total_wait = -1;
    longest_response = -1;
    n_switches = -1;
}

// https://www.w3schools.com/cpp/cpp_function_reference.asp
bool Stats::compute_scheduling_stats(std::ifstream& results_file, std::vector<Customer> &customers)
{
    int num_switches = -1; // how many times customers were switched
    int previous_id = -2;
    int max_response = 0; // to keep track of the longest reponse time

    int total_wait_time_0 = 0; // wait time for all priority 0 customers
    int total_wait_time_1 = 0; // wait time for all priority 1 customers

    // priority 0 customers that are currently either playing or waiting
    int playing_or_waiting_0 = 0;
    // priority 1 customers that are currently either playing or waiting
    int playing_or_waiting_1 = 0;

    int last_arrived_id = -1; // most recently arrived customer

    int reference_time = 0;
    int current_time, customer_id;
    while (results_file >> current_time >> customer_id)
    {
        if (current_time != reference_time)
        {
            std::cerr << "Times are incorrect." << std::endl;
            return false;
        }
        while (last_arrived_id + 1 < customers.size())
        {
            // if not all customers have arrived yet
            // identify newly arriving customers at this time slot;
            // the code relies on customers appearing sequentially in the input;
            if (customers[last_arrived_id + 1].arrival_time == current_time)
            {
                // new customer has arrived
                last_arrived_id++;
                if (customers[last_arrived_id].priority == 0)
                {
                    playing_or_waiting_0++;
                }
                else
                {
                    playing_or_waiting_1++;
                }
                // note that more than one customer can arrive at the same time,
                // so check again
                continue;
            }
            break;
        }
        if (customer_id == -1)
        {
            // the machine is not used, increase total wait time
            // for everyone who has arrived but hasn't left yet
            total_wait_time_0 += playing_or_waiting_0;
            total_wait_time_1 += playing_or_waiting_1;
        }
        else
        {
            // some customer is playing
            if ((customer_id < 0) || (customer_id >= customers.size()))
            {
                std::cerr << "Unknown customer." << std::endl;
                return false;
            }
            if (customer_id > last_arrived_id)
            {
                // this relies on customer IDs appearing sequentially in the input
                std::cerr << customer_id << ": scheduled too early." << std::endl;
                return false;
            }

            // everyone who arrived and is not playing is waiting
            total_wait_time_0 += playing_or_waiting_0;
            total_wait_time_1 += playing_or_waiting_1;
            if (customers[customer_id].priority == 0)
            {
                total_wait_time_0--;
            }
            else
            {
                total_wait_time_1--;
            }

            if (customers[customer_id].response_time == -1)
            {
                // this is the first time this customer is playing
                int response_time = current_time - customers[customer_id].arrival_time;
                customers[customer_id].response_time = response_time;
                if (response_time > max_response)
                {
                    max_response = response_time;
                }
            }

            customers[customer_id].slots_remaining--;
            if (customers[customer_id].slots_remaining < 0)
            {
                std::cerr << customer_id << ": redundant runs." << std::endl;
                return false;
            }
            else if (customers[customer_id].slots_remaining == 0)
            {
                // this customer has been satisfied and will be leaving
                if (customers[customer_id].priority == 0)
                {
                    playing_or_waiting_0--;
                }
                else
                {
                    playing_or_waiting_1--;
                }
            }
        }
        if (previous_id != customer_id)
        {
            num_switches++;
            previous_id = customer_id;
        }
        reference_time++;
    }
    if (reference_time == 0)
    {
        std::cerr << "Empty output." << std::endl;
        return false;
    }
    if (playing_or_waiting_0 + playing_or_waiting_1 > 0)
    {
        std::cerr << "Some customers were not satisfied." << std::endl;
        return false;
    }
    if (customer_id != -1)
    {
        std::cerr << "Last line should have no customers (-1 as id)." << std::endl;
        return false;
    }
    total_wait_0 = total_wait_time_0;
    total_wait_1 = total_wait_time_1;
    total_wait = (total_wait_time_0 + total_wait_time_1);
    longest_response = max_response;
    n_switches = num_switches;

    return true;
}

void Stats::print()
{
    std::cout << "total_wait_0 total_wait_1 total_wait ";
    std::cout << "longest_response ";
    std::cout << "n_switches " << std::endl;
    std::cout << total_wait_0 << " ";
    std::cout << total_wait_1 << " ";
    std::cout << total_wait << " ";
    std::cout << longest_response << " ";
    std::cout << n_switches << std::endl;
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
    std::ifstream data_file(argv[1]);
    std::ifstream results_file(argv[2]);
    if ((!data_file) || (!results_file))
    {
        std::cerr << "Cannot open files." << std::endl;
        return -1;
    }

    // vector: https://www.geeksforgeeks.org/vector-in-cpp-stl/
    std::vector<Customer> customers; // information about each customer
    read_customer_info(data_file, customers);

    Stats stats;
    if (!stats.compute_scheduling_stats(results_file, customers))
    {
        std::cerr << "INVALID scheduling" << std::endl;
        return -1;
    }
    stats.print();

    return 0;
}
