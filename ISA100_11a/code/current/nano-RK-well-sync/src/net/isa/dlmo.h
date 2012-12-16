/******************************************************************************
*  Nano-RK, a real-time operating system for sensor networks.
*  Copyright (C) 2007, Real-Time and Multimedia Lab, Carnegie Mellon University
*  All rights reserved.
*
*  This is the Open Source Version of Nano-RK included as part of a Dual
*  Licensing Model. If you are unsure which license to use please refer to:
*  http://www.nanork.org/nano-RK/wiki/Licensing
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, version 2.0 of the License.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*  Contributing Authors (specific to this file):
*  Azriel Samson
*  Vignesh Anantha Subramanian
*******************************************************************************/
#ifndef DLMO_H
#define DLMO_H

//********************************Configure max limits**********************************

#define DLMO_LINK_MAX_COUNT 20
#define DLMO_GRAPH_MAX_COUNT 10
#define DLMO_NEIGHBOR_MAX_COUNT 16
#define TRANSMIT_QUEUE_MAX_SIZE 20
#define DLMO_CANDIDATE_MAX_SIZE 20
#define MAX_RETRIES 3 //the number working of neighbors in graph should be checked if this changes

//****************************************************************************************

#define ISA_SLOTS_PER_FRAME 50
#define ISA_MAX_SLOTS 63

//************************linkType field in DLMO_LINK******************************
#define TRANSMIT_BIT 		0x80
#define RECEIVE_BIT	 		0x40
#define EXP_BACK_BIT 		0x20
#define IDLE_BIT 			0x10
#define DISCOVERY			0x0C
#define JOIN_RESPONSE		0x02
#define SELECTIVE			0x01

//Discovery
#define DISCOVERY_NONE 0
#define DISCOVERY_ADVERTISEMENT 1
#define DISCOVERY_BURST 2
#define DISCOVERY_SOLICITATION 3
//SELECTIVE
#define SELECTIVE_ALLOWED 1
#define SELECTIVE_NOT_ALLOWED 0


//enums for linkType

typedef enum {
	JOIN_RESP,
	TX_NO_ADV,
	TX_RX_NO_ADV,
	TX_ADV,
	ADV,
	BURST_ADV,
	BURST_ADV_SCAN,
	SOLICITATION,
	RX
} LinkType;

//***********************************************************************************
typedef enum{
	FAILURE,
	SUCCESS
} status;

//***********************************************************************************

/* For isa link*/

typedef enum{
    DHDR_INDEX=0,
    DHR_INDEX=0,
    DMXHR_INDEX=1,
    DAUX_INDEX=5,
    DROUT_INDEX=4, //compressed variant is 3 bytes
    DADDR_INDEX=37,
    //SLOT_INDEX=6,
    SLOT_INDEX=1,
    SRC_INDEX=2,//change
    DEST_INDEX = 3,
    OFFSET_HIGH=1,
    OFFSET_LOW=2,
    PKT_DATA_START= 7
    //PKT_DATA_START=42
} isa_pkt_field_t;

//**********************************************ISA_QUEUE***************************************************

typedef struct ISA_QUEUE ISA_QUEUE;

struct ISA_QUEUE
{
    int8_t length;
    uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
    bool transmitPending;
    bool usedSlot;
    void (*slot_callback)(ISA_QUEUE *, status);
    uint8_t priority;
    uint16_t neighbor;
    nrk_time_t time;
    uint8_t numTries;
} ;


//************************************dlmo.neighbor structure***************************************
//for typeInfo
# define CLOCK_NONE 0x00
# define CLOCK_SECONDARY 0x40
# define CLOCK_PREFERRED 0x80
# define CLOCK_RESERVED 0xC0

typedef struct {
	bool isPresent;
    uint16_t index;
    uint64_t EUI64;
    uint8_t groupCode1;
    uint8_t groupCode2;
    uint8_t typeInfo;// 7-6 ClockSource, 5-4 ExtGrCnt, DiagLevel 3-2, LinkBacklog 1,Reserved 0
    uint8_t linkBacklogIndex;//
    uint8_t linkBacklogDur;//
    uint8_t linkBacklogActivate;
}DLMO_NEIGHBOR;


