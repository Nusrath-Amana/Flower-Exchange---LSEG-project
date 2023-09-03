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
    double price;
    string status;
    string reason;
    string trans_time;
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
bool compareOrders_buy(const Order &order1, const Order &order2)
{
    return order1.price > order2.price; // Sort by Price in decending order
}

bool compareOrders_sell(const Order &order1, const Order &order2)
{
    return order1.price < order2.price; // Sort by Price in ascending order
}

void exchange_buy(Order &order, const string &instrument)
{
    // check whether buy table is empty or sell price is hiher than buy price
    if (buy_orders_map[instrument].size() == 0 || order.price > buy_orders_map[instrument][0].price) 
    {
        sell_orders_map[instrument].push_back(order);
        sort(sell_orders_map[instrument].begin(), sell_orders_map[instrument].end(), compareOrders_sell);   //sort in descending order
        
        if (!repeat){   //new order
            order.status="New";
            orderUpdates.push_back(order);
        }
    }

    else
    {
        int quantity_diff = order.quantity - buy_orders_map[instrument][0].quantity;

        //if buy and sell quantity are same
        if (quantity_diff == 0)
        {
            order.status="Fill";
            order.price=buy_orders_map[instrument][0].price;
            orderUpdates.push_back(order);

            buy_orders_map[instrument][0].status="Fill";
            orderUpdates.push_back(buy_orders_map[instrument][0]);

            buy_orders_map[instrument].erase(buy_orders_map[instrument].begin()); // erace the first row of buy table
        }

        //if sell quantity is higher than buy quantity;
        else if (quantity_diff > 0)
        {
            order.status="Pfill";
            temp=order.price;
            order.price=buy_orders_map[instrument][0].price;    //sold price
            order.quantity=buy_orders_map[instrument][0].quantity;
            orderUpdates.push_back(order);

            buy_orders_map[instrument][0].status="Fill";
            orderUpdates.push_back(buy_orders_map[instrument][0]);

            buy_orders_map[instrument].erase(buy_orders_map[instrument].begin()); // erace the first row of buy table
            
            order.price=temp;   //selling price
            order.quantity = quantity_diff; //remaining quantity
            repeat=true;
            exchange_buy(order,order.instrument);  
        }

        //if sell quantity is lesser than buy quantity;
        else
        {
            order.status="Fill";
            order.price=buy_orders_map[instrument][0].price;
            orderUpdates.push_back(order);

            buy_orders_map[instrument][0].status="Pfill";
            buy_orders_map[instrument][0].quantity = order.quantity;
            orderUpdates.push_back(buy_orders_map[instrument][0]);

            buy_orders_map[instrument][0].quantity = (abs(quantity_diff));  //remaining quantity
        }
    }
    return;
}

void exchange_sell(Order &order, const string &instrument)
{
    // check whether sell table is empty or sell price is hiher than buy price
    if (sell_orders_map[instrument].size() == 0 || order.price < sell_orders_map[instrument][0].price) 
    {
        buy_orders_map[instrument].push_back(order);
        sort(buy_orders_map[instrument].begin(), buy_orders_map[instrument].end(), compareOrders_buy);  //sort in descending order
     
        if (!repeat){   //new order
            order.status="New";
            orderUpdates.push_back(order);
        }
    }

    else 
    {
        int quantity_diff = order.quantity - sell_orders_map[instrument][0].quantity;

        //if buy and sell quantity are same
        if (quantity_diff == 0)
        {
            order.status="Fill";
            order.price=sell_orders_map[instrument][0].price;
            orderUpdates.push_back(order);
        
            sell_orders_map[instrument][0].status="Fill";
            orderUpdates.push_back(sell_orders_map[instrument][0]);
            sell_orders_map[instrument].erase(sell_orders_map[instrument].begin()); // erace the first row of sell table
        }

        //if buy quantity is higher than sell quantity;
        else if (quantity_diff > 0)
        {
            temp=order.price;
            order.price=sell_orders_map[instrument][0].price;   //sold price
            order.status="Pfill";
            order.quantity=sell_orders_map[instrument][0].quantity;
            orderUpdates.push_back(order);
            
            sell_orders_map[instrument][0].status="Fill";
            orderUpdates.push_back(sell_orders_map[instrument][0]);

            sell_orders_map[instrument].erase(sell_orders_map[instrument].begin()); // erace the first row of sell table
            
            order.price=temp;   //orginal selling price
            order.quantity = quantity_diff; //remaining quantity
            repeat=true;
            exchange_sell(order,order.instrument);
            
        }

        //if buy quantity is lesser than sell quantity;
        else
        {
            order.status="Fill";
            order.price=sell_orders_map[instrument][0].price;
            orderUpdates.push_back(order);

            sell_orders_map[instrument][0].status="Pfill";
            sell_orders_map[instrument][0].quantity = order.quantity;
            orderUpdates.push_back(sell_orders_map[instrument][0]);

            sell_orders_map[instrument][0].quantity = (abs(quantity_diff)); //remaining quantity
        }
    }
    return;
}

