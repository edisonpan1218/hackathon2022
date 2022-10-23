#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <queue>
#include <set>
using namespace std;
int times;
queue<double> q;
double Max;
double max(double a, double b){
    if(a >= b) return a;
    return b;
}
int abs(int x){
    if(x < 0) return -x;
    return x;
}
struct data{
    string ID;
    double price, size, size_in_quote;
    string trade_time;
    bool Can_buy;
    bool ignore;
};

struct item_data{
    double price, size;
};

multiset<item_data> item;

bool operator<(item_data a, item_data b){
    return a.price < b.price;
}

vector<data> trade;
double profit;
double money;

void buy(int index){
    money -= trade[index].price * trade[index].size;
    times ++;
}

void sell(int index, int Size){
    money += Size * trade[index].price;
}

double size(int index){
    return (double)(trade[index].size);
}

double price(int index){
    return (double)(trade[index].price);
}

void get_data(int m){
    fstream file;
    file.open("BTCUSDT-trades-2022-10-19.csv");
    string line;
    int loc = 1;
    while(getline(file, line) && loc <= m){
        string S = "";
        istringstream tmp(line);
        string s;
        while(getline(tmp, s, ',')){
            S += s;
            S += " ";
        }
        cout << S << '\n';
        stringstream ss;
        ss << S;
        string ID;
        double price, size, size_in_quote;
        string trade_time, Can_buy, ignore;
        ss >> ID;
        ss >> price;
        ss >> size;
        ss >> size_in_quote;
        ss >> trade_time;
        ss >> Can_buy;
        ss >> ignore;
  //      cout << fixed << setprecision(5) << "pb(" << ID << ", " << price << ", " << size << ", " << size_in_quote << ", " << trade_time << ", " << Can_buy << ", true)\n";
        trade.push_back({ID, price, size, size_in_quote, trade_time, (Can_buy == "True" ? false : true), true});
        
        
        
    //    cout << S << '\n';
        loc ++;
    }
    file.close();
}

void period(int n, int m){
    int now_loc = 0;
    double w_sum = 0;
    double w = 1;
    double plus = 0.1;
    double sum = 0.0, weighted_sum = 0.0, last;
    double last_price;
    last = w;
    while(now_loc < n){
        now_loc ++;
        weighted_sum += w * price(now_loc);
    //    sum += price(now_loc);
        q.push(w);
    //    cout << "w = " << w << '\n';
        
        w_sum += w;
        w += plus;
        plus += 0.1;
    }
 
    while(now_loc < m){
        Max = max(Max, abs(money));
        now_loc ++;
        if(now_loc == m) last_price = price(now_loc);
        double price_base = weighted_sum / (double)w_sum;
        if(trade[now_loc].Can_buy && price(now_loc) <= price_base){  // buy
            buy(now_loc);
            item.insert({price(now_loc), size(now_loc)});
        }
        else if(!trade[now_loc].Can_buy && price(now_loc) >= price_base){
            double remain_size = size(now_loc);
            while(item.size() > 0 && remain_size > 0.0){
                auto it = item.begin();
                if(it->price > price(now_loc)) break; // if the items we have are more expensive, then don't sell them.
                if(it->size <= remain_size){   // if buyer's needings are more than the items we have, sell them all.
                    profit += (price(now_loc) - it->price) * it->size;
                    remain_size -= it->size;
                    item.erase(it);
                }
                else {
                    profit += (price(now_loc) - it->price) * it->size;
                    remain_size = 0;
                    item_data New_item = {it->price, it->size - remain_size};
                    item.erase(it);
                    item.insert(New_item);
                    break;
                }
            }
        }
        // sell or buy
        
        
        
        weighted_sum += w * price(now_loc);
     //   cout << "w = " << w << '\n';
        double last_w = q.front();
        q.pop();
        q.push(w);
        weighted_sum -= last_w * price(now_loc - n);
        w_sum += w;
        w += plus;
        w_sum -= last_w;
        plus += 0.1;
    //    sum = sum - price(now_loc - n) + price(now_loc);
    }
    cout << "last_price = " << last_price << '\n';
    if(item.size() > 0){
        for(auto it = item.begin(); it != item.end(); it ++){
            money += last_price * (it->size);
        }
        item.clear();
    }
}


void up (int n,int m){ //n is the big range, m is the small range
    int pos = 1;
    double remain_size = size(pos);
    bool checksell = 0;
    while(pos <= n){
        if(!checksell){
            bool counter = 1;
            int pos_copy = pos;
            for(int j = pos_copy + 1; j < pos_copy + m + 1; j++){
                if(j == n){
                    pos = j;
                    break;
                }
                if(price(j) - price(j - 1) <= 0){
                    counter = 0;
                    break;
                }
                pos++;
            }
            if(counter){
                buy(pos);
                item.insert({price(pos), size(pos)});
                checksell = 1;
                remain_size = size(pos);
            }
        }//buy
        if(checksell){
            while(item.size() > 0 && remain_size > 0.0){
                auto it = item.begin();
                if(it->size <= remain_size){   // if buyer's needings are more than the items we have, sell them all.
                    profit += (price(pos) - it->price) * it->size;
                    remain_size -= it->size;
                    item.erase(it);
                    checksell = 0;
                    pos++;
                }
                else{
                    profit += (price(pos) - it->price) * it->size;
                    remain_size = 0;
                    item_data New_item = {it->price, it->size - remain_size};
                    item.erase(it);
                    item.insert(New_item);
                    pos++;
                    break;
                }
            }
        }
    }
}

void _up(int n,int m){ //n is the big range, m is the small range
    int k = 2;
    double remain_size = 0, buy_price = 0;
    bool checksell = 0;
    for(int i = 1; i <= n; i = k){
        //cout << "k = " << k << '\n';
        if(!checksell){
            bool change = false, counter = true;
            for(int j = i + 1; j < i + m + 1; j++){
                if(j == n){
                    k = n;
                    //cout << "1\n";
                    change = true;
                    counter = false;
                    break;
                }
                if(price(j) - price(j - 1) <= 0){
                    k = j;
                    //cout << "2\n";
                    change = true;
                    counter = false;
                    break;
                }
            }
            if(counter){
                buy(k);
                buy_price = price(k);
                checksell = 1;
                remain_size = size(k);
                continue;
            }
            if(!change) k++;
        }//buy
        
        if(checksell && trade[i].Can_buy){
            if(remain_size > 0.0){
                if(remain_size >= size(i)){
                    sell(i, size(i));
                    remain_size -= size(i);
                }
                else{
                    sell(i, remain_size);
                    remain_size = 0;
                    checksell = 0;
                }
                k++;
            }
        }
    }
    sell(n, remain_size);
}



int main(){
    
    Max= 0;
    profit = 0.0;
    money = 0.0;
    trade.push_back({"", 0.0, 0.0, 0.0, "", "false", "false"});
    get_data(100000);
    item.clear();
    period(10, 10000);
    cout << fixed << setprecision(5) << "money = " << money << '\n';
    cout << profit << '\n';
    cout << Max << '\n';
    cout << fixed << setprecision(2) << profit / Max << '\n';
    cout << "times = " << times << '\n';
}
