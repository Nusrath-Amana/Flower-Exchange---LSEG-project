#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include<chrono>
#include <map>
#include <iomanip> // Include for std::put_time and std::setfill

using namespace std;

struct Order
{
    string cl_ord_id;
    string ord_id;
    string instrument;
    int side;
    int quantity;
    int price;
    string status;
    string reason;
    string trans_time;
    chrono::time_point<chrono::system_clock> timestamp;


};

string current_time();
void writeCSV(const Order &order);

int temp =0;
bool repeat= false;

// Separate orders into two tables based on side (1 and 2)
map<string, vector<Order>> sell_orders_map;
map<string, vector<Order>> buy_orders_map;


vector<Order> orderUpdates;


// Custom comparator function for sorting the Order structs
bool compareOrders(const Order &order1, const Order &order2)
{
    if (order1.side == 1)
    {
        return order1.price > order2.price; // Sort by Price in decending order
    }
    else
    {
        return order1.price < order2.price; // Sort by Price in ascending order
    }
}

void exchange_buy(Order &order, const string &instrument)
{

    if (buy_orders_map[instrument].size() == 0 || order.price > buy_orders_map[instrument][0].price) // check whether rose_buy table is empty
    {
        // place sell order in descending order
        sell_orders_map[instrument].push_back(order);
        sort(sell_orders_map[instrument].begin(), sell_orders_map[instrument].end(), [](const Order &order1, const Order &order2) {
                 return order1.price < order2.price;
             });
        
        if (!repeat){
            order.status="new";
            orderUpdates.push_back(order);

        }
       

    }

    else
    {
        int quantity_diff = order.quantity - buy_orders_map[instrument][0].quantity;
        if (quantity_diff == 0)
        {
            order.status="fill";
            order.price=buy_orders_map[instrument][0].price;
      
            orderUpdates.push_back(order);

            buy_orders_map[instrument][0].status="fill";
            orderUpdates.push_back(buy_orders_map[instrument][0]);

            buy_orders_map[instrument].erase(buy_orders_map[instrument].begin()); // erace the first row of rose_buy table
        }

        else if (quantity_diff > 0)
        {
            order.status="pfill";
            temp=order.price;
            order.price=buy_orders_map[instrument][0].price;
            order.quantity=buy_orders_map[instrument][0].quantity;
         
            orderUpdates.push_back(order);
            order.price=temp;

            buy_orders_map[instrument][0].status="fill";
            orderUpdates.push_back(buy_orders_map[instrument][0]);
            buy_orders_map[instrument].erase(buy_orders_map[instrument].begin()); // erace the first row of rose_buy table
            order.quantity = quantity_diff;
            repeat=true;
            exchange_buy(order,order.instrument);
            
        }
        else
        {
            order.status="fill";
            order.price=buy_orders_map[instrument][0].price;
            orderUpdates.push_back(order);

            buy_orders_map[instrument][0].status="pfill";
            buy_orders_map[instrument][0].quantity = order.quantity;
            orderUpdates.push_back(buy_orders_map[instrument][0]);
            buy_orders_map[instrument][0].quantity = (abs(quantity_diff));
            
        }
    }

    return;
}

void exchange_sell(Order &order, const string &instrument)
{

    if (sell_orders_map[instrument].size() == 0 || order.price < sell_orders_map[instrument][0].price) // check whether rose_buy table is empty
    {
        buy_orders_map[instrument].push_back(order);
        sort(buy_orders_map[instrument].begin(), buy_orders_map[instrument].end(), [](const Order &order1, const Order &order2) {
                 return order1.price > order2.price;
             });
        if (!repeat){
        order.status="new";
        orderUpdates.push_back(order);
        }

    }

    else 
    {
        
        int quantity_diff = order.quantity - sell_orders_map[instrument][0].quantity;
        if (quantity_diff == 0)
        {
            order.status="fill";
            order.price=sell_orders_map[instrument][0].price;
         
            orderUpdates.push_back(order);

            sell_orders_map[instrument][0].status="fill";
      
            orderUpdates.push_back(sell_orders_map[instrument][0]);
            sell_orders_map[instrument].erase(sell_orders_map[instrument].begin()); // erace the first row of rose_buy table
            
        }
        else if (quantity_diff > 0)
        {
            sell_orders_map[instrument][0].status="fill";
            orderUpdates.push_back(sell_orders_map[instrument][0]);

            order.status="pfill";
            order.quantity=sell_orders_map[instrument][0].quantity;
            temp=order.price;
            order.price=sell_orders_map[instrument][0].price;
          
            orderUpdates.push_back(order);

            order.price=temp;
            sell_orders_map[instrument].erase(sell_orders_map[instrument].begin()); // erace the first row of rose_buy table
            order.quantity = quantity_diff;
            repeat=true;

            exchange_sell(order,order.instrument);
            
        }
        else
        {
            order.status="fill";
            order.price=sell_orders_map[instrument][0].price;
           
            orderUpdates.push_back(order);

            sell_orders_map[instrument][0].status="pfill";
            sell_orders_map[instrument][0].quantity = order.quantity;
        
            orderUpdates.push_back(sell_orders_map[instrument][0]);

            sell_orders_map[instrument][0].quantity = (abs(quantity_diff));
            
        }
    }
    
    return;
}


