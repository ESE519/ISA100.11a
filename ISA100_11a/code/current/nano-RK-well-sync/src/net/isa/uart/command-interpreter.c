/**
 * File: command-interpreter.c
 * Description: processes commands incoming over the serial port.
 *
 * Culprit(s): Richard Kelsey, Matteo Paris
 *
 * Copyright 2008 by Ember Corporation.  All rights reserved.               *80*
 */


// #include "serial.h"
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <command-interpreter.h>
#include <error-def.h>
#include <dlmo.h>
#include <dmo.h>
#include <isa_messagetypes.h>

//---------------------------------------------------------------------------
//Extern variables

extern uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
extern  DMO dmo;
extern DLMO_NEIGHBOR dlmoNeighbor[DLMO_NEIGHBOR_MAX_COUNT];
extern DLMO_GRAPH dlmoGraph[DLMO_GRAPH_MAX_COUNT];
extern DLMO_LINK dlmoLink[DLMO_LINK_MAX_COUNT];
extern uint16_t adjacencyMatrix[DLMO_NEIGHBOR_MAX_COUNT];
//-----------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Forward declarations.
static void callCommandAction(void);
static uint32_t stringToUnsignedInt(uint8_t argNum, bool swallowLeadingSign);
static uint8_t charDowncase(uint8_t c);
static uint16_t cnt1 = 0;
static bool isPrintEnabled = true;
//------------------------------------------------------------------------------
// Command parsing state

typedef struct {

  // Finite-state machine's current state.
  uint8_t state;

  // The command line is stored in this buffer.
  // Spaces and trailing '"' and '}' characters are removed,
  // and hex strings are converted to bytes.
  uint8_t buffer[EMBER_COMMAND_BUFFER_LENGTH];

  // Indices of the tokens (command(s) and arguments) in the above buffer.
  // The (+ 1) lets us store the ending index.
  uint8_t tokenIndices[MAX_TOKEN_COUNT + 1];

  // The number of tokens read in, including the command(s).
  uint8_t tokenCount;

  // Used while reading in the command line.
  uint8_t index;

  // First error found in this command.
  uint8_t error;

  // Storage for reading in a hex string. A value of 0xFF means unused.
  uint8_t hexHighNibble;

  // The token number of the first true argument after possible nested commands.
  uint8_t argOffset;

} EmberCommandState;

static EmberCommandState commandState;

// Remember the previous character seen by emberProcessCommandString() to ignore
// an LF following a CR.
static uint8_t previousCharacter = 0;

EmberCommandEntry *emberCurrentCommand;

enum {
  CMD_AWAITING_ARGUMENT,
  CMD_READING_ARGUMENT,
  CMD_READING_STRING,                  // have read opening " but not closing "
  CMD_READING_HEX_STRING,              // have read opening { but not closing }
  CMD_READING_TO_EOL                   // clean up after error
};

// This byte is used to toggle certain internal features on or off.
// By default all are off.
uint8_t emberCommandInterpreter2Configuration = 0x00;


void printTempCommand(void){
	uint8_t arg1;
	uint8_t arg2;
	arg1 = emberUnsignedCommandArgument(0);
	arg2 = emberUnsignedCommandArgument(1);
	if (isPrintEnabled){

		printf ("Came to temp command\r\n");
		printf ("Arg1:%d  Arg2:%d\n\r", arg1, arg2);
	}

}

void sendDummyPacket(void){

	MESSAGE *message;
	uint8_t length;
		message = &tx_buf[PKT_DATA_START];
		message->type = DUMMY_PAYLOAD;
		sprintf( &message->data,"node %d,%c",1,cnt1++);
	  	length=strlen(&tx_buf[PKT_DATA_START])+PKT_DATA_START+2; //1 for \0 + 1 for message->type
	  	sendPacket(emberUnsignedCommandArgument(0),emberUnsignedCommandArgument(1), length, tx_buf, NULL);
}


