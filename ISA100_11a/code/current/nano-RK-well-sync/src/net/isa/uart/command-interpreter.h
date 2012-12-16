/** @file command-interpreter2.h
* @brief Processes commands coming from the serial port.
* See @ref commands2 for documentation.
*
* <!-- Culprit(s): Richard Kelsey, Matteo Paris -->
*
* <!-- Copyright 2004-2007 by Ember Corporation.  All rights reserved.  *80* -->
*/

#ifndef __COMMAND_INTERPRETER2_H__
#define __COMMAND_INTERPRETER2_H__

/** @addtogroup commands2
* Interpret serial port commands. See command-interpreter2.c for source code.
*
* See the following application usage example followed by a brief explanation.
* @code
*
* // Usage: network form 22 0xAB12 -3 { 00 01 02 A3 A4 A5 A6 A7 }
* void formCommand(void)
* {
*   int8u channel = emberUnsignedCommandArgument(0);
*   int16u panId  = emberUnsignedCommandArgument(1);
*   int8s power   = emberSignedCommandArgument(2);
*   int8u length;
*   int8u *eui64  = emberStringCommandArgument(3, &length);
*   ...
*   ... call emberFormNetwork() etc
*   ...
* }
*
* // The main command table.
* EmberCommandEntry emberCommandTable[] = {
*   { "network",  (CommandAction)networkCommands, "n",  "network commands" },
*   { "status",   statusCommand,                  "",   "app status" },
*   ...
*   { NULL }
  };
*
* // The table of network commands.
* EmberCommandEntry networkCommands[] = {
*   { "form",       formCommand, "uvsh" },
*   { "join",       joinCommand, "uvsh" },
*   ...
*   { NULL }
  };
*
* void main(void)
* {
*    emberCommandReaderInit();
*    while(0) {
*      ...
*      // Process input and print prompt if it returns TRUE.
*      if (emberProcessCommandInput(serialPort)) {
*         emberSerialPrintf(1, "%p>", PROMPT);
*      }
*      ...
*    }
* }
* @endcode
*
* -# Applications specify the commands that can be interpreted
*    by defining the emberCommandTable array of type ::EmberCommandEntry.
*    The table includes the following information for each command:
*   -# The full command name.
*   -# Your application's function name that implements the command.
*   -# An ::EmberCommandEntry::argumentTypes string specifies the number and types of arguments
*      the command accepts.  See ::argumentTypes for details.
*   -# A description string explains the command.
*   .
* -# A default error handler ::emberCommandErrorHandler() is provided to
*    deal with incorrect command input. Applications may override it.
* -# The application calls ::emberCommandReaderInit() to initalize, and
*    ::emberProcessCommandInput() in its main loop.
* -# Within the application's command functions, use emberXXXCommandArgument()
*    functions to retrieve command arguments.
*
* The command interpreter does extensive processing and validation of the
* command input before calling the function that implements the command.
* It checks that the number, type, syntax, and range of all arguments
* are correct.  It performs any conversions necessary (for example,
* converting integers and strings input in hexadecimal notation into
* the corresponding bytes), so that no additional parsing is necessary
* within command functions.  If there is an error in the command input,
* ::emberCommandErrorHandler() is called rather than a command function.
*
* The command interpreter allows inexact matches of command names.  The
* input command may be either shorter or longer than the actual command.
* However, if more than one inexact match is found and there is no exact
* match, an error of type EMBER_CMD_ERR_NO_SUCH_COMMAND will be generated.
* To disable this feature, define EMBER_REQUIRE_EXACT_COMMAND_NAME in the
* application configuration header.
*
*@{
*/

/** @name Command Table Settings
 *@{
 */
#ifndef EMBER_MAX_COMMAND_ARGUMENTS
/** The maximum number of arguments a command can have.  A nested command
 * counts as an argument.
 */
#define EMBER_MAX_COMMAND_ARGUMENTS 10
#endif

#ifndef EMBER_COMMAND_BUFFER_LENGTH
#define EMBER_COMMAND_BUFFER_LENGTH 32
#endif

/** @} // END name group
 */

#define PGM     const
#define PGM_P   const char *
#define PGM_PU  const unsigned char *


// The (+ 1) takes into account the leading command.
#define MAX_TOKEN_COUNT (EMBER_MAX_COMMAND_ARGUMENTS + 1)

typedef void (*CommandAction)(void);
typedef PGM struct {
  /** Use letters, digits, and underscores, '_', for the command name.
   * Command names are case-sensitive.
   */
  PGM_P name;
  /** A reference to a function in the application that implements the
   *  command.
   *  If this entry refers to a nested command, then action field
   *  has to be set to NULL.
   */
  CommandAction action;
  /**
   * In case of normal (non-nested) commands, argumentTypes is a
   * string that specifies the number and types of arguments the
   *  command accepts.  The argument specifiers are:
   *  - u:   one-byte unsigned integer.
   *  - v:   two-byte unsigned integer
   *  - w:   four-byte unsigned integer
   *  - s:   one-byte signed integer
   *  - b:   string.  The argument can be entered in ascii by using
   *         quotes, for example: "foo".  Or it may be entered
   *         in hex by using curly braces, for example: { 08 A1 f2 }.
   *         There must be an even number of hex digits, and spaces
   *         are ignored.
   *  - *:   zero or more of the previous type.
   *         If used, this must be the last specifier.
   *  - ?:   Unknown number of arguments. If used this must be the only
   *         character. This means, that command interpreter will not
   *         perform any validation of arguments, and will call the
   *         action directly, trusting it that it will handle with
   *         whatever arguments are passed in.
   *  Integer arguments can be either decimal or hexidecimal.
   *  A 0x prefix indicates a hexidecimal integer.  Example: 0x3ed.
   *
   *  In case of a nested command (action is NULL), then this field
   *  contains a pointer to the nested EmberCommandEntry array.
   */
  PGM_P argumentTypes;
  /** A description of the command.
   */
  PGM_P description;
} EmberCommandEntry;