vector<Order> readCSV(const std::string &filename)
{
    vector<Order> orders;
    ifstream file(filename);

    if (!file)
    {
        std::cerr << "Error: Unable to open file: " << filename << std::endl;
        return orders;
    }

    std::string line;
    while (std::getline(file, line))
    {
        // Skip over empty lines
        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::string field;
        Order new_order;

        // Read the fields one by one and handle any errors
        if (!std::getline(ss, new_order.cl_ord_id, ','))
        {
            std::cerr << "Error: Unable to read Cl.Ord.ID" << std::endl;
            continue;
        }
        if (!std::getline(ss, new_order.instrument, ','))
        {
            std::cerr << "Error: Unable to read Instrument" << std::endl;
            continue;
        }
        if (!(ss >> new_order.side))
        {
            std::cerr << "Error: Unable to read Side" << std::endl;
            continue;
        }
        ss.ignore(1); // Skip comma
        if (!(ss >> new_order.quantity))
        {
            std::cerr << "Error: Unable to read Quantity" << std::endl;
            continue;
        }
        ss.ignore(1); // Skip comma
        if (!(ss >> new_order.price))
        {
            std::cerr << "Error: Unable to read Price" << std::endl;
            continue;
        }

        orders.push_back(new_order);
    }

    file.close();
    return orders;
}


// Function to write sorted orders back to CSV file
void writeCSV(const Order &order) {
    const std::string filename = "exchange_sub.csv";
     ifstream fileCheck(filename);
    bool isFileEmpty = fileCheck.peek() == ifstream::traits_type::eof();

    ofstream file(filename, ios::app);

    if (!file) {
        cerr << "Error: Unable to open file: " << filename << endl;
        return;
    }
    if (isFileEmpty)
    {
        // File is empty, so write the title row
        file << "Ord.ID,Cl.Ord.ID,Instrument,Side,Status,Quantity,Price,Reason,Trans.Time\n";
    }

    file << order.ord_id << ","<< order.cl_ord_id << "," << order.instrument << ","
         << order.side << ","<< order.status <<","  << order.quantity << ","
         << order.price << "," << order.reason <<"," <<current_time()<<"\n";

    file.close();
}

string current_time()
{
    auto now = std::chrono::system_clock::now();
    auto time_c = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000;

    std::tm tm = *std::localtime(&time_c);

    std::stringstream formatted_time;
    formatted_time << std::put_time(&tm, "%Y/%m/%d-%H:%M:%S")
                   << '.' << std::setfill('0') << std::setw(3) << ms.count()
                   << '.' << std::setfill('0') << std::setw(3) << us.count();

    std::string formatted_time_str = formatted_time.str();

    return formatted_time_str;
}


bool isValidOrder(Order &order) {
    // Check if all required fields are present
    if (order.cl_ord_id.empty() ) {
        order.reason ="invalid cl_order id";
    }

    // Check if the instrument is valid (modify with your list of valid instruments)
    else if (order.instrument != "Rose" && order.instrument != "Lavender" && order.instrument != "Lotus"
        && order.instrument != "Tulip" && order.instrument != "Orchid") {
        order.reason="invalid instrument";
    }

    // Check if side is valid (1 or 2)
    else if (order.side != 1 && order.side != 2) {
        order.reason="invalid side";
    }

    // Check if price is greater than 0
    else if (order.price <= 0) {
        order.reason="invalid price";
    }

    // Check if quantity is a multiple of 10  , quantity is within the range (min = 10, max = 1000)
    else if (order.quantity % 10 != 0 || order.quantity < 10 || order.quantity > 1000 ) {
        order.reason="invalid quantity";
    }

    // All checks passed, the order is valid
    else{
        order.reason=" ";
        order.status=" ";
        return true;
    }
    
    order.status="Reject";
    orderUpdates.push_back(order);
    return false;
}


int main()
{
    
    ofstream file("exchange_sub.csv",ios::trunc); 
    
    const std::string filename = "Book2.csv";

    // Read the CSV file and populate orders vector
    std::vector<Order> orders = readCSV(filename);
    int i=1;
    auto start_intm = chrono::high_resolution_clock::now();// start time calculation
    for (auto &order : orders)
    {   
        order.ord_id="ord"+ to_string(i);
        if (isValidOrder(order)) 
        {   
            repeat= false;

            if (order.side == 1)    
            {
                order.timestamp = chrono::system_clock::now(); // Record order timestamp
                exchange_sell(order,order.instrument);
            }
            else
            {
                order.timestamp = chrono::system_clock::now(); // Record order timestamp
                exchange_buy(order,order.instrument);
            }
            
        }
        i++;
    }

    for (const auto &order : orderUpdates) {
    writeCSV(order);
    }
    auto end_intm = chrono::high_resolution_clock::now();// start time calculation
    auto diff_intm = chrono::duration_cast<chrono::microseconds>(end_intm - start_intm).count();

    cout<<"\nTransaction Time only for the execution: "<<diff_intm<<" microseconds.";//printing transaction times

    return 0;
}
