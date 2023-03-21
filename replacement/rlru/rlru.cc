////////////////////////////////////////////
//                                        //
//     SRRIP [Jaleel et al. ISCA' 10]     //
//     Jinchun Kim, cienlux@tamu.edu      //
//                                        //
////////////////////////////////////////////
//
#include "cache.h"

#define NUM_CORE 1
#define MAX_LLC_SETS 8192
#define LLC_WAYS 16

#define SAT_INC(x, max) (x < max) ? x + 1 : x
#define SAT_DEC(x) (x > 0) ? x - 1 : x
#define TRUE 1
#define FALSE 0

#define RRIP_OVERRIDE_PERC 0

// The base policy is SRRIP. SHIP needs the following on a per-line basis
#define maxRRPV 3
uint32_t age_counter[MAX_LLC_SETS][LLC_WAYS]; // to store the age_counter value for each cache line of each cache set 
uint32_t hit_register[MAX_LLC_SETS][LLC_WAYS]; // to store the hit register value for each cache line of each cache set
//uint32_t type_register[MAX_LLC_SETS][LLC_WAYS]; // to store the type of hit of each cache line of each cache set
uint32_t is_prefetch[MAX_LLC_SETS][LLC_WAYS];
uint32_t accumulator[MAX_LLC_SETS][LLC_WAYS];// to accumulate the age_counter values for each cache line of cache set after 32 demand hits
uint32_t demand_hits[MAX_LLC_SETS][LLC_WAYS];// to store the number of demand hits, after 32 demand hits, change the rd
uint32_t reuse_distance[MAX_LLC_SETS][LLC_WAYS];// to store the number of demand hits, after 32 demand hits, change the rd
// These two are only for sampled sets (we use 64 sets)
#define NUM_LEADER_SETS 64

//uint32_t ship_sample[MAX_LLC_SETS];
//uint32_t line_reuse[MAX_LLC_SETS][LLC_WAYS];
//uint64_t line_sig[MAX_LLC_SETS][LLC_WAYS];

// SHCT. Signature History Counter Table
// per-core 16K entry. 14-bit signature = 16k entry. 3-bit per entry
//#define maxSHCTR 7
//#define SHCT_bits 14
//#define SHCT_SIZE (1 << SHCT_bits)
//uint32_t SHCT[NUM_CORE][SHCT_SIZE];

// Statistics
//uint64_t insertion_distrib[NUM_TYPES][maxRRPV + 1];
//uint64_t total_prefetch_downgrades;

// initialize replacement state
void CACHE::initialize_replacement()
{
	cout << "Initialize RLR state" << endl;
	//set hit register to 0
	//set age counter to ?
	//set accumulator to 0
	//set is_prefetch to 0
	//set 
	 for (int i = 0; i < MAX_LLC_SETS; i++) {
		for (int j = 0; j < LLC_WAYS; j++) {
			hit_register[i][j] = 0;
		  	accumulator[i][j] = 0;
		  	is_prefetch[i][j] = 0;
		  	age_counter[i][j] = 0;
		  	demand_hits[i][j] = 0;
		  	reuse_distance[i][j] = 0; 
		}
	} 
}

//-----------------------------------------------------------------

// find replacement element way
uint32_t CACHE::find_victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t PC, uint64_t paddr, uint32_t type) //based on the priorities and recency
{
  	uint32_t priorities[LLC_WAYS];
  	int lowest_priority=20;
  	int eviction_index=0;
  	// find the priority of all the lines in the current set
  	for (int i = 0; i < LLC_WAYS; i++)
  	{
		priorities[i] = 0;
		if(age_counter[set][i] <= reuse_distance[set][i])
		{
			priorities[i] = priorities[i] + 8;
		}
		if(hit_register[set][i] > 0)
		{
			priorities[i] = priorities[i] + 1;
		}
		if(is_prefetch[set][i] == 0) // if using is_prefetch --> one: prefetch, zero: other; if using type_register --> one : prefetch, two: load, three: writeback, four: rfo..we can only keep one: prefetch and two: other
		{
			priorities[i] = priorities[i] + 1;
		}
		if(priorities[i] < lowest_priority)
		{
			lowest_priority = priorities[i];
			eviction_index = i;
		}
		else if(priorities[i] == lowest_priority)
		{
			if(age_counter[set][i] < age_counter[set][eviction_index]) //recency
			{
				eviction_index = i;
			}
		}
  	}

  	return eviction_index;
}

// called on every cache hit and cache fill
void CACHE::update_replacement_state(uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit) //what does type mean exactly ? type of access ?
{
	//uint32_t sig = line_sig[set][way];
	// if hit then update the hit register, the accumulator, the demand hits, and if demand hits >= 32 then reuse distance as well
	// update is prefetch if type is prefetch
	// update age counter to 0 of the way which was hit
	// if miss update the age counter to 0 again
	// increment age counter of all other ways
	// do we update the victim on a miss ?
	if (hit)
  	{
    	hit_register[set][way] = 1;  	
    	if (type == PREFETCH) 
    	{
    		is_prefetch[set][way] = 1;	
    	}   
    	accumulator[set][way] = accumulator[set][way] + age_counter[set][way];
    	demand_hits[set][way] = demand_hits[set][way] + 1;
    	if (demand_hits[set][way] > 31)
    	{
	    	reuse_distance[set][way] = accumulator[set][way]/16; // two * accumulator/thirty-two
	    	accumulator[set][way] = 0;
	    	demand_hits[set][way] = 0;
    	} 
  	}
  	else
  	{
    //TODO: figure out what happens on a miss	
  	}
  
	for (int i = 0; i < LLC_WAYS; i++)
  	{
	 	if(i!=way)
	 	{
		 	age_counter[set][way]++;
	 	}
	 	else
	 	{
		 	age_counter[set][way] = 0;
	 	} 
  	}
}

void CACHE::repl_rreplacementDrlru_final_stats() {
}