/**
 * A pointer to the currently matching command entry.
 * Only valid from within a command function.
 * If the original command was nested, points to the
 * final (non-nested) command entry.
 */
extern EmberCommandEntry *emberCurrentCommand;

extern EmberCommandEntry emberCommandTable[];

/**
 * Configuration byte.
 */
extern uint8_t emberCommandInterpreter2Configuration;

#define EMBER_COMMAND_INTERPRETER_CONFIGURATION_ECHO (0x01)


typedef uint8_t EmberCommandStatus;
enum
{
  EMBER_CMD_SUCCESS,
  EMBER_CMD_ERR_PORT_PROBLEM,
  EMBER_CMD_ERR_NO_SUCH_COMMAND,
  EMBER_CMD_ERR_WRONG_NUMBER_OF_ARGUMENTS,
  EMBER_CMD_ERR_ARGUMENT_OUT_OF_RANGE,
  EMBER_CMD_ERR_ARGUMENT_SYNTAX_ERROR,
  EMBER_CMD_ERR_STRING_TOO_LONG,
  EMBER_CMD_ERR_INVALID_ARGUMENT_TYPE
};

/** @name Functions to Retrieve Arguments
 * Use the following functions in your functions that process commands to
 * retrieve arguments from the command interpreter.
 * These functions pull out unsigned integers, signed integers, and strings,
 * and hex strings.  Index 0 is the first command argument.
 *@{
 */

/** Returns the number of arguments for the current command. */
uint8_t emberCommandArgumentCount(void);

/** Retrieves unsigned integer arguments. */
uint32_t emberUnsignedCommandArgument(uint8_t argNum);

/** Retrieves signed integer arguments. */
int16_t emberSignedCommandArgument(uint8_t argNum);

/** Retrieve quoted string or hex string arguments.
 * Hex strings have already been converted into binary.
 * To retrieve the name of the command itself, use an argNum of -1.
 * For example, to retrieve the first character of the command, do:
 * int8u firstChar = emberStringCommandArgument(-1, NULL)[0].
 * If the command is nested, an index of -2, -3, etc will work to retrieve
 * the higher level command names.
 */
uint8_t *emberStringCommandArgument(int8_t argNum, uint8_t *length);

/** Copies the string argument to the given destination up to maxLength.
 * If the argument length is nonzero but less than maxLength
 * and leftPad is TRUE, leading zeroes are prepended to bring the
 * total length of the target up to maxLength.  If the argument
 * is longer than the maxLength, it is truncated to maxLength.
 * Returns the minimum of the argument length and maxLength.
 *
 * This function is commonly used for reading in hex strings
 * such as EUI64 or key data and left padding them with zeroes.
 * See ::emberCopyKeyArgument and ::emberCopyEui64Argument for
 * convenience macros for this purpose.
 */
uint8_t emberCopyStringArgument(int8_t argNum,
                              uint8_t *destination,
                              uint8_t maxLength,
                              bool leftPad);

/** A convenience macro for copying security key arguments to an
 * EmberKeyData pointer.
 */
#define emberCopyKeyArgument(index, keyDataPointer)            \
  (emberCopyStringArgument((index),                            \
                           emberKeyContents((keyDataPointer)), \
                           EMBER_ENCRYPTION_KEY_SIZE,          \
                           TRUE))

/** A convenience macro for copying eui64 arguments to an EmberEUI64. */
#define emberCopyEui64Argument(index, eui64) \
  (emberCopyStringArgument((index), (eui64), EUI64_SIZE, TRUE))

/** @} // END name group
 */

/** The application may implement this handler.  To override
 * the default handler, define EMBER_APPLICATION_HAS_COMMAND_ERROR_HANDLER
 * in the CONFIGURATION_HEADER.  Defining this will also remove the
 * help functions ::emberPrintCommandUsage(), ::emberPrintCommandUsageNotes(),
 * and ::emberPrintCommandTable().
 */
void emberCommandErrorHandler(EmberCommandStatus status);
void emberPrintCommandUsage(EmberCommandEntry *entry);
void emberPrintCommandUsageNotes(void);
void emberPrintCommandTable(void);

/** @brief Initialize the command interpreter.
 */
void emberCommandReaderInit(void);

/** @brief Process the given string as a command.
 */
bool emberProcessCommandString(uint8_t *input, uint8_t sizeOrPort);

/** @brief Process input coming in on the given serial port.
 * @return TRUE if an end of line character was read.
 * If the application uses a command line prompt,
 * this indicates it is time to print the prompt.
 * @code
 * void emberProcessCommandInput(int8u port);
 * @endcode
 */
#define emberProcessCommandInput(port) \
  emberProcessCommandString(NULL, port)

/** @brief Turn echo of command line on.
 */
#define emberCommandInterpreterEchoOn()                 \
  (emberCommandInterpreter2Configuration                \
   |= EMBER_COMMAND_INTERPRETER_CONFIGURATION_ECHO)

/** @brief Turn echo of command line off.
 */
#define emberCommandInterpreterEchoOff()                \
  (emberCommandInterpreter2Configuration                \
   &= (~EMBER_COMMAND_INTERPRETER_CONFIGURATION_ECHO))

/** @brief Returns true if echo is on, false otherwise.
 */
#define emberCommandInterpreterIsEchoOn()               \
  (emberCommandInterpreter2Configuration                \
   & EMBER_COMMAND_INTERPRETER_CONFIGURATION_ECHO)
/** @} // END addtogroup
*/

#endif // __COMMAND_INTERPRETER2_H__