string current_time()
{
    auto now = std::chrono::system_clock::now();
    auto time_c = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    //auto us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000;

    std::tm tm = *std::localtime(&time_c);

    std::stringstream formatted_time;
    formatted_time << std::put_time(&tm, "%Y/%m/%d-%H:%M:%S")
                   << '.' << std::setfill('0') << std::setw(3) << ms.count();

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


vector<Order> readCSV(const std::string &filename)
{
    vector<Order> orders;
    ifstream file(filename);

    if (!file)
    {
        std::cerr << "Error: Unable to open file: " << filename << std::endl;
        return orders;
    }

    bool isFirstLine= true;
    std::string line;
    while (std::getline(file, line))
    {

        // Skip over empty lines
        if (line.empty())
            continue;

        // Skip the first line (header)
        if (isFirstLine)
        {
            isFirstLine = false;
            continue;
        }

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
        if (ss.peek() == ',')
        {
            new_order.side = 0;
        }
        else if (!(ss >> new_order.side))
        {
            std::cerr << "Error: Unable to read Side" << std::endl;
            continue;
        }

        ss.ignore(1); // Skip comma
        if (ss.peek() == ',')
        {
            new_order.quantity = 0;
        }
        else if (!(ss >> new_order.quantity))
        {
            std::cerr << "Error: Unable to read quantity" << std::endl;
            continue;
        }
        ss.ignore(1); // Skip comma
        if (!(ss >> new_order.price))
        {
            new_order.price=0;
        }

        orders.push_back(new_order);
    }

    file.close();
    return orders;
}



int main()
{
    const string input_filename = "Book2.csv";
    const string output_filename = "execution_rep.csv";

    // Read the CSV file 
    std::vector<Order> orders = readCSV(input_filename);

    int i=1;
    auto start_intm = chrono::high_resolution_clock::now(); // start time calculation
    for (auto &order : orders)
    {   
        order.ord_id="ord"+ to_string(i);
        order.trans_time = current_time();

        if (isValidOrder(order)) 
        {   
            repeat= false;

            if (order.side == 1)    
            {
                exchange_sell(order,order.instrument);
            }
            else
            {
                exchange_buy(order,order.instrument);
            }
            
        }
        i++;
    }

    ifstream fileCheck(output_filename);
    bool isFileEmpty = fileCheck.peek() == ifstream::traits_type::eof();

    std::ofstream file(output_filename, std::ios::trunc); 

    if (!file) {
        cerr << "Error: Unable to open file: " << output_filename << endl;
        return 0;
    }
    
    // write the title row
    file << "Ord.ID,Cl.Ord.ID,Instrument,Side,Status,Quantity,Price,Reason,Trans.Time\n";

    for (const auto &order : orderUpdates) {
        file << order.ord_id << ","<< order.cl_ord_id << "," << order.instrument << ","
            << order.side << ","<< order.status <<","  << order.quantity << ","
            << std::fixed << std::setprecision(2) << order.price << "," 
            << order.reason <<"," << order.trans_time<<"\n";
    }

    file.close();


    auto end_intm = chrono::high_resolution_clock::now(); // end time calculation
    auto diff_intm = chrono::duration_cast<chrono::microseconds>(end_intm - start_intm).count();

    cout << "\nTime only for the execution: " << diff_intm << " microseconds." << endl; // printing transaction times

    return 0;
}
