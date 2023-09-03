#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>

using namespace std;

vector<string> instruments{"Rose", "Lavender", "Lotus", "Tulip", "Orchid"};//instruments' type 
int ord_id = 0;
string reason = "";

//current time calculation
string currentTime(){
    auto currentTime = chrono::system_clock::now();
    auto timepoint = chrono::system_clock::to_time_t(currentTime);
    tm* timeinfo = localtime(&timepoint);

    // Calculate the milliseconds part
    int milliseconds = chrono::duration_cast<chrono::milliseconds>(currentTime.time_since_epoch()).count() % 1000;

    // Reuse the ostringstream for efficiency
    static ostringstream oss;
    oss.str(""); // clear the stream
    oss.clear(); // clear any error flags

    oss << put_time(timeinfo, "%Y%m%d-%H%M%S") << "." << setfill('0') << setw(3) << milliseconds;
    return oss.str();
}

//function to find order id
string find_Id(vector<vector<string>>& exe_rep, string& id){
    for(int i =0; i<exe_rep.size(); i++){
        if(exe_rep[i][1] == id){
            return exe_rep[i][0];//if the order is already in the execution report
        }
    }
    return "0";//if the order is not in the execution report
}

//function to add an order to a exexution report
vector<vector<string>> add_exe_rep_line(vector<string>& order,vector<vector<string>>& exe_rep){
    string ord_id1 = find_Id(exe_rep, order[0]);
    if(ord_id1 == "0"){
        ord_id++;
        ord_id1 = "ord"+to_string(ord_id);
    }
    //enter details to execution report
    exe_rep.push_back({ord_id1, order[0], order[1], order[2], "New", order[3], order[4],reason, ""});//creating structure of the execution report
    return exe_rep;
}

//function to check validity of an order
bool isValid(vector<string>& order,string& reason){
    if(order[2] == "1" or order[2] == "2"){//order type                                                                                         
        if(stoi(order[4])>0){//price >0
            if(stoi(order[3])>10 && stoi(order[3])<1000 && stoi(order[3])%10 == 0){//quantity max = 1000 , min=10 and 10x
                for (string value:instruments){//checking for valid instruments
                    if(order[1] == value){
                        if(order[0] != ""){
                            reason = "";
                            return true;}
                        reason = "no Client ID.";
                        return false;
                    }
                }
                reason = "Invalid instrumment type";
                return false;
            }
            reason = "Invalid quantity";
            return false;
        }
        reason = "Invalid price";
        return false;
    }
    reason = "Invalid order type";
    return false;
}