// uint16_t dest, uint8_t graphId ,uint16_t index
void sendConfigNeighbor(void){


	uint16_t destId = emberUnsignedCommandArgument(0);
	if (destId == dmo.dlAddress) {
		  addNeighbor(emberUnsignedCommandArgument(2),0,0,0,false,0,0,0);		//Add a neighbor for myself and return if this is my address
		return;
	}
	MESSAGE *message;
	uint8_t length;
	message = &tx_buf[PKT_DATA_START];
	message->type = ADD_NEIGHBOR;
	CONFIG_NEIGHBOR* configNeighbor = &message->data;
	configNeighbor->neighborId = emberUnsignedCommandArgument(2);;
	length=PKT_DATA_START+ sizeof(MESSAGE) + sizeof(CONFIG_NEIGHBOR) -1; // -1 because CONFIG_NEIGHBOR is casted to the last byte of MESSAGE
	sendPacket(destId,emberUnsignedCommandArgument(1), length, tx_buf, NULL);

}


void flushCandidateTable(void){

	uint16_t destId = emberUnsignedCommandArgument(0);
	if (destId == dmo.dlAddress) {
		flushCandidateEntries;			//Flush my own table and return if this is my address
		return;
	}
	MESSAGE *message;
	uint8_t length;
	message = &tx_buf[PKT_DATA_START];
	message->type = FLUSH_CANDIDATE_TABLE;
	//CONFIG_NEIGHBOR* configNeighbor = &message->data;
	//configNeighbor->neighborId = emberUnsignedCommandArgument(2);;
	length=PKT_DATA_START+ sizeof(MESSAGE) ;
	sendPacket(destId,emberUnsignedCommandArgument(1), length, tx_buf, NULL);

}

//uint16_t dest, uint8_t graphId, uint8_t graphId, uint8_t neighborCount, uint16_t neigh1, uint16_t neigh2, uint16_t neigh3
void sendConfigGraph(void){

	uint16_t destId = emberUnsignedCommandArgument(0);
		if (destId == dmo.dlAddress) {
			addGraph(emberUnsignedCommandArgument(2),emberUnsignedCommandArgument(3),emberUnsignedCommandArgument(4),emberUnsignedCommandArgument(5),emberUnsignedCommandArgument(6));
			return;
		}
	MESSAGE *message;
	uint8_t length;
	message = &tx_buf[PKT_DATA_START];
	message->type = ADD_GRAPH;
	CONFIG_GRAPH* configGraph = &message->data;
	configGraph->graphId =  emberUnsignedCommandArgument(2);
	configGraph->neighborCount = emberUnsignedCommandArgument(3);
	configGraph->neigh1 = emberUnsignedCommandArgument(4);
	configGraph->neigh2 = emberUnsignedCommandArgument(5);
	configGraph->neigh3 = emberUnsignedCommandArgument(6);
	length=PKT_DATA_START+ sizeof(MESSAGE) + sizeof(CONFIG_GRAPH) -1; // -1 because CONFIG_GRAPH is casted to the last byte of MESSAGE
	sendPacket(emberUnsignedCommandArgument(0),emberUnsignedCommandArgument(1), length, tx_buf, NULL);
}
void sendConfigLink(void){

	uint16_t destId = emberUnsignedCommandArgument(0);
			if (destId == dmo.dlAddress) {
				addLink(emberUnsignedCommandArgument(2),emberUnsignedCommandArgument(3),emberUnsignedCommandArgument(4),emberUnsignedCommandArgument(5),emberUnsignedCommandArgument(6));
				return;
			}
	MESSAGE *message;
	uint8_t length;
	message = &tx_buf[PKT_DATA_START];
	message->type = ADD_LINK;
	CONFIG_LINK* configLink = &message->data;
	configLink->slotNumber =  emberUnsignedCommandArgument(2);
	configLink->neighborId = emberUnsignedCommandArgument(3);
	configLink->graphId = emberUnsignedCommandArgument(4);
	configLink->linkType = emberUnsignedCommandArgument(5);
	configLink->graphType = emberUnsignedCommandArgument(6);
	length=PKT_DATA_START+ sizeof(MESSAGE) + sizeof(CONFIG_LINK) -1; // -1 because CONFIG_LINK is casted to the last byte of MESSAGE
	sendPacket(emberUnsignedCommandArgument(0),emberUnsignedCommandArgument(1), length, tx_buf, NULL);

}

