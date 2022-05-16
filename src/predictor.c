//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <math.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Feiyu Yang";
const char *studentID   = "A59009686";
const char *email       = "f8yang@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

//define number of bits required for indexing the BHT here. 
int ghistoryBits = 14; // Number of bits used for Global History
int bpType;       // Branch Prediction Type
int verbose;


//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
//gshare
uint8_t *bht_gshare;
uint64_t ghistory;

//local
int localBits = 10;

uint64_t *local_feature;
uint8_t *local_bht;

//global
int globalBits = 16;

uint8_t *global_bht;
uint64_t path_history;

//tournament

uint8_t *tour_bht;

//custom
int customBits = 14;
uint8_t *custom_bht;


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

//gshare functions

// Initialize the predictor
//

void init_gshare() {
 int bht_entries = 1 << ghistoryBits;
  bht_gshare = (uint8_t*)malloc(bht_entries * sizeof(uint8_t));
  int i = 0;
  for(i = 0; i< bht_entries; i++){
    bht_gshare[i] = WN;
  }
  ghistory = 0;
}


uint8_t 
gshare_predict(uint32_t pc) {
  //get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries-1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries -1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;
  switch(bht_gshare[index]){
    case WN:
      return NOTTAKEN;
    case SN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
      return NOTTAKEN;
  }
}

void
train_gshare(uint32_t pc, uint8_t outcome) {
  //get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries-1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries -1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;

  //Update state of entry in bht based on outcome
  switch(bht_gshare[index]){
    case WN:
      bht_gshare[index] = (outcome==TAKEN)?WT:SN;
      break;
    case SN:
      bht_gshare[index] = (outcome==TAKEN)?WN:SN;
      break;
    case WT:
      bht_gshare[index] = (outcome==TAKEN)?ST:WN;
      break;
    case ST:
      bht_gshare[index] = (outcome==TAKEN)?ST:WT;
      break;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
  }

  //Update history register
  ghistory = ((ghistory << 1) | outcome); 
}

void
cleanup_gshare() {
  free(bht_gshare);
}

//local functions

// Initialize the predictor
//
void init_local() {
 int local_entries = 1 << localBits;
  local_feature = (uint64_t*)malloc(local_entries * sizeof(uint64_t));
  local_bht = (uint8_t*)malloc(local_entries * sizeof(uint8_t));
  int i = 0;
  for(i = 0; i< local_entries; i++){
    local_feature[i] = 0;
    local_bht[i] = WN;
  }
}

uint8_t 
local_predict(uint32_t pc) {
  //get lower ghistoryBits of pc
  uint32_t local_entries = 1 << localBits;
  uint32_t pc_lower_bits = pc & (local_entries-1);
  uint32_t index = local_feature[pc_lower_bits];
  switch(local_bht[index]){
    case WN:
      return NOTTAKEN;
    case SN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
      return NOTTAKEN;
  }
}

void
train_local(uint32_t pc, uint8_t outcome) {
  //get lower ghistoryBits of pc
  uint32_t local_entries = 1 << localBits;
  uint32_t pc_lower_bits = pc & (local_entries-1);
  uint32_t index = local_feature[pc_lower_bits];

  //Update state of entry in bht based on outcome
  switch(local_bht[index]){
    case WN:
      local_bht[index] = (outcome==TAKEN)?WT:SN;
      break;
    case SN:
      local_bht[index] = (outcome==TAKEN)?WN:SN;
      break;
    case WT:
      local_bht[index] = (outcome==TAKEN)?ST:WN;
      break;
    case ST:
      local_bht[index] = (outcome==TAKEN)?ST:WT;
      break;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
  }
  local_feature[pc_lower_bits] = (local_feature[pc_lower_bits] << 1 | outcome) & (local_entries-1);
}

void
cleanup_local() {
  free(local_feature);
  free(local_bht);
}

//global

void init_global() {
 int global_entries = 1 << globalBits;
  global_bht = (uint8_t*)malloc(global_entries * sizeof(uint8_t));
  int i = 0;
  for(i = 0; i< global_entries; i++){
    global_bht[i] = WN;
  }
  ghistory = 0;
}


uint8_t 
global_predict(uint16_t history) {
  int global_entries = 1 << globalBits;
  uint32_t index = history & (global_entries-1);
  switch(global_bht[index]){
    case WN:
      return NOTTAKEN;
    case SN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
      return NOTTAKEN;
  }
}