//MACROS for bit manipulation

#define BITSET(x,y) ((x) |= 1<<(y))
#define BITCLEAR(x,y) ((x) &= ~(1<<(y)))
#define BITGET(x,y) ((x) & 1<<(y))
#define ISAMASK(x,y) ((x) & (y))
#define ISASET(x,y) ((x) | (y))
#define SHIFTRIGHT(x,y) ((x)>>(y))
#define SHIFTLEFT(x,y) ((x)<<(y))



/*************************************dlmo.Graph***************************************/
#define GRAPH_TYPE_MASK 0x30
#define GRAPH_TYPE_BIT 4


#define NEIGHBOR_COUNT_LOWER_BIT 4
#define NEIGHBOR_COUNT_MASK 0x70



typedef struct{
	uint16_t index;
	/*
	 * dlmo.Graph[].PreferredBranch. If this indicator is 1, treat the first listed neighbor as the
preferred branch, and the DL should wait until there is an opportunity to try at least one
transmission along the preferred branch before attempting other alternatives. If this
indicator is 0, do not give such preferential treatment to the first listed neighbor.
• dlmo.Queue allows the system manager to reserve up to 15 buffers of the message queue
   for DPDUs that are following the graph.
	 *
	 */
	uint8_t info; // Preferred branch (1)NeighborCount(3)Queue(4)
	/*
	 * dlmo.Graph[].MaxLifetime (units 1⁄4 s). If this element is non-zero, the value of
dlmo.MaxLifetime shall be overridden for all DPDUs being forwarded following this graph.
	 *
	 */
	uint8_t maxLifeTime;
	/*
	 * List of neighbors (commonly two neighbors for next-hop link diversity).
	 */
	uint16_t neighbor[3];
}DLMO_GRAPH;



// ********************************* dlmo.link structure *******************************


//For graphType subfield
typedef enum{
NEIGHBOR,
GRAPH,
GRAPH_NEIGHBOR
}GraphType;

typedef struct {
	bool   isPresent;
    uint16_t index;
    /*
     * dlmo.SuperframeIndex. Indicates the superframe reference for the link.
     *
     */
	uint8_t superframeIndex;
/*
 * Indicates how the link is configured for transmission and/or reception,
 * and/or neighbor discovery. See Table 182.
 *
 */
	uint8_t linkType;
	/*
	 * dlmo.Link[].Template1. Primary timeslot template. See 9.4.3.3 for a discussion of
	 * templates.
	 *
	 */
	uint8_t template1;
	/*
	 * dlmo.Link[].Template2. Secondary timeslot template, for transmit/receive (TR) slots only,
in combination with other link selections. Use Template2 as the receive template, if there
is no DPDU in the queue for the primary template. Template 2 is transmitted and
meaningful only for TRx links, that is, links where Link[].Type bits 6 and 7 both have a
value of 1.
	 *
	 */
	uint8_t template2;

    uint8_t typeInfo;		//Neighbor Type(7,6) |Graph Type (5,4) | Sched Type (3,2) | ChType(1) | Priority Type(0)

    /*
     *  A neighbor is designated for transmit
	 *	links. See 9.4.3.4 for a discussion of neighbors. When a neighbor is designated in a link, it
     * may reference either a dlmo.Neighbor index or a group
     *
     */
    DLMO_NEIGHBOR *neighbor;		//pointer to the neighbor that this link is configured for
    /*
     * DPDUs following a particular graph may be
given exclusive or priority access to certain transmit links. These fields, when so
configured, limit link access to certain graphs, thereby connecting the link to a particular
communication flow through the DL subnet. When GraphType is left blank, the transmit
link is available to any DPDU that is being routed through the link’s designated neighbor.
 When GraphType is used, a particular graph is given exclusive or priority access to the
link.
     *
     */
	DLMO_GRAPH* graphPtr;			//pointer to the graph that is configured for this link

	/*
	 * dlmo.Link[].SchedType, dlmo.Link[].Schedule. Indicates the timeslot position(s) of the link
within each superframe cycle. The schedule may designate a fixed offset, a fixed set of
intervals, a range, or a bitmap.
• 0=Offset only
• 1=Offset and interval
• 2=Range
• 3=Bitmap
	 *
	 */
	uint32_t schedule;
	/*
	 * Indicates how the link’s channel is selected
	 *
	 */
	uint8_t chOffset;
	/*
	 *  Indicates how the links priority is set. Link
priorities are functionally described in 9.1.8.5.
	 *
	 */
	uint8_t priority;
} DLMO_LINK;