void printSendCommandArguments(void){

	if (isPrintEnabled){
	printf ("\n\n\n\tneighbor -->| message_destId | message_graphId | neighborId \n\r");
	printf ("\tgraph\t -->| message_destId | message_graphId | graphID |");
	printf ("neighCount | n1 | n2 | n3\n\r");
	printf ("\tlink\t -->| message_destId | message_graphId | slotNo");
	printf ("neighbor | graphId | linkType | graphType\n\n\n");
	}
}

void printEnable(void){

	isPrintEnabled = true;
}

void printDisable(void){
	isPrintEnabled = false;
}


void printNeighbor(void){

	if (isPrintEnabled){
		printf ("\tNeighborId\tClock Source\r\n");
		for(int i=0;i<DLMO_NEIGHBOR_MAX_COUNT;i++){
			if (dlmoNeighbor[i].isPresent){
				printf("\t%d\t\t\t%d\r\n",dlmoNeighbor[i].index, (ISAMASK(dlmoNeighbor[i].typeInfo, CLOCK_PREFERRED) == CLOCK_PREFERRED));
			}

		}
	}

}

void printGraph(void){

	if (isPrintEnabled){
			printf ("\tGraphID\t\tNo. of neighbors\tn1\tn2\tn3\r\n");

				for(int i=0;i<DLMO_GRAPH_MAX_COUNT;i++)
					{
						if(dlmoGraph[i].index){
							printf("\t%d\t\t\t%d\t\t%d\t%d\t%d\r\n",dlmoGraph[i].index, SHIFTRIGHT(ISAMASK(dlmoGraph[i].info, NEIGHBOR_COUNT_MASK),NEIGHBOR_COUNT_LOWER_BIT), dlmoGraph[i].neighbor[0],dlmoGraph[i].neighbor[1], dlmoGraph[i].neighbor[2] );
						}
			}
		}

}

void printLink(void){

	if (isPrintEnabled){
			printf ("\tLinkID\tSlot Number\tNeighID\tGraphID\t    LinkType\tGraphType\r\n");
		for (uint8_t i = 0; i < DLMO_LINK_MAX_COUNT; i++)
			{
				if (dlmoLink[i].isPresent) {

					printf("\t%d\t%d\t\t%d\t%d\t    %d\t\t%d\r\n",i,dlmoLink[i].chOffset,dlmoLink[i].neighbor->index,dlmoLink[i].graphPtr->index,dlmoLink[i].linkType, SHIFTRIGHT(ISAMASK(dlmoLink[i].typeInfo,GRAPH_TYPE_MASK),GRAPH_TYPE_BIT));
				}
			}
	}

}


void printMatrix(void){


		for (uint8_t i = 0; i < DLMO_NEIGHBOR_MAX_COUNT; i++)
			{
					printf("%d ",adjacencyMatrix[i]);
				}
		putchar('\r');
		putchar('\n');

	}



void setEcho(void){
	if (emberUnsignedCommandArgument(0) == 0)  emberCommandInterpreterEchoOff();
	else emberCommandInterpreterEchoOn();

}
EmberCommandEntry printCommands[] = {
		  {"temp", printTempCommand, "uu"},
		  {"command", emberPrintCommandTable, ""},
		//  {"sendcommands", printSendCommandArguments, ""},
		  {"enable", printEnable, ""},
		  {"disable", printDisable, ""},
		  {"Link", printLink, ""},
		  {"Neighbor", printNeighbor, ""},
		  {"Graph", printGraph, ""},
		  {"Matrix", printMatrix, ""},
		  { NULL }
		};

