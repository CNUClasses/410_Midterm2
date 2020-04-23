//============================================================================
// *** PROBLEM ***
// A store is selling widgets for WIDGET_PRICE (given below), the store
// only lets 2 shoppers in at a time, when 1 shopper leaves, another can enter.
// The rest of the shoppers must wait to enter the Store
// Each shopper has cash and will buy as many widgets as it can with that cash.
//
// The store employs 1 stocker, an inventory specialist whose job is to replenish widgets.
// The stocker will continue to add widgets, 1 at a time, until there are no more widgets
//
// *** DEPENDENCIES ***
// This project uses the Semaphore library given as an example project in Week 13
//
// *** REQUIREMENTS ***
// Please handle efficient notifications between threads
// Please ensure compact critical sections so there is maximal interactivity between threads
// Please synchronize access to all global variables and non threadsafe APIs
// Please ensure that unrelated critical sections are protected by different synchronization
//        objects for maximum performance
// Please ensure that program does not deadlock
//
// *** POINTS ***
// see TODOs below
//
// *** SUBMIT ***
// Please write required code in this file and submit THIS file only to scholar
//============================================================================

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include "../../410_Semaphore_Lib/src/Semaphore.h"

using namespace std;

const int   WIDGETS_IN_STOCKROOM			= 10000; //how many are initialy in stockroom
const int 	WIDGET_PRICE					= 1;	//cost per widget
const int 	MAX_SHOPPERS 					= 2;	//max number of shoppers at a time

//TODO 10 pts.. add any globals you need here, be sure to comment them
int 		widgetsAvailableToShoppers		= 0;	//total number widgets available to buy
bool		stocker_done 					= false;//has the stocker finished stocking

Semaphore 			s(MAX_SHOPPERS);
mutex 				m;
condition_variable 	cv;
mutex 				mReport;

//TODO 10 pts.. make this function efficiently threadsafe
//Prints information to the console
//id: thread id
//s:  string thread wants to output to the console
void report(int id, string s){
	lock_guard<mutex> lck(mReport);
	cout<<"Thread "<<id<<" reported:"<<s<<endl;
}

//ensures only MAX_SHOPPERS shoppers can be in the store at
//a time. The first MAX_SHOPPERS shoppers to call this function will return,
//the next will block, until one of the previous shoppers calls exitStore()
void enterStore(){
	//TODO 10 pts.. implement logic to limit  number of shoppers in store
	s.wait();
}
//allows another shopper in if 1 is waiting
void exitStore(){
	//TODO 10 pts.. implement logic to call when a shopper exits store
	s.signal();
}

//Waits to enter store and then buys as many widgets, 1 at a time,
//as it can afford from widgetsAvailableToShoppers
//
//id:    A thread ID
//cash:  The amount of money the shopper starts with, it spends it all on widgets
void shopper(int id, int cash ){
	//TODO be sure to handle appropriate synchronization
	enterStore();

	//If here thread is in store. Efficiently buy widgets until either
	//cash runs out or there are no more widgets to buy
	while(true){

		//TODO 20 pts as long as this shopper has cash>WIDGET_PRICE, and widgetsAvailableToShoppers == 0
		//and the stocker has not put out all the widgets, then wait until widgets are available to shoppers
		unique_lock<mutex> lck(m);
		while( widgetsAvailableToShoppers == 0 && stocker_done==false && cash >= WIDGET_PRICE)
			cv.wait(lck);

		//TODO 2 pts if shopper out of cash then leave
		//use the report function to indicate the shopper is "out of money, so leaving"
		if(cash<WIDGET_PRICE){
			report(id, "out of money, so leaving");
			break;
		}

		//TODO 4 pts if there are widgets, then buy one (1) if shopper has the cash
		//use the report function to indicate the shopper "bought 1 widget"
		if(widgetsAvailableToShoppers>0)
		{
			widgetsAvailableToShoppers--;
			cash -=WIDGET_PRICE;
			report(id, "bought 1 widget");
		}

		//TODO 4 pts if sold out then leave
		//use the report function to indicate "ran out of widgets to buy before ran out of money"
		if(widgetsAvailableToShoppers ==0 && stocker_done == true){
			report(id, "ran out of widgets to buy before ran out of money");
			break;
		}
	}

	exitStore();
}

//TODO 15 pts..replenishes widgets 1 at a time by transferring them
//from stock room (widgetsInStockRoom) to the floor (widgetsAvailableToShoppers)
//and then notify all shoppers that a widget is available
//TODO 5 pts..When the stocker runs out of widgets (widgetsInStockRoom == 0)
//it notifies all shoppers that stocker is done and then exits
//
//widgetsInStockRoom: the total number of widgets in the stockroom
void stocker(int widgetsInStockRoom){

	//TODO handle synchronization code
	while(widgetsInStockRoom>0){
		//TODO 15 pts .. take 1 from widgetsInStockRoom
		//and add it to WidgetsAvailable
		{
			lock_guard<mutex> lck(m);
			widgetsAvailableToShoppers += 1;
			widgetsInStockRoom--;
		}
		cv.notify_all();
	}

	//TODO 5 pts.. tell shoppers stocker has finished putting out all widgets
	{
		lock_guard<mutex> lck(m);
		stocker_done=true;
	}
	cv.notify_all();
}


int main() {
	//TODO 10 pts.. handle synchronization code
	cout << "The initial value of widgetsAvailableToShoppers is " << widgetsAvailableToShoppers << endl; //

	thread stocker1(stocker, WIDGETS_IN_STOCKROOM);	//has 10,000 widgets to sell
	thread shopper1(shopper, 1, 10);	//Thread 1, has 10 dollars to spend widgets,
	thread shopper2(shopper, 2, 1000);
	thread shopper3(shopper, 3, 1);
	thread shopper4(shopper, 4, 0);
	thread shopper5(shopper, 5, 15);
	thread shopper6(shopper, 6, 20);
	thread shopper7(shopper, 7, 11);

	{
		unique_lock<mutex> lck(m);
		cout<<"***************** in Main thread current value of widgetsAvailableToShoppers is"<<widgetsAvailableToShoppers<<endl;
	}

	stocker1.join();
	shopper1.join();
	shopper2.join();
	shopper3.join();
	shopper4.join();
	shopper5.join();
	shopper6.join();
	shopper7.join();

	cout << endl << "The final value of widgetsAvailableToShoppers is " << widgetsAvailableToShoppers << endl; //
	return 0;
}