void
train_global(uint8_t outcome) {
  int global_entries = 1 << globalBits;
  //Update history register
  uint32_t index = path_history & (global_entries-1);

  //Update state of entry in bht based on outcome
  switch(global_bht[index]){
    case WN:
      global_bht[index] = (outcome==TAKEN)?WT:SN;
      break;
    case SN:
      global_bht[index] = (outcome==TAKEN)?WN:SN;
      break;
    case WT:
      global_bht[index] = (outcome==TAKEN)?ST:WN;
      break;
    case ST:
      global_bht[index] = (outcome==TAKEN)?ST:WT;
      break;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
  }
  path_history = ((path_history << 1) | outcome) & (global_entries-1); 
}

void
cleanup_global() {
  free(global_bht);
}

//tournament
void init_tour() {
 int tour_entries = 1 << globalBits;
  tour_bht = (uint8_t*)malloc(tour_entries * sizeof(uint8_t));
  int i = 0;
  for(i = 0; i< tour_entries; i++){
    tour_bht[i] = WN;
  }
  init_local();
  init_global();
}


uint8_t 
tour_predict(uint32_t pc) {
  int tour_entries = 1 << globalBits;
  uint32_t index = path_history & (tour_entries-1);
  switch(tour_bht[index]){
    case WN:
      return local_predict(pc);
    case SN:
      return local_predict(pc);
    case WT:
      return global_predict(path_history);
    case ST:
      return global_predict(path_history);
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
      return NOTTAKEN;
  }
}

void
train_tour(uint32_t pc, uint8_t outcome) {
  int tour_entries = 1 << globalBits;

  uint32_t index = path_history & (tour_entries-1);
  int correct = (global_predict(path_history) == outcome)? 1 : 0;
  //Update state of entry in bht based on outcome
  switch(tour_bht[index]){
    case WN:
      tour_bht[index] = (correct==1)?WT:SN;
      break;
    case SN:
      tour_bht[index] = (correct==1)?WN:SN;
      break;
    case WT:
      tour_bht[index] = (correct==1)?ST:WN;
      break;
    case ST:
      tour_bht[index] = (correct==1)?ST:WT;
      break;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
  }
  train_global(outcome);
  train_local(pc, outcome);
}

void
cleanup_tour() {
  free(tour_bht);
  cleanup_global();
  cleanup_local();
}

//custom
void init_custom() {
 int custom_entries = 1 << customBits;
  custom_bht = (uint8_t*)malloc(custom_entries * sizeof(uint8_t));
  int i = 0;
  for(i = 0; i< custom_entries; i++){
    custom_bht[i] = WN;
  }
  init_local();
  init_gshare();
}


uint8_t 
custom_predict(uint32_t pc) {
  int custom_entries = 1 << customBits;
  uint32_t index = ghistory & (custom_entries-1);
  switch(custom_bht[index]){
    case WN:
      return local_predict(pc);
    case SN:
      return local_predict(pc);
    case WT:
      return gshare_predict(pc);
    case ST:
      return gshare_predict(pc);
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
      return NOTTAKEN;
  }
}

void
train_custom(uint32_t pc, uint8_t outcome) {
  int custom_entries = 1 << customBits;

  uint32_t index = ghistory & (custom_entries-1);
  int correct = (gshare_predict(pc) == outcome)? 1 : 0;
  //Update state of entry in bht based on outcome
  switch(custom_bht[index]){
    case WN:
      custom_bht[index] = (correct==1)?WT:SN;
      break;
    case SN:
      custom_bht[index] = (correct==1)?WN:SN;
      break;
    case WT:
      custom_bht[index] = (correct==1)?ST:WN;
      break;
    case ST:
      custom_bht[index] = (correct==1)?ST:WT;
      break;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
  }
  train_local(pc, outcome);
    train_gshare(pc, outcome);
}

void
cleanup_custom() {
  free(custom_bht);
  cleanup_gshare();
  cleanup_local();
}

void
init_predictor()
{
  switch (bpType) {
    case STATIC:
    case GSHARE:
      init_gshare();
      break;
    case TOURNAMENT:
      init_tour();
    case CUSTOM:
      init_custom();
    default:
      break;
  }
  
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return gshare_predict(pc);
    case TOURNAMENT:
      return tour_predict(pc);
    case CUSTOM:
      return custom_predict(pc);
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//

void
train_predictor(uint32_t pc, uint8_t outcome)
{

  switch (bpType) {
    case STATIC:
    case GSHARE:
      return train_gshare(pc, outcome);
    case TOURNAMENT:
      return train_tour(pc, outcome);
    case CUSTOM:
      return train_custom(pc, outcome);
    default:
      break;
  }
  

}