EmberCommandEntry sendCommands[] = {
		  {"dummyPacket", sendDummyPacket, "vu", "Destination ID| Graph ID "}, 				//uint16_t dest, uint8_t graphId
//dest, graphId, slotNumber, neighborId, linkType,clockSource,graphId,neighborCount,n1,n2,n3,graphType)
		  {"Neighbor", sendConfigNeighbor, "vuv", "Destination ID| Graph ID| Neighbor to add"},			// uint16_t dest, uint8_t graphId ,uint16_t index
		  {"Graph", sendConfigGraph, "vuuuvvv", "Destination ID| Graph ID| Graph to add| neighCount| n1| n2| n3"},		 //uint16_t dest, uint8_t graphId, uint8_t graphId, uint8_t neighborCout, uint16_t neigh1, uint16_t neigh2, uint16_t neigh3
		  {"Link", sendConfigLink, "vuuvuuu", "Destination ID| Graph ID| Slot| neigh| graph| linkType| GraphType"},				////uint16_t dest, uint8_t graphId, uint8_t slotNumber, uint16_t neighborID, uin8_t graphId, uint8_t linkType, uint8_graphType
		  {"flushCandidate", flushCandidateTable, "vu", "Sends a command to flush candidate table"},
		  { NULL }
		};

EmberCommandEntry emberCommandTable[] = {

	    {"print",            NULL,                             (PGM_P)printCommands},
	    {"send",            NULL,                             (PGM_P)sendCommands, "List of commands that can be sent"},
	    {"echo",            setEcho,                             "u"},
  {NULL,               NULL,                             NULL}, // terminator
};

/**
 * @addtogroup cli
 * @{
 */
/**
 * @brief
 *        <b>debugprint status</b>
 *        - <i>Displays the current status of debug printing on the
 *          application.</i>
 *
 *        <b>debugprint all_on</b>
 *        - <i>Turns on all debug printing
 *        for printing options which are compiled into the application.</i>
 *
 *        <b>debugprint all_off</b>
 *        - <i>Turns off all debug printing for printing options which are compiled
 *        into the application.</i>
 *
 *        <b>debugprint on &lt;area&gt;</b>
 *        - <i>Turns on debug printing for a specific area
 *           - area - two byte value indicating the area to turn on</i>
 *
 *        <b>debugprint off &lt;area&gt;</b>
 *        - <i>Turns off debug printing for a specific area
 *           - area - two byte value indicating the area to turn off</i>
 */






//----------------------------------------------------------------
// Initialize the state machine.

void emberCommandReaderInit(void)
{
  commandState.state = CMD_AWAITING_ARGUMENT;
  commandState.index = 0;
  commandState.tokenIndices[0] = 0;
  commandState.tokenCount = 0;
  commandState.error = EMBER_CMD_SUCCESS;
  commandState.hexHighNibble = 0xFF;
  commandState.argOffset = 0;
  emberCurrentCommand = NULL;
}

// Returns a value > 15 if ch is not a hex digit.
static uint8_t hexToInt(uint8_t ch)
{
  return ch - (ch >= 'a' ? 'a' - 10
               : (ch >= 'A' ? 'A' - 10
                  : (ch <= '9' ? '0'
                     : 0)));
}

static uint8_t tokenLength(uint8_t num)
{
  return (commandState.tokenIndices[num + 1]
          - commandState.tokenIndices[num]);
}

static uint8_t *tokenPointer(int8_t tokenNum)
{
  return commandState.buffer + commandState.tokenIndices[tokenNum];
}

EmberStatus emberSerialReadByte(uint8_t *dataByte)
{
  int8_t ch=-1;
	if (nrk_uart_data_ready(NRK_DEFAULT_UART)!=0)
  ch = getchar();

  if(ch<0) {
    return EMBER_SERIAL_RX_EMPTY;
  }
  *dataByte = (uint8_t)ch;
  return EMBER_SUCCESS;
}

//----------------------------------------------------------------
// This is a state machine for parsing commands.  If 'input' is NULL
// 'sizeOrPort' is treated as a port and characters are read from there.
//
// Goto's are used where one parse state naturally falls into another,
// and to save flash.

