// tests/test_main.cpp
#ifndef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <memory>
#include "../include/doctest/doctest/doctest.h"
#include "structures/typedefs.h"
#include "structures/vroom/input/input.h"
#include "structures/vroom/job.h"
#include "structures/vroom/location.h"
#endif


#include "../src/structures/vroom/raw_route.h"

//#include "../src/my_lib.h" // Include the code you want to test

/**

shipments 0-9: pickup at depot, delivery at customer (stops 0-19)
shipments 19-20: pickup at customer, delivery at depot (stops 20-39)
*/
void test_is_return_to_depot_with_undelivered_jobs(bool expected,std::vector<int> stops,const vroom::Index first_rank=0,int first_rank_jobId=UINT16_MAX){
    // 1) create a dummy input with 5 shipments
    vroom::Input input;
    auto depotLocation=vroom::Location(0);

    vroom::Amount capacity(4);
    vroom::Vehicle vehicle_0(0, depotLocation, depotLocation,
      "profilename",capacity);
    input.vehicles.push_back(vehicle_0);

    for(int i=0; i<20; i++){
        auto customerLocation=vroom::Location(i+1);
        vroom::Location * deliveryLocation;
        vroom::Location * pickupLocation;

        if (i<10){
            pickupLocation=&depotLocation;
            deliveryLocation=&customerLocation;            
        } else {
            pickupLocation=&customerLocation;            
            deliveryLocation=&depotLocation;
        }
        
       vroom::UserDuration default_setup=0;
       vroom::UserDuration default_service=0;
        vroom::Amount delivery(4);
        vroom::Amount pickup(4);

        vroom::Job job_p((vroom::Id)i*2, vroom::JOB_TYPE::PICKUP, *pickupLocation,
        default_setup,default_service,pickup);
        vroom::Job job_d((vroom::Id)i*2, vroom::JOB_TYPE::DELIVERY, *deliveryLocation,
        default_setup,default_service,delivery);


        input.jobs.push_back(job_p);
        input.jobs.push_back(job_d);    
    }

    // 2) create a route with the given stops
    int v=0;
    vroom::RawRoute route(input, v, stops.size());
    for (int num : stops){
        route.route.push_back(num);
    }      
    route.update_amounts(input);

    assert(expected==route.is_return_to_depot_with_undelivered_jobs(input,first_rank,first_rank_jobId));
    
}
// constants for job ids:
// Shipment #, Pickup/Deliver at Depot/Customer

// shipments fwd: pickup at depot, delivery at customer
const int S00_PD=0;
const int S00_DC=1;
const int S01_PD=2;
const int S01_DC=3;
const int S02_PD=4; 
const int S02_DC=5;
const int S03_PD=6;
const int S03_DC=7;
const int S04_PD=8;
const int S04_DC=9;  
// ... 5 more shipments

// shipments back: pickup at customer, delivery at depot
const int S10_PC=20;
const int S10_DD=21;
const int S11_PC=22;
const int S11_DD=23;
const int S12_PC=24; 
const int S12_DD=25;
const int S13_PC=26;
const int S14_DD=27;
const int S15_PC=28;
const int S16_DD=29;  
// ... 5 more shipments




TEST_CASE("riding") {

    // inserting pickup at depot
    test_is_return_to_depot_with_undelivered_jobs(false,{
    S00_PD,S01_PD,S00_DC,S01_DC},
    2,S02_PD);
}




// 3) here comes the tough part: testing a route with unapplied change
TEST_CASE("Unapplied.empty") {
   test_is_return_to_depot_with_undelivered_jobs(false,{},0,S00_PD);
   test_is_return_to_depot_with_undelivered_jobs(false,{},0,S00_DC);

}


TEST_CASE("Unapplied.easy1") {
    
    // inserting pickup at depot
    test_is_return_to_depot_with_undelivered_jobs(false,{
    S00_PD,S01_PD,S00_DC,S01_DC},
    0,S02_PD);

    test_is_return_to_depot_with_undelivered_jobs(false,{
    S00_PD,S01_PD,S00_DC,S01_DC},
    1,S02_PD);

    // inserting pickup at depot
    test_is_return_to_depot_with_undelivered_jobs(false,{
    S00_PD,S01_PD,S00_DC,S01_DC},
    2,S02_PD);

    test_is_return_to_depot_with_undelivered_jobs(false,{
    S00_PD,S01_PD,S00_DC,S01_DC},
    4,S02_PD);

}


// 1) easiest cases. delivery to customer only

TEST_CASE("PD.easy1") {
   test_is_return_to_depot_with_undelivered_jobs(false,{S00_PD,S00_DC});
}
TEST_CASE("PD.easy2") {
   test_is_return_to_depot_with_undelivered_jobs(false,{S00_PD,S01_PD,S00_DC,S01_DC});
   // different delivery order
  test_is_return_to_depot_with_undelivered_jobs(false,{S00_PD,S01_PD,S01_DC,S00_DC});
}
TEST_CASE("PD. 2 routes") {
   test_is_return_to_depot_with_undelivered_jobs(false,{S00_PD,S01_PD,S00_DC,S01_DC,S02_PD,S02_DC});
}

TEST_CASE("PD.empty") {
   test_is_return_to_depot_with_undelivered_jobs(false,{});
}
TEST_CASE("PD. 2 routes undelivered") {
   test_is_return_to_depot_with_undelivered_jobs(true,{
    S00_PD,S01_PD,S00_DC,
    S02_PD,S01_DC,S02_DC});
}


// 2) pickup from customer only

TEST_CASE("PC.easy1") {
   test_is_return_to_depot_with_undelivered_jobs(false,{S10_PC,S10_DD});
}
TEST_CASE("PC.easy2") {
   test_is_return_to_depot_with_undelivered_jobs(false,{S10_PC,S11_PC,S10_DD,S11_DD});
   // different delivery order
   test_is_return_to_depot_with_undelivered_jobs(false,{S10_PC,S11_PC,S11_DD,S10_DD});
}
TEST_CASE("PC.2 routes") {
   test_is_return_to_depot_with_undelivered_jobs(false,{S10_PC,S11_PC,S10_DD,S11_DD,S12_PC,S12_DD});
}

TEST_CASE("PC.empty") {
   test_is_return_to_depot_with_undelivered_jobs(false,{});
}
TEST_CASE("PC.2 routes undelivered") {
   test_is_return_to_depot_with_undelivered_jobs(true,{
    S10_PC,S10_PC,S10_DD,
    S12_PC,S10_DD,S12_DD});
}

