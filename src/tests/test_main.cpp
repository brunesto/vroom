// tests/test_main.cpp
#ifndef DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <cstdint>
#include <memory>
#include <sys/types.h>
#include "../include/doctest/doctest/doctest.h"
#include "structures/typedefs.h"
#include "structures/vroom/input/input.h"
#include "structures/vroom/job.h"
#include "structures/vroom/location.h"
#endif


#include "../src/structures/vroom/raw_route.h"

// void forfucksakeb(
//      const std::vector<uint16_t>::iterator &first_job,
//      const std::vector<uint16_t>::iterator &last_job){ 

  
//     std::cout << &first_job<<":"<< *first_job <<std::endl;
//     std::cout << &last_job<<":"<< *last_job << std::endl; 
// }

// void forfucksake(){
//     std::vector<uint16_t> as={};  
//     as.push_back(4);
//     as.push_back(5);

//     long ptrdiff=(long)&as.end()-(long)&as.begin();
//     std::cout << "ptrdiff: " << ptrdiff << std::endl;
//     std::cout << &as.begin()<< std::endl;
//     std::cout << &as.end()<< std::endl; 

//    forfucksakeb(as.begin(),as.end());
    
// }
//#include "../src/my_lib.h" // Include the code you want to test

/**

shipments 0-9: pickup at depot, delivery at customer (stops 0-19)
shipments 19-20: pickup at customer, delivery at depot (stops 20-39)
*/
void test_is_return_to_depot_with_undelivered_jobs(
    std::vector<bool> expecteds,
    std::vector<uint16_t> stops,
    std::vector<uint16_t> insertedJobIds={}){

//   forfucksake();
   

    // 1) create a dummy input with 5 shipments
    vroom::Input input;
    auto depotLocation=vroom::Location(0);

    vroom::Amount capacity(1);
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
        vroom::Amount delivery(1);
        vroom::Amount pickup(1);

        vroom::Job job_p((vroom::Id)i*2, vroom::JOB_TYPE::PICKUP, *pickupLocation,
        default_setup,default_service,pickup);
        vroom::Job job_d((vroom::Id)i*2, vroom::JOB_TYPE::DELIVERY, *deliveryLocation,
        default_setup,default_service,delivery);


        input.jobs.push_back(job_p);
        input.jobs.push_back(job_d);    
    }

    // 2) create a route with the given stops
    int v=0;
    vroom::RawRoute route(input, v, 1);
    for (int num : stops){
        route.route.push_back(num);
    }      
    route.update_amounts(input);

  
    
    for(int i=0;i<(int)stops.size();i++){
        bool expected=expecteds[i];
        auto actual=route.is_return_to_depot_with_undelivered_jobs(input,i,insertedJobIds.begin(),insertedJobIds.end());
        if (actual!=expected){
            // run again for debug
            route.is_return_to_depot_with_undelivered_jobs(input,i,insertedJobIds.begin(),insertedJobIds.end());
            assert(false);
        }
        if (insertedJobIds.size()==0){
          break;
        }
    }
    
}
// constants for job ids:
// Shipment #, Pickup/Deliver at Depot/Customer

// shipments fwd: pickup at depot, delivery at customer
const uint16_t S00_PD=0;
const uint16_t S00_DC=1;
const uint16_t S01_PD=2;
const uint16_t S01_DC=3;
const uint16_t S02_PD=4; 
const uint16_t S02_DC=5;
const uint16_t S03_PD=6;
const uint16_t S03_DC=7;
const uint16_t S04_PD=8;
const uint16_t S04_DC=9;  
// ... + 5 more shipments

// shipments back: pickup at customer, delivery at depot
const uint16_t S10_PC=20;
const uint16_t S10_DD=21;
const uint16_t S11_PC=22;
const uint16_t S11_DD=23;
const uint16_t S12_PC=24; 
const uint16_t S12_DD=25;
const uint16_t S13_PC=26;
const uint16_t S13_DD=27;
const uint16_t S14_DD=28;
const uint16_t S14_PC=29;
// ... + 5 more shipments






// // missing the pc-dd cases

// TEST_CASE("Unapplied 2") {

//     int first_job=S02_PD;
//     int last_job=S02_DC;
//     std::vector<uint16_t> stops={
//         S00_PD,S01_PD,S00_DC,S01_DC};

//     // inserting at depot at start
//     test_is_return_to_depot_with_undelivered_jobs(false,stops,0,first_job,last_job);
   
//     // inserting at depot at end
//     test_is_return_to_depot_with_undelivered_jobs(false,stops,4,first_job,last_job);


//     // inserting pickup just before leaving depor works
//     test_is_return_to_depot_with_undelivered_jobs(false,stops,2,first_job,last_job);

//     test_is_return_to_depot_with_undelivered_jobs(true,stops,1,first_job,last_job);
//     test_is_return_to_depot_with_undelivered_jobs(true,stops,3,first_job,last_job);
    
// }


// 1) easiest cases. delivery to customer only