bool emberProcessCommandString(uint8_t *input, uint8_t sizeOrPort)
{
  bool isEol = FALSE;
  bool isSpace, isQuote;

  while (TRUE) {
    uint8_t next;

    if (input == NULL) {
      switch (emberSerialReadByte(&next)) {
      case EMBER_SUCCESS:
        break;
      case EMBER_SERIAL_RX_EMPTY:
        return isEol;
      default:
        commandState.error = EMBER_CMD_ERR_PORT_PROBLEM;
        goto READING_TO_EOL;
      }
    } else if (sizeOrPort == 0) {
      return isEol;
    } else {
      next = *input;
      input += 1;
      sizeOrPort -= 1;
    }

    //   fprintf(stderr, "[processing '%c' (%s)]\n", next, stateNames[commandState.state]);

    if (previousCharacter == '\r' && next == '\n') {
      previousCharacter = next;
      continue;
    }
    previousCharacter = next;
    isEol = ((next == '\r') || (next == '\n'));
    isSpace = (next == ' ');
    isQuote = (next == '"');


    switch (commandState.state) {

    case CMD_AWAITING_ARGUMENT:
      if (isEol) {
        callCommandAction();
      } else if (! isSpace) {
        if (isQuote) {
          commandState.state = CMD_READING_STRING;
        } else if (next == '{') {
          commandState.state = CMD_READING_HEX_STRING;
        } else {
          commandState.state = CMD_READING_ARGUMENT;
        }
        goto WRITE_TO_BUFFER;
      }
      break;

    case CMD_READING_ARGUMENT:
      if (isEol || isSpace) {
        goto END_ARGUMENT;
      } else {
        goto WRITE_TO_BUFFER;
      }

    case CMD_READING_STRING:
      if (isQuote) {
        goto END_ARGUMENT;
      } else if (isEol) {
        commandState.error = EMBER_CMD_ERR_ARGUMENT_SYNTAX_ERROR;
        goto READING_TO_EOL;
      } else {
        goto WRITE_TO_BUFFER;
      }

    case CMD_READING_HEX_STRING: {
      bool waitingForLowNibble = (commandState.hexHighNibble != 0xFF);
      if (next == '}') {
        if (waitingForLowNibble) {
          commandState.error = EMBER_CMD_ERR_ARGUMENT_SYNTAX_ERROR;
          goto READING_TO_EOL;
        }
        goto END_ARGUMENT;
      } else {
        uint8_t value = hexToInt(next);
        if (value < 16) {
          if (waitingForLowNibble) {
            next = (commandState.hexHighNibble << 4) + value;
            commandState.hexHighNibble = 0xFF;
            goto WRITE_TO_BUFFER;
          } else {
            commandState.hexHighNibble = value;
          }
        } else if (! isSpace) {
          commandState.error = EMBER_CMD_ERR_ARGUMENT_SYNTAX_ERROR;
          goto READING_TO_EOL;
        }
      }
      break;
    }

    READING_TO_EOL:
      commandState.state = CMD_READING_TO_EOL;

    case CMD_READING_TO_EOL:
      if (isEol) {
        if (commandState.error != EMBER_CMD_SUCCESS) {
          emberCommandErrorHandler(commandState.error);
        }
        emberCommandReaderInit();
      }
      break;

    END_ARGUMENT:
      if (commandState.tokenCount == MAX_TOKEN_COUNT) {
        commandState.error = EMBER_CMD_ERR_WRONG_NUMBER_OF_ARGUMENTS;
        goto READING_TO_EOL;
      }
      commandState.tokenCount += 1;
      commandState.tokenIndices[commandState.tokenCount] = commandState.index;
      commandState.state = CMD_AWAITING_ARGUMENT;
      if (isEol) {
        callCommandAction();
      }
      break;

    WRITE_TO_BUFFER:
      if (commandState.index == EMBER_COMMAND_BUFFER_LENGTH) {
        commandState.error = EMBER_CMD_ERR_STRING_TOO_LONG;
        goto READING_TO_EOL;
      }
      if (commandState.state == CMD_READING_ARGUMENT) {
        next = charDowncase(next);
      }
      commandState.buffer[commandState.index] = next;
      commandState.index += 1;
      break;

    default: {
    }
    } //close switch.
  }
}

//----------------------------------------------------------------
// Command lookup and processing

