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

#define DLMO_LINK_MAX_COUNT 10
#define DLMO_NEIGHBOR_MAX_COUNT 25
#define TRANSMIT_QUEUE_MAX_SIZE 10

//****************************************************************************************

#define ISA_SLOTS_PER_FRAME 25
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

/* For isa link*/

typedef enum{
    DHDR_INDEX=0,
    DHR_INDEX=0,
    DMXHR_INDEX=1,
    DAUX_INDEX=5,
    DROUT_INDEX=34,
    DADDR_INDEX=37,
    //SLOT_INDEX=6,
    SLOT_INDEX=1,
    SRC_INDEX=2,//change
    OFFSET_HIGH=1,
    OFFSET_LOW=2,
    PKT_DATA_START=3
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
    void (*slot_callback)(ISA_QUEUE *);
    uint8_t priority;
    uint16_t neighbor;
    nrk_time_t time;
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
#define BITCLEAR(x,y) ((x) &= 1<<(y))
#define BITGET(x,y) ((x) & 1<<(y))
#define ISAMASK(x,y) ((x) & (y))
#define ISASET(x,y) ((x) | (y))





// ********************************* dlmo.link structure *******************************


typedef struct {
	bool   isPresent;
    uint16_t index;
	uint8_t superframeIndex;
	uint8_t linkType;
	uint8_t template1;
	uint8_t template2;
    uint8_t typeInfo;
    DLMO_NEIGHBOR *neighbor;		//pointer to the neighbor that this link is configured for
	uint16_t graphId;
	uint32_t schedule;
	uint8_t chOffset;
	uint8_t priority;
} DLMO_LINK;



//************************************extern functions**************************************

extern void configureSlot(uint8_t slotNumber, uint16_t neighborId, LinkType linkType, bool clockSource);
extern void dlmoInit();
extern int8_t addLink (uint8_t slotNumber, DLMO_NEIGHBOR* neighborIndex,LinkType linkType);
extern DLMO_NEIGHBOR* addNeighbor(uint16_t index,uint64_t EUI64, uint8_t groupCode1, uint8_t groupCode2, bool clockSource,uint8_t  linkBacklogIndex,uint8_t linkBacklogDur, uint8_t linkBacklogActivate);
extern void sendPacket(uint16_t destAddr, uint8_t length, uint8_t *payload, void (*slot_callback)(ISA_QUEUE *entry)) ;
extern DLMO_LINK * findLink(uint8_t slot);
extern void isaFreePacket(ISA_QUEUE *entry);
#endif