TEST_CASE("PD.easy1") {
   test_is_return_to_depot_with_undelivered_jobs({false},{S00_PD,S00_DC});
}
TEST_CASE("PD.easy2") {
   test_is_return_to_depot_with_undelivered_jobs({false},{S00_PD,S01_PD,S00_DC,S01_DC});
   // different delivery order
  test_is_return_to_depot_with_undelivered_jobs({false},{S00_PD,S01_PD,S01_DC,S00_DC});
}
TEST_CASE("PD. 2 routes") {
   test_is_return_to_depot_with_undelivered_jobs({false},{S00_PD,S01_PD,S00_DC,S01_DC,S02_PD,S02_DC});
}

TEST_CASE("empty") {
   test_is_return_to_depot_with_undelivered_jobs({false},{});
}
TEST_CASE("PD. 2 routes undelivered") {
   test_is_return_to_depot_with_undelivered_jobs({true},{
    S00_PD,S01_PD,S00_DC,
    S02_PD,S01_DC,S02_DC});
}


// 2) pickup from customer only

TEST_CASE("PC.easy1") {
   test_is_return_to_depot_with_undelivered_jobs({false},{S10_PC,S10_DD});
}
TEST_CASE("PC.easy2") {
   test_is_return_to_depot_with_undelivered_jobs({false},{S10_PC,S11_PC,S10_DD,S11_DD});
   // different delivery order
   test_is_return_to_depot_with_undelivered_jobs({false},{S10_PC,S11_PC,S11_DD,S10_DD});
}
TEST_CASE("PC.2 routes") {
   test_is_return_to_depot_with_undelivered_jobs({false},{S10_PC,S11_PC,S10_DD,S11_DD,S12_PC,S12_DD});
}
TEST_CASE("PC.2 routes undelivered") {
   test_is_return_to_depot_with_undelivered_jobs({true},{
    S10_PC,S10_PC,S10_DD,
    S12_PC,S10_DD,S12_DD});
}

// 3) mixed pickups and deliveries


TEST_CASE("PCPD.1 route") {
    test_is_return_to_depot_with_undelivered_jobs({false},{S01_PD,S01_DC,S10_PC,S10_DD});
    test_is_return_to_depot_with_undelivered_jobs({true},{S01_PD,S10_PC,S10_DD,S01_DC});
}
TEST_CASE("PCPD.1 route undelivered") {
    test_is_return_to_depot_with_undelivered_jobs({false},{S01_PD,S02_PD,S01_DC,S10_PC,S02_DC,S10_DD});
    test_is_return_to_depot_with_undelivered_jobs({true},{S01_PD,S02_PD,S01_DC,S10_PC,S10_DD,S02_DC});
    
}


// 3) here comes the tough part: testing a route with unapplied change

TEST_CASE("Unapplied.empty") {
   test_is_return_to_depot_with_undelivered_jobs({false},{},{S00_PD});
   test_is_return_to_depot_with_undelivered_jobs({false},{},{S00_DC});
}


TEST_CASE("Unapplied.PD easy") {
    
    // inserting pickup at depot
    test_is_return_to_depot_with_undelivered_jobs(
      {false,false,false},
      {    S00_PD,S00_DC},
      {S02_PD});

    
    test_is_return_to_depot_with_undelivered_jobs(
      {false,false,false,true,false},{
      S00_PD,S01_PD,S00_DC,S01_DC},
      {S02_PD});

        
    test_is_return_to_depot_with_undelivered_jobs(
      {false,true,false,true,false},
      {S00_PD,S01_PD,S00_DC,S01_DC},
      {S02_PD,S02_DC});

    // long insert
    test_is_return_to_depot_with_undelivered_jobs(
      {false,true,false,true,false},
      {S00_PD,S01_PD,S00_DC,S01_DC},
      {S02_PD,S03_PD,S03_DC,S02_DC});
    
}

TEST_CASE("Unapplied.PC easy") {
    
    // inserting pickup at depot
    test_is_return_to_depot_with_undelivered_jobs(
      {false,false,false},
      {      S10_PC,S10_DD},
      {S12_PC});

    
    test_is_return_to_depot_with_undelivered_jobs(
      {false,false,false,true,false},
      {    S10_PC,S11_PC,S10_DD,S11_DD},
      {S12_PC});

    test_is_return_to_depot_with_undelivered_jobs(
      {false,true,false,true,false},
      {    S10_PC,S11_PC,S10_DD,S11_DD},
      {S12_PC,S12_DD});

    // long insert  
    test_is_return_to_depot_with_undelivered_jobs(
      {false,true,false,true,false},
      {    S10_PC,S11_PC,S10_DD,S11_DD},
      {S12_PC,S13_PC,S13_DD,S12_DD});
    }
    


TEST_CASE("Unapplied.PCPD") {
    
    test_is_return_to_depot_with_undelivered_jobs(
      {false,false,false,true,false},
      {S01_PD,S01_DC,S10_PC,S10_DD},
      {S02_PD,S02_DC});

    test_is_return_to_depot_with_undelivered_jobs(
      {false,true,false,false,false},
      {S01_PD,S01_DC,S10_PC,S10_DD},
      {S12_PC,S12_DD});



}