// Returs true if entry is a nested command, and in this case
// it populates the nestedCommand pointer.
// Otherwise it returns false, and does nothing with nestedCommand
//
// Nested commands are implemented by setting the action
// field to NULL, and the argumentTypes field is a pointer
// to a nested EmberCommandEntry array. The older mechanism is
// to set argumentTypes to "n" and then the action field
// contains the EmberCommandEntry, but that approach has a problem
// on AVR128, therefore it is technically deprecated. If you have
// a choice, put NULL for action and a table under argumentTypes.
static bool getNestedCommand(EmberCommandEntry *entry,
                                EmberCommandEntry **nestedCommand) {
  if ( entry -> action == NULL ) {
    *nestedCommand = (EmberCommandEntry*)entry->argumentTypes;
    return TRUE;
  } else if ( entry -> argumentTypes[0] == 'n' ) {
    *nestedCommand = (EmberCommandEntry*)(void*)entry->action;
    return TRUE;
  } else {
    return FALSE;
  }
}

static uint8_t charDowncase(uint8_t c)
{
  if ('A' <= c && c <= 'Z')
    return c + 'a' - 'A';
  else
    return c;
}

static uint8_t firstByteOfArg(uint8_t argNum)
{
  uint8_t tokenNum = argNum + commandState.argOffset;
  return commandState.buffer[commandState.tokenIndices[tokenNum]];
}

// To support existing lazy-typer functionality in the app framework,
// we allow the user to shorten the entered command so long as the
// substring matches no more than one command in the table.
//
// To allow CONST savings by storing abbreviated command names, we also
// allow matching if the input command is longer than the stored command.
// To reduce complexity, we do not handle multiple inexact matches.
// For example, if there are commands 'A' and 'AB', and the user enters
// 'ABC', nothing will match.

static EmberCommandEntry *commandLookup(EmberCommandEntry *commandFinger,
                                        uint8_t tokenNum)
{
  EmberCommandEntry *inexactMatch = NULL;
  uint8_t *inputCommand = tokenPointer(tokenNum);
  uint8_t inputLength = tokenLength(tokenNum);
  bool multipleMatches = FALSE;

  for (; commandFinger->name != NULL; commandFinger++) {
    PGM_P entryFinger = commandFinger->name;
    uint8_t *inputFinger = inputCommand;
    for (;; entryFinger++, inputFinger++) {
      bool endInput = (inputFinger - inputCommand == inputLength);
      bool endEntry = (*entryFinger == 0);
      if (endInput && endEntry) {
        return commandFinger;  // Exact match.
      } else if (endInput || endEntry) {
        if (inexactMatch != NULL) {
          multipleMatches = TRUE;  // Multiple matches.
          break;
        } else {
          inexactMatch = commandFinger;
          break;
        }
      } else if (charDowncase(*inputFinger) != charDowncase(*entryFinger)) {
        break;
      }
    }
  }
  return (multipleMatches || false ? NULL : inexactMatch);
}

EmberStatus emberSerialWriteData(uint8_t *data, uint8_t length)
{

	while(length--) {
    putchar(*data);
    data++;
  }

  return EMBER_SUCCESS;
}

static void echoPrint(void)
{

  uint8_t tokenNum = 0;
  for ( ; tokenNum < commandState.tokenCount; tokenNum++ ) {
    uint8_t *ptr = tokenPointer(tokenNum);
    uint8_t len = tokenLength(tokenNum);
    emberSerialWriteData(ptr, len);
    printf(" ");
  }
  printf("\r\n");
}