//function to execute the order
void execution(vector<string>& order,vector<vector<string>>& exe_rep, vector<vector<string>>& sell_ord,vector<vector<string>>& buy_ord ){
    string time;
    //if the order is buy order
    if(order[2] == "1"){
        //if the sell order is already there it will start execution, if the sell order price is lower than the buy order price
        if((sell_ord.size() != 0) && (stoi(sell_ord[sell_ord.size()-1][4]) <= stoi(order[4]))){
            if(stoi(sell_ord[sell_ord.size()-1][3]) == stoi(order[3])){//quantities are equal
                exe_rep[exe_rep.size()-1][4] ="Fill";
                add_exe_rep_line(sell_ord[sell_ord.size()-1], exe_rep); 
                exe_rep[exe_rep.size()-1][4] ="Fill";
                exe_rep[exe_rep.size()-1][6] =order[4];
                time = currentTime();
                exe_rep[exe_rep.size()-1][8] = time; 
                exe_rep[exe_rep.size()-2][8] = time;                    
                sell_ord.pop_back();
            }
            else{
                if(stoi(sell_ord[sell_ord.size()-1][3]) < stoi(order[3])){//sell order quantity is lower than new buy order quantity
                    exe_rep[exe_rep.size()-1][4] ="Pfill";
                    exe_rep[exe_rep.size()-1][5] = sell_ord[sell_ord.size()-1][3];
                    add_exe_rep_line(sell_ord[sell_ord.size()-1], exe_rep); 
                    exe_rep[exe_rep.size()-1][4] ="Fill";
                    exe_rep[exe_rep.size()-1][6] =order[4];
                    time = currentTime();
                    exe_rep[exe_rep.size()-1][8] = time; 
                    exe_rep[exe_rep.size()-2][8] = time; 
                    buy_ord.push_back(order);
                    buy_ord[buy_ord.size()-1][3] = to_string(stoi(order[3]) - stoi(sell_ord[sell_ord.size()-1][3]));
                    sell_ord.pop_back();     

                    //execution for remaining quantity of buy order
                    while(buy_ord.size()!= 0 && sell_ord.size() != 0){
                        if(stoi(buy_ord[buy_ord.size()-1][4]) < stoi(sell_ord[sell_ord.size()-1][4])){//if the buy order price is lower than sell order size there is no execution
                            break;
                        }

                        add_exe_rep_line(buy_ord[buy_ord.size()-1], exe_rep);
                        cout<<"buy order list\n";
                        if(stoi(buy_ord[buy_ord.size()-1][3]) == stoi(sell_ord[sell_ord.size()-1][3])){//for equal quantities
                            exe_rep[exe_rep.size()-1][4] ="Fill";
                            add_exe_rep_line(sell_ord[sell_ord.size()-1], exe_rep);
                            exe_rep[exe_rep.size()-1][4] ="Fill";
                            exe_rep[exe_rep.size()-1][6] =buy_ord[buy_ord.size()-1][4];  
                            time = currentTime();
                            exe_rep[exe_rep.size()-1][8] = time; 
                            exe_rep[exe_rep.size()-2][8] = time;                  
                            sell_ord.pop_back(); 
                            buy_ord.pop_back(); 
                            break;
                        }
                        else{
                            if(stoi(sell_ord[sell_ord.size()-1][3]) < stoi(buy_ord[buy_ord.size()-1][3])){//if the sell order quantity is lower
                                exe_rep[exe_rep.size()-1][4] ="Pfill";
                                exe_rep[exe_rep.size()-1][5] = sell_ord[sell_ord.size()-1][3];
                                add_exe_rep_line(sell_ord[sell_ord.size()-1], exe_rep); 
                                exe_rep[exe_rep.size()-1][4] ="Fill";
                                exe_rep[exe_rep.size()-1][6] =buy_ord[buy_ord.size()-1][4];
                                time = currentTime();
                                exe_rep[exe_rep.size()-1][8] = time; 
                                exe_rep[exe_rep.size()-2][8] = time;       
                                buy_ord[buy_ord.size()-1][3] = to_string(stoi(buy_ord[buy_ord.size()-1][3]) - stoi(sell_ord[sell_ord.size()-1][3]));
                                sell_ord.pop_back(); 
                                }
                            else{//if the buy order quantity is lower
                                exe_rep[exe_rep.size()-1][4] ="Fill";
                                add_exe_rep_line(sell_ord[sell_ord.size()-1], exe_rep); 
                                exe_rep[exe_rep.size()-1][4] ="Pfill";
                                exe_rep[exe_rep.size()-1][6] =buy_ord[buy_ord.size()-1][4];
                                exe_rep[exe_rep.size()-1][5] =buy_ord[buy_ord.size()-1][3];
                                time = currentTime();
                                exe_rep[exe_rep.size()-1][8] = time; 
                                exe_rep[exe_rep.size()-2][8] = time;       
                                sell_ord[sell_ord.size()-1][3] = to_string(stoi(sell_ord[sell_ord.size()-1][3]) - stoi(buy_ord[buy_ord.size()-1][3]));
                                buy_ord.pop_back();
                                break;
                            }
                        }
                    }
                }
                else{
                    exe_rep[exe_rep.size()-1][4] ="Fill";//if the buy order quantity is lower
                    add_exe_rep_line(sell_ord[sell_ord.size()-1], exe_rep); 
                    exe_rep[exe_rep.size()-1][4] ="Pfill";
                    exe_rep[exe_rep.size()-1][6] =order[4];
                    exe_rep[exe_rep.size()-1][5] =order[3];
                    time = currentTime();
                    exe_rep[exe_rep.size()-1][8] = time; 
                    exe_rep[exe_rep.size()-2][8] = time;       
                    sell_ord[sell_ord.size()-1][3] = to_string(stoi(sell_ord[sell_ord.size()-1][3]) - stoi(order[3]));    
                }
            }
        }
        //if the sell order is empty or cannot have execution         
        else{
            exe_rep[exe_rep.size()-1][8] = currentTime();
            buy_ord.push_back(order);
            //sorting in increasing order after adding buy order to the buy order list
            sort(buy_ord.begin(), buy_ord.end(), [](const vector<string>& a, const vector<string>& b) {
                if (a[4] != b[4]) {
                    return (stoi(a[4]) < stoi(b[4]));
                } else {
                    return (stoi(a[0].substr(2)) > stoi(b[0].substr(2)));
                }
            });   
        }
  

    }
    //if the order is a sell order
    else if(order[2] == "2")
    {
        if(buy_ord.size() != 0 && (stoi(buy_ord[buy_ord.size()-1][4]) >= stoi(order[4]))){//buy order list is not empty,it will start the execution 
            if(stoi(buy_ord[buy_ord.size()-1][3]) == stoi(order[3])){//for equal quantities

                exe_rep[exe_rep.size()-1][4] ="Fill";
                exe_rep[exe_rep.size()-1][6] = buy_ord[buy_ord.size()-1][4];
                add_exe_rep_line(buy_ord[buy_ord.size()-1], exe_rep);
                exe_rep[exe_rep.size()-1][4] ="Fill";
                time = currentTime();
                exe_rep[exe_rep.size()-1][8] = time; 
                exe_rep[exe_rep.size()-2][8] = time;       
                buy_ord.pop_back();      
            }
            else{
                if(stoi(buy_ord[buy_ord.size()-1][3]) < stoi(order[3])){//buy order quantity is lower than sell order quantity
                        
                    exe_rep[exe_rep.size()-1][4] ="Pfill";
                    exe_rep[exe_rep.size()-1][5] = buy_ord[(buy_ord.size()-1)][3];
                    exe_rep[exe_rep.size()-1][6] = buy_ord[buy_ord.size()-1][4];
                    add_exe_rep_line(buy_ord[buy_ord.size()-1], exe_rep);
                    exe_rep[exe_rep.size()-1][4] ="Fill";
                    time = currentTime();
                    exe_rep[exe_rep.size()-1][8] = time; 
                    exe_rep[exe_rep.size()-2][8] = time;       
                    sell_ord.push_back(order);
                    sell_ord[sell_ord.size()-1][3] = to_string(stoi(order[3]) - stoi(buy_ord[(buy_ord.size()-1)][3]));
                    buy_ord.pop_back();

                    //execution for remaining quantity of the sell order list
                    while(buy_ord.size()!= 0 && sell_ord.size() != 0){
                        if(stoi(buy_ord[buy_ord.size()-1][4]) < stoi(sell_ord[sell_ord.size()-1][4])){//if the buy order price is lower than sell order size there is no execution
                            break;
                        }
                        add_exe_rep_line(sell_ord[sell_ord.size()-1], exe_rep);
                        if(stoi(sell_ord[sell_ord.size()-1][3]) == stoi(buy_ord[buy_ord.size()-1][3])){//for equal quantities
                            exe_rep[exe_rep.size()-1][4] ="Fill";
                            exe_rep[exe_rep.size()-1][6] = buy_ord[buy_ord.size()-1][4];
                            add_exe_rep_line(buy_ord[buy_ord.size()-1], exe_rep);
                            exe_rep[exe_rep.size()-1][4] ="Fill";
                            time = currentTime();
                            exe_rep[exe_rep.size()-1][8] = time; 
                            exe_rep[exe_rep.size()-2][8] = time;                        
                            sell_ord.pop_back(); 
                            buy_ord.pop_back(); 
                            break;
                        }
                        else{
                            if(stoi(buy_ord[buy_ord.size()-1][3]) < stoi(sell_ord[sell_ord.size()-1][3])){//if the buy order quantity is higher
                                exe_rep[exe_rep.size()-1][4] ="Pfill";
                                exe_rep[exe_rep.size()-1][5] =buy_ord[buy_ord.size()-1][3];
                                exe_rep[exe_rep.size()-1][6] = buy_ord[buy_ord.size()-1][4];
                                add_exe_rep_line(buy_ord[buy_ord.size()-1], exe_rep);
                                exe_rep[exe_rep.size()-1][4] ="Fill";
                                time = currentTime();
                                exe_rep[exe_rep.size()-1][8] = time; 
                                exe_rep[exe_rep.size()-2][8] = time;       
                                sell_ord[sell_ord.size()-1][3] = to_string(stoi(sell_ord[sell_ord.size()-1][3]) - stoi(buy_ord[buy_ord.size()-1][3]));
                                buy_ord.pop_back(); 
                                }
                            else{//if the buy order quantity is higher
                                exe_rep[exe_rep.size()-1][4] ="Fill";
                                exe_rep[exe_rep.size()-1][6] = buy_ord[buy_ord.size()-1][4];
                                add_exe_rep_line(buy_ord[buy_ord.size()-1], exe_rep);
                                exe_rep[exe_rep.size()-1][4] ="Pfill";
                                exe_rep[exe_rep.size()-1][5] =sell_ord[sell_ord.size()-1][3];
                                time = currentTime();
                                exe_rep[exe_rep.size()-1][8] = time; 
                                exe_rep[exe_rep.size()-2][8] = time;       
                                buy_ord[buy_ord.size()-1][3] = to_string(stoi(buy_ord[buy_ord.size()-1][3]) - stoi(sell_ord[sell_ord.size()-1][3]));
                                sell_ord.pop_back();
                                break;
                            }
                        }
                    }
                }
                else{
                    exe_rep[exe_rep.size()-1][4] ="Fill";//if the buy order quantity is lower
                    exe_rep[exe_rep.size()-1][6] = buy_ord[buy_ord.size()-1][4];
                    add_exe_rep_line(buy_ord[buy_ord.size()-1], exe_rep); 
                    exe_rep[exe_rep.size()-1][4] ="Pfill";
                    exe_rep[exe_rep.size()-1][5] =order[3];
                    time = currentTime();
                    exe_rep[exe_rep.size()-1][8] = time; 
                    exe_rep[exe_rep.size()-2][8] = time;       
                    buy_ord[buy_ord.size()-1][3] = to_string(stoi(buy_ord[buy_ord.size()-1][3]) - stoi(order[3]));   
                }
            }
        }
        else{//if the buy order list is empty
            exe_rep[exe_rep.size()-1][8] = currentTime();
            sell_ord.push_back(order);
            //sorting the sell order list in decreasing oreder after storing 
            sort(sell_ord.begin(), sell_ord.end(), [](const vector<string>& a, const vector<string>& b) {
                if (a[4] != b[4]) {
                    return (stoi(a[4]) > stoi(b[4]));
                } else {
                    return (stoi(a[0].substr(2)) > stoi(b[0].substr(2)));
                }
            });     
        }  
    }
    else{
        cout<<"Invalid order type\n";
    }
}
int main() {
    vector<vector<string >> orders;//order list initialization
    vector<string> order(5);//a one order in the list
    string line, word;

    fstream file("Book5.csv", ios::in);
    ofstream myfile;

    // Open the .csv file of the orders
    if (file.is_open()) {
        while (getline(file, line)) {
            order.clear();
            stringstream str(line);
            while (getline(str, word, ','))
                order.push_back(word);
            orders.push_back(order);
        }
    } 
    //if the file can not be found
    else {
        cout << "Could not open the file\n";
    }

    auto start_intm = chrono::high_resolution_clock::now();// start time calculation
    vector<vector<string >> exe_rep;//execution report initialization

    //create 5 order books with sell and buy sides
    vector<vector<string >> Rose_buy;//for Rose
    vector<vector<string >> Rose_sell;

    vector<vector<string >> Lavender_buy;//for Lavender
    vector<vector<string >> Lavender_sell;

    vector<vector<string >> Lotus_buy;//for Lotus
    vector<vector<string >> Lotus_sell;

    vector<vector<string >> Tulip_buy;//for Tulip
    vector<vector<string >> Tulip_sell;

    vector<vector<string >> Orchid_buy;//for Orchid
    vector<vector<string >> Orchid_sell;
    bool validity;
    
    for (int i = 2; i < orders.size(); i++) {//starting execution from the first order
        validity = isValid(orders[i], reason);
        exe_rep = add_exe_rep_line(orders[i], exe_rep);//add new order to the execution report
        
        if(not(validity)){//checking validity of the order
            exe_rep[exe_rep.size()-1][4] = "Rejected";
            exe_rep[exe_rep.size()-1][8] = currentTime();
            continue;
        }
        reason = ""; 
        
        if(orders[i][1] == "Rose"){//if the order was not rejected then doing selection of orderbook and execution
            execution(orders[i], exe_rep, Rose_sell, Rose_buy);
        }
        else{
            if(orders[i][1] == "Lavender"){
                execution(orders[i], exe_rep, Lavender_sell, Lavender_buy);
            } 
            else{
                if(orders[i][1] == "Lotus") {
                    execution(orders[i], exe_rep, Lotus_sell, Lotus_buy);
                }
                else{
                    if(orders[i][1] == "Tulip"){
                        execution(orders[i], exe_rep, Tulip_sell, Tulip_buy);
                    }
                    else{
                        execution(orders[i], exe_rep, Orchid_sell, Orchid_buy);
                    }
                }
            }
        }
    }
    auto end_intm = chrono::high_resolution_clock::now();// end time calculation
    // Create final execution report
    myfile.open("execution_rep_1.csv");
    myfile << "execution_rep.csv\n";
    myfile << "Order ID, Client Order, Instrument, Side, Exec Status, Quantity, Price, Reason, Transaction time\n";

    // Write to the file
    for (int i = 0; i < exe_rep.size(); i++) {
        for (int j = 0; j < exe_rep[i].size(); j++) {
            myfile << exe_rep[i][j] << ",";
        }
        myfile << "\n";
    }
    auto diff_intm = chrono::duration_cast<chrono::microseconds>(end_intm - start_intm).count();//time calculation
    myfile<<"\nTransaction Time to the fully execution of the file: "<<diff_intm<<" microseconds.";
    myfile.close();//closing
    
    //Printing order.csv
    for (int i = 0; i < orders.size(); i++) {
        for (int j = 0; j < orders[i].size(); j++) {
            cout << orders[i][j] << " ";
        }
        cout << "\n";
    }
    cout << "\n";
    //printing execution_report.csv 
    for (int i = 0; i < exe_rep.size(); i++) {
        for (int j = 0; j < exe_rep[i].size(); j++) {
            cout<< exe_rep[i][j] << ",";
        }
        cout<< "\n";
    }
    cout<<"\nTransaction Time to the fully execution of the file: "<<diff_intm<<" microseconds.";//printing transaction times
    return 0;
}