// *******************************Alert report Descriptor **************************

typedef struct {
bool alertReportDisabled;
uint8_t alertReportPriority;
}ALERT_REPORT;

/*
 *
			Descriptor
Type: Alert Report Descriptor (Table 257)
Default: Disabled=False
Default: Priority=0

				Duration
Type: Unsigned16
Units: 1 s
Default: 60

 *
 *
 */

typedef struct {
ALERT_REPORT alertReport;
uint16_t duration;
}DLMO_DISCOVERY_ALERT;


//************************************ dlmo.Candidate***********************************

typedef struct {


	/*
	 * dlmo.Candidates.Neighbor N is the 16-bit address of each candidate neighbor in the DL
	 * subnet.
	 */

	uint16_t neighbor;
	/*
	 * dlmo.RSSI N indicates the strength of the radio signal from each candidate neighbor,
	 * based on received advertisements and possibly other DPDUs. See 9.1.15.2 for description
	 * of RSSI.
	 *
	 */
	uint8_t rssi;
	/*
	 * dlmo.RSQI N indicates the quality of the radio signal from each candidate neighbor, based
	 * on received advertisements and possibly other considerations. A higher number indicates
	 * a better radio signal. See 9.1.15.2. for description of RSQI. If the chipset does not support
	 * RSQI, i.e.
	 */
	uint8_t rsqi;
}CANDIDATE;

typedef struct {
uint8_t n;				//represents the number of neighbors that have been discovered
CANDIDATE candidate[DLMO_CANDIDATE_MAX_SIZE];		//candidate information
}DLMO_CANDIDATE;


//************************************ extern variables ***********************************
extern uint8_t isa_clk_src_id;


//************************************extern functions**************************************

extern void configureSlot(uint8_t slotNumber, uint16_t neighborId, LinkType linkType, bool clockSource, uint16_t graphId, uint8_t neighborCount, uint16_t n1, uint16_t n2, uint16_t n3, uint8_t graphType);
extern void dlmoInit();
extern DLMO_GRAPH* addGraph(uint16_t graphId, uint8_t neighborCount, uint16_t n1, uint16_t n2, uint16_t n3);
extern int8_t addLink(uint8_t slotNumber, uint16_t neighborId,uint16_t graphId , LinkType linkType, GraphType graphType);
extern DLMO_NEIGHBOR* addNeighbor(uint16_t index,uint64_t EUI64, uint8_t groupCode1, uint8_t groupCode2, bool clockSource,uint8_t  linkBacklogIndex,uint8_t linkBacklogDur, uint8_t linkBacklogActivate);
extern  void dd_data_indication(uint16_t srcAddr,uint16_t destAddr,uint8_t priority,bool discardEligibile, bool lh, uint8_t length, uint8_t *payload);
extern void sendPacket(uint16_t destAddr,uint8_t graphId, uint8_t length, uint8_t *payload, void (*slot_callback)(ISA_QUEUE *entry, status)) ;
extern DLMO_LINK * findLink(uint8_t slot);
extern bool isTransmitLinkPresent (uint8_t *payloadr);
extern void isaFreePacket(ISA_QUEUE *entry);
extern void clearCandidateTable();
extern int8_t addCandidate(uint16_t candidate);
extern  bool isDiscoveryAlertDue();
extern void updateLastSentTime();
extern int8_t sendAdv ();
extern void flushCandidateEntries();

#endif