static void callCommandAction(void)
{
  EmberCommandEntry *commandFinger = emberCommandTable;
  uint8_t tokenNum = 0;
  // We need a separate argTypeNum index because of the '*' arg type.
  uint8_t argTypeNum, argNum;

  if (commandState.tokenCount == 0) {
    goto kickout2;
  }

  // If we have echo, we echo here.
  if ( emberCommandInterpreterIsEchoOn() ) {
	  if (isPrintEnabled){
	  echoPrint();
	  }
  }

  // Lookup the command.
  while (TRUE) {
    commandFinger = commandLookup(commandFinger, tokenNum);
    if (commandFinger == NULL) {
      commandState.error = EMBER_CMD_ERR_NO_SUCH_COMMAND;
      goto kickout;
    } else {
      emberCurrentCommand = commandFinger;
      tokenNum += 1;
      commandState.argOffset += 1;

      if ( getNestedCommand(commandFinger, &commandFinger) ) {
        if (tokenNum >= commandState.tokenCount) {
          commandState.error = EMBER_CMD_ERR_WRONG_NUMBER_OF_ARGUMENTS;
          goto kickout;
        }
      } else {
        break;
      }
    }
  }

  // If you put '?' as the first character
  // of the argument format string, then you effectivelly
  // prevent the argument validation, and the command gets executed.
  // At that point it is down to the command to deal with whatever
  // arguments it got.
  if ( commandFinger->argumentTypes[0] == '?' )
    goto kickout;

  // Validate the arguments.
  for(argTypeNum = 0, argNum = 0;
      tokenNum < commandState.tokenCount;
      tokenNum++, argNum++) {
    uint8_t type = commandFinger->argumentTypes[argTypeNum];
    uint8_t firstChar = firstByteOfArg(argNum);
    switch(type) {

    // Integers
    case 'u':
    case 'v':
    case 'w':
    case 's': {
      uint32_t limit = (type == 'u' ? 0xFF
                      : (type == 'v' ? 0xFFFF
                         : (type =='s' ? 0x7F : 0xFFFFFFFFUL)));
      if (stringToUnsignedInt(argNum, true) > limit) {
        commandState.error = EMBER_CMD_ERR_ARGUMENT_OUT_OF_RANGE;
      }
      break;
    }

    // String
    case 'b':
      if (firstChar != '"' && firstChar != '{') {
        commandState.error = EMBER_CMD_ERR_ARGUMENT_SYNTAX_ERROR;
      }
      break;

    case 0:
      commandState.error = EMBER_CMD_ERR_WRONG_NUMBER_OF_ARGUMENTS;
      break;

    default:
      commandState.error = EMBER_CMD_ERR_INVALID_ARGUMENT_TYPE;
      break;
    }

    if (commandFinger->argumentTypes[argTypeNum + 1] != '*') {
      argTypeNum += 1;
    }

    if (commandState.error != EMBER_CMD_SUCCESS) {
      goto kickout;
    }
  }

  if (! (commandFinger->argumentTypes[argTypeNum] == 0
         || commandFinger->argumentTypes[argTypeNum + 1] == '*')) {
    commandState.error = EMBER_CMD_ERR_WRONG_NUMBER_OF_ARGUMENTS;
  }

 kickout:

  if (commandState.error == EMBER_CMD_SUCCESS) {
    (commandFinger->action)();
  } else {
	  if (isPrintEnabled){
    emberCommandErrorHandler(commandState.error);
	  }
	  }

 kickout2:

  emberCommandReaderInit();
}


//----------------------------------------------------------------
// Retrieving arguments

uint8_t emberCommandArgumentCount(void)
{
  return (commandState.tokenCount - commandState.argOffset);
}

static uint32_t stringToUnsignedInt(uint8_t argNum, bool swallowLeadingSign)
{
  uint8_t tokenNum = argNum + commandState.argOffset;
  uint8_t *string = commandState.buffer + commandState.tokenIndices[tokenNum];
  uint8_t length = tokenLength(tokenNum);
  uint32_t result = 0;
  uint8_t base = 10;
  uint8_t i;
  for (i = 0; i < length; i++) {
    uint8_t next = string[i];
    if (swallowLeadingSign && i == 0 && next == '-') {
      // do nothing
    } else if ((next == 'x' || next == 'X')
               && result == 0
               && (i == 1 || i == 2)) {
      base = 16;
    } else {
      uint8_t value = hexToInt(next);
      if (value < base) {
        result = result * base + value;
      } else {
        commandState.error = EMBER_CMD_ERR_ARGUMENT_SYNTAX_ERROR;
        return 0;
      }
    }
  }
  return result;
}

uint32_t emberUnsignedCommandArgument(uint8_t argNum)
{
  return stringToUnsignedInt(argNum, false);
}

int16_t emberSignedCommandArgument(uint8_t argNum)
{
  bool negative = (firstByteOfArg(argNum) == '-');
  int16_t result = (int16_t) stringToUnsignedInt(argNum, negative);
  return (negative ? -result : result);
}

uint8_t *emberStringCommandArgument(int8_t argNum, uint8_t *length)
{
  uint8_t tokenNum = argNum + commandState.argOffset;
  uint8_t leadingQuote = (argNum < 0 ? 0 : 1);
  if (length != NULL) {
    *length = tokenLength(tokenNum) - leadingQuote;
  }
  return tokenPointer(tokenNum) + leadingQuote;
}

uint8_t emberCopyStringArgument(int8_t argNum,
                              uint8_t *destination,
                              uint8_t maxLength,
                              bool leftPad)
{
  uint8_t padLength;
  uint8_t argLength;
  uint8_t *contents = emberStringCommandArgument(argNum, &argLength);
  if (argLength > maxLength) {
    argLength = maxLength;
  }
  padLength = leftPad ? maxLength - argLength : 0;
  memset(destination, 0, padLength);
  memcpy(destination + padLength, contents, argLength);
  return argLength;
}

PGM_P emberCommandErrorNames[] =
  {
    "",
    "Serial port error",
    "No such command",
    "Wrong number of args",
    "Arg out of range",
    "Arg syntax error",
    "Too long",
    "Bad arg type"
  };


static void printCommandUsage(EmberCommandEntry *entry)
{
  PGM_P arg = entry->argumentTypes;
  printf("%s", entry->name);

  if ( entry -> action == NULL ); //printf("...");
  else
    while (*arg) {
      uint8_t c = *arg;

      printf(c == 'u' ? " <uint8_t>"
                         : c == 'v' ? " <uint16_t>"
                         : c == 'w' ? " <uint32_t>"
                         : c == 's' ? " <int8_t>"
                         : c == 'b' ? " <string>"
                         : c == 'n' ? " ..."
                         : c == '*' ? " *"
                         : " ?");


      arg += 1;
    }

  if(entry->description) {
    printf(" - %s", entry->description);
  }

  printf( "\r\n");
 // emberSerialWaitSend(APP_SERIAL);
}

void emberPrintCommandUsage(EmberCommandEntry *entry)
{
  EmberCommandEntry *commandFinger;
  printCommandUsage(entry);

  if ( getNestedCommand(entry, &commandFinger) ) {
    for (; commandFinger->name != NULL; commandFinger++) {
      printf("  ");
      printCommandUsage(commandFinger);
    }
  }
}

void emberPrintCommandUsageNotes(void)
{

	printf( "Usage:\r\n"
                    "<int>: 123 or 0x1ABC\r\n"
                    "<string>: \"foo\" or {0A 1B 2C}\r\n\r\n");

}

void emberPrintCommandTable(void)
{
  EmberCommandEntry *commandFinger = emberCommandTable;

  emberPrintCommandUsageNotes();
  for (; commandFinger->name != NULL; commandFinger++) {
    printCommandUsage(commandFinger);
  }
}

void emberCommandErrorHandler(EmberCommandStatus status)
{
  printf("%s\r\n", emberCommandErrorNames[status]);

  if (emberCurrentCommand == NULL) {
		if (isPrintEnabled){
	  emberPrintCommandTable();
		}
  } else {
    uint8_t *finger;
    uint8_t tokenNum, i;
    emberPrintCommandUsageNotes();
    // Reconstruct any parent commands from the buffer.
    for (tokenNum = 0; tokenNum < commandState.argOffset - 1; tokenNum++) {
      finger = tokenPointer(tokenNum);
      for (i = 0; i < tokenLength(tokenNum); i++) {
        printf( "%c", finger[i]);
      }
      printf(" ");
    }
    if (isPrintEnabled){
    emberPrintCommandUsage(emberCurrentCommand);
    }
  }
}


