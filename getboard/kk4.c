/* dgtnix, a POSIX driver for the Digital Game Timer chess board and clock
   Copyright (C) 2006 Pierre Boulenguez

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include </home/kevin/lincoln2/board/dgtnix-1.9.1/config.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

/* Includes for the virtual board socket
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <stdio.h>
#include <signal.h>
#include <netdb.h>

#include </home/kevin/lincoln2/board/dgtnix-1.9.1/include/dgtnix.h>

int dgtnix_errno=0;

/* The version of the dgtnix driver version as returned by the dgtnixQueryDriverVersion() function */

/* #define _DGTNIX_DRIVER_VERSION 1.8 */


#ifndef VERSION
#error "VERSION, _DGTNIX_DRIVER_VERSION not defined !"
#endif


#define _DGTNIX_DRIVER_VERSION VERSION


/* Internal chess pieces definitions */
#define _DGTNIX_EMPTY       0x00
#define _DGTNIX_WPAWN       0x01
#define _DGTNIX_WROOK       0x02
#define _DGTNIX_WKNIGHT     0x03
#define _DGTNIX_WBISHOP     0x04
#define _DGTNIX_WKING       0x05
#define _DGTNIX_WQUEEN      0x06
#define _DGTNIX_BPAWN       0x07
#define _DGTNIX_BROOK       0x08
#define _DGTNIX_BKNIGHT     0x09
#define _DGTNIX_BBISHOP     0x0a
#define _DGTNIX_BKING       0x0b
#define _DGTNIX_BQUEEN      0x0c

/* Message sent to the board */
#define _DGTNIX_SEND_CLK        0x41
#define _DGTNIX_SEND_BRD        0x42
#define _DGTNIX_SEND_UPDATE     0x43
#define _DGTNIX_SEND_UPDATE_BRD 0x44
#define _DGTNIX_SEND_SERIALNR 0x45
#define _DGTNIX_SEND_BUSADDRESS 0x46
#define _DGTNIX_SEND_TRADEMARK  0x47
#define _DGTNIX_SEND_VERSION  0x4d
#define _DGTNIX_SEND_UPDATE_NICE 0x4b
#define _DGTNIX_SEND_EE_MOVES   0x49
#define _DGTNIX_SEND_RESET      0x40

#define _DGTNIX_MESSAGE_BIT         0x80
#define _DGTNIX_MSG_SERIALNR       (_DGTNIX_MESSAGE_BIT|_DGTNIX_SERIALNR)
#define _DGTNIX_SIZE_SERIALNR      12
#define _DGTNIX_MSG_BUSADDRESS      (_DGTNIX_MESSAGE_BIT|_DGTNIX_BUSADDRESS)
#define _DGTNIX_SIZE_BUSADDRESS   24
#define _DGTNIX_MSG_VERSION      (_DGTNIX_MESSAGE_BIT|_DGTNIX_VERSION)
#define _DGTNIX_SIZE_VERSION   24 
#define _DGTNIX_MSG_BWTIME      (_DGTNIX_MESSAGE_BIT|_DGTNIX_BWTIME)
#define _DGTNIX_SIZE_BWTIME     10
#define _DGTNIX_MSG_BOARD_DUMP  (_DGTNIX_MESSAGE_BIT|_DGTNIX_BOARD_DUMP)

#define _DGTNIX_SIZE_BOARD_DUMP 67
#define _DGTNIX_NONE            0x00
#define _DGTNIX_BOARD_DUMP      0x06
#define _DGTNIX_BWTIME          0x0d
#define _DGTNIX_FIELD_UPDATE    0x0e
#define _DGTNIX_EE_MOVES        0x0f
#define _DGTNIX_BUSADDRESS        0x10
#define _DGTNIX_SERIALNR        0x11
#define _DGTNIX_TRADEMARK       0x12
#define _DGTNIX_VERSION         0x13

#define _DGTNIX_VIRTUAL_BOARD 0x10
#define _DGTNIX_REAL_BOARD 0x20

/* Size of the internal g_readBuffer array */
#define READBUFFERSIZE 512

/*********************************/
/* Intern functions declarations */
/*********************************/

static void* _threadManagedFunc(void *);
static void _sendMessageToBoard(int);
static int _readMessageFromBoard();
static void _sendMessageToEngine(const char*, size_t);
static int _debug(const char *, ...);
static int _closeDescriptor(int *);
static int _closeAllDescriptors();
static int _mySleep(double);
static char _convertInternalPieceToExternal(char);
static int _queryVendorStrings();
static void _dumpBoard(const char *);
static void _fieldUpdateReceived(int, char );
static void _bwtimeReceived(unsigned char [6]);
static void _assertDriverInitialised(const char *);
static int _setTTY(const char *);
static int _setUnixSocket(const char *);
static void _setBoardOrientation(unsigned int orientation);
static void _setDebugMode(unsigned int value);
/****************************************/
/* Intern global variables declarations */
/****************************************/
/* Internal representation of the board, synced with DGT */
static char g_board[64];
/* The board returned by dgtnixGetBoard(). It is a copy of g_board but 
   converted with _convertInternalPieceToExternal(...) */
static char g_transmitedBoard[64];
/* The string to print before the debug message */
static const char *g_debugString="dgtnix-debug:";
/* Descriptor for the running thread */
static pthread_t g_driverThread;
/* Buffer for the chars read on the board-driver file */
static unsigned char g_readBuffer[READBUFFERSIZE];
/* Descriptor of the  board-driver communication file */
static int g_descriptorDriverBoard=-1;
/* Descriptor of the communication file as returned by dgtnixInit(...) */
static int g_pipeEngineReadSide=-1;
/* Descriptor of the communication file on the driver to the engine side */ 
static int g_pipeDriverWriteSide=-1;
/* Buffer to store the serial number, returned by dgtnixQueryString(DGTNIX_SERIAL_STRING) */
static char g_serialBuffer[_DGTNIX_SIZE_SERIALNR+1];
/* Buffer to store the version of the board, returned by dgtnixQueryString(DGTNIX_VERSION_STRING) */
static char g_versionBuffer[_DGTNIX_SIZE_VERSION+1];
/* Buffer to store the bus adress of the board, returned by dgtnixQueryString(DGTNIX_BUSADDRESS_STRING) */
static char g_busadressBuffer[_DGTNIX_SIZE_BUSADDRESS+1];
/* Buffer to store the trademark of the board, returned by dgtnixQueryString(DGTNIX_TRADEMARK_STRING) */
static char g_trademarkBuffer[257];
/* Flag to test wether the version string was already queryed to the board */
static char g_versionFlag;
/* Flag to test wether the serial string was already queryed to the board */
static char g_serialFlag;
/* Flag to test wether the trademark string was already queryed to the board */
static char g_trademarkFlag;
/* Flag to test wether the busadress string was already queryed to the board */
static char g_busadressFlag;
/* Flag for the verbose debug mode */
static char g_debugMode=DGTNIX_DEBUG_OFF;
/* Flag set to the real board or the virtual board */
static char g_virtualBoardMode=_DGTNIX_REAL_BOARD;
/* Used by dgtnixGetBoard(...), 
   if the board has not changed it is not necessary to copy g_board to g_transmitedBoard */
static char g_boardUpdated;
/* The time received from the DGT clock for white player (in seconds) */
static  int g_wtime;
/* The time received from the DGT clock for black player (in seconds) */
static int g_btime;
/* Regarding the button pressed on the DGT clock, wether it is white turn's or not */
static  int g_wturn;
/* Control the orientation of the board, can be DGTNIX_BOARD_ORIENTATION_CLOCKLEFT or DGTNIX_BOARD_ORIENTATION_CLOCKRIGHT */
static  char g_boardOrientation=DGTNIX_BOARD_ORIENTATION_CLOCKLEFT;
/* simply ensure that the driver was initialised with dgtnixInit( ... ) */
static char g_initialised=0;
/* This mutex is used by to ensure that during a dgtnixGetBoard(...) call, the board is'nt updated */
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

/**************************************/
/* Intern function begins with _...   */
/**************************************/
/* A redefinition of the function sleep, 
 * with double precision, sleep_time in seconds.  
 */
static int _mySleep (double sleep_time)
{
  struct timespec tv;
  /* Construct the timespec from the number of whole seconds... */
  tv.tv_sec = (time_t) sleep_time;
  /* ... and the remainder in nanoseconds. */
  tv.tv_nsec = (long) ((sleep_time - tv.tv_sec) * 1e+9);

  while (1)
    {
      /* Sleep for the time specified in tv. If interrupted by a
	 signal, place the remaining time left to sleep back into tv. */
      int rval = nanosleep (&tv, &tv);
      if (rval == 0)
	/* Completed the entire sleep time; all done. */
	return 0;
      else if (errno == EINTR)
	/* Interrupted by a signal. Try again. */
	continue;
      else 
	/* Some other error; bail out. */
	return rval;
    }
  return 0;
}

/* 
 * Debug function equivalent to vprintf(stderr, ...) 
 * but append g_debugString at the beginning of the line
 * does nothing if g_debugMode=0. 
 * See dgtnixSetDebugMode(...) 
 */
static int _debug(const char *format, ...)
{
  if( (g_debugMode == DGTNIX_DEBUG_ON) || (g_debugMode == DGTNIX_DEBUG_WITH_TIME))
    {
      fputs(g_debugString,stderr);
      if(dgtnix_errno!=0)
	{
	  fprintf(stderr, "dgtnix_errno:");
	  fputs(strerror(dgtnix_errno),stderr);
	  fprintf(stderr, ":");
	}
      va_list ap;
      va_start(ap, format);
      int res=vfprintf(stderr, format, ap);
      va_end(ap);
      return res;
    }
  return 0;
}

/* 
 * Convert a piece from the internal representation to 
 * the representation defined in dgtnix.h (ex:  white pawn = 'P').
 */
static char _convertInternalPieceToExternal(char c)
{
  switch(c)
    {
    case _DGTNIX_EMPTY: 
      return ' ';
    case _DGTNIX_WPAWN: 
      return 'P'; 
    case _DGTNIX_WROOK: 
      return 'R'; 
    case _DGTNIX_WKNIGHT: 
      return 'N'; 
    case _DGTNIX_WBISHOP: 
      return 'B'; 
    case _DGTNIX_WKING: 
      return 'K'; 
    case _DGTNIX_WQUEEN: 
      return 'Q'; 
    case _DGTNIX_BPAWN: 
      return 'p'; 
    case _DGTNIX_BROOK: 
      return 'r'; 
    case _DGTNIX_BKNIGHT: 
      return 'n'; 
    case _DGTNIX_BBISHOP: 
      return 'b'; 
    case _DGTNIX_BKING: 
      return 'k'; 
    case _DGTNIX_BQUEEN:
      return 'q'; 
    default:
      perror("dgtnix critical:");
      fprintf(stderr,"dgtnix convertInternalToExternal error :%c %d\n", c, c); 
      exit(-1);
    }
}

/*
 * queryVendorStrings
 * update the internal representation of 
 * all the board strings.
 */
static int _queryVendorStrings()
{
  _assertDriverInitialised("_queryVendorStrings()");

  if(g_serialFlag==0)
    {
      _sendMessageToBoard(_DGTNIX_SEND_SERIALNR);
    }
  if(g_busadressFlag==0)
    {
      _sendMessageToBoard( _DGTNIX_SEND_BUSADDRESS);
    }
  if(g_versionFlag==0)
    {
      _sendMessageToBoard(_DGTNIX_SEND_VERSION);
    }
  if(g_trademarkFlag==0)
    {
      _sendMessageToBoard( _DGTNIX_SEND_TRADEMARK);
    }
  return 1;
}

/*
 * Wrapper of the unix write(...) that sends a 
 * message to the DGT board.
 */
static void _sendMessageToBoard(int command)
{
  if(!(g_debugMode == DGTNIX_DEBUG_OFF))
    {
      switch(command)
	{
	case _DGTNIX_SEND_CLK: 
	  _debug("Sending _DGTNIX_SEND_CLK to the board\n"); break;       
	case _DGTNIX_SEND_BRD: 
	  _debug("Sending _DGTNIX_SEND_BRD to the board\n"); break;        
	case _DGTNIX_SEND_UPDATE:
	  _debug("Sending _DGTNIX_SEND_UPDATE to the board\n"); break;    
	case _DGTNIX_SEND_UPDATE_BRD: 
	  _debug("Sending _DGTNIX_SEND_UPDATE_BRD to the board\n"); break; 
	case _DGTNIX_SEND_SERIALNR: 
	  _debug("Sending _DGTNIX_SEND_SERIALNR to the board\n"); break;
	case _DGTNIX_SEND_BUSADDRESS: 
	  _debug("Sending _DGTNIX_SEND_BUSADDRESS to the board\n"); break;
	case _DGTNIX_SEND_TRADEMARK: 
	  _debug("Sending _DGTNIX_SEND_TRADEMARK to the board\n"); break;  
	case _DGTNIX_SEND_VERSION: 
	  _debug("Sending _DGTNIX_SEND_VERSION to the board\n"); break; 
	case _DGTNIX_SEND_UPDATE_NICE: 
	  _debug("Sending _DGTNIX_SEND_UPDATE_NICE to the board\n"); break; 
	case _DGTNIX_SEND_EE_MOVES: 
	  _debug("Sending _DGTNIX_SEND_EE_MOVES to the board\n"); break;  
	case _DGTNIX_SEND_RESET: 
	  _debug("Sending _DGTNIX_SEND_RESET to the board\n"); break;      
	default: 
	  {
	    perror("dgtnix critical:sendMessageToBoard");
	    fprintf(stderr,"unknown command %d\n", command);	 
	    exit(-1);
	  }
	}
      if(g_descriptorDriverBoard < 0)
	{
	  perror("dgtnix critical:sendMessageToBoard: invalid file descriptor\n");
	  exit(-1);
	}
    }
  if(write(g_descriptorDriverBoard,&command ,1)!=1)
    {
      perror("dgtnix critical:sendMessageToBoard: write() error\n");
      _closeDescriptor(&g_descriptorDriverBoard);
      exit(-1);
    }
}

/*
 * Wrapper of the unix function write(...) 
 * that sends a message to the chess engine.
 */
static void _sendMessageToEngine(const char*message, size_t length)
{
  if(g_pipeDriverWriteSide < 0)
    {
      perror("dgtnix critical:sendMessageToEngine: invalid descriptor\n");
      exit(-1);
    }
  if(write(g_pipeDriverWriteSide,(void *) message, length) != length)
    {
      perror("dgtnix critical:sendMessageToEngine: write error\n");
      exit(-1);
    }
}

/*
 * The main polling loop of the thread, 
 * waits for events on the ports. 
 * When they appear, call _readMessageFromBoard().
 */
static void *_threadManagedFunc(void *params)
{ 
  g_initialised = 1;
  _queryVendorStrings();
  _sendMessageToBoard(_DGTNIX_SEND_UPDATE);
  while( 1 ) 
    {  
      if(_readMessageFromBoard()<0)
	{
	  fprintf(stderr,"dgtnixManagerFunc:read error\n");
	  break;
	}
      /*_dumpBoard(g_board);*/
    }
  _closeAllDescriptors();
  g_initialised = 0;
  return params;
}

/*
 * Print the board in parameter (a char[64]) to stderr.
 */
static void _dumpBoard(const char *board)
{
  int square;
  for (square = 0; square < 64; square++)
    {
      if(!(square%8))
	{ 
	  if(!(square==0))
	    {
	      fprintf(stderr,"|\n");
	      fputs(g_debugString,stderr);
	    }
	  else
	    {
	      fputs(g_debugString,stderr);
	    }
	}
      fprintf(stderr,"|%c", _convertInternalPieceToExternal(board[square]));
    }
  fprintf(stderr,"|\n");
}

/*
 * Wrapper to close a descriptor
 * usefull to track if all opened descriptors have been closed.
 * also set the descriptor to -1
*/ 
static int _closeDescriptor(int *descriptor)
{
  if(close(*descriptor)<0)
    {
      dgtnix_errno= errno;
      _debug("close() < 0\n");
      /* As this usually  append with the socket */
      /* of the virtual board, let's do it ... */
      *descriptor = -1; 
      return -1;
    }
  *descriptor = -1;
  return 0;
}

/* 
 *  Manage the reception of the BWTIME message 
 *  function called only by _readMessageFromBoard()
 */
static void _bwtimeReceived(unsigned char buffer[7])
{
  int j;
  for (j = 0; j < 6; j++)
    buffer[j] = (buffer[ j] >> 4) * 10 + (buffer[ j] & 15);
  if( ((buffer[0] & 15)==8) && (buffer[1] ==0) && ( buffer[2] ==0))
    g_btime = 0;
  else
    g_btime = (buffer[0] & 15) * 3600 + buffer[1] * 60 + buffer[2];
  if( ((buffer[3] & 15)==8) && (buffer[4] ==0) && ( buffer[5] ==0))
    g_wtime = 0;
  else
    g_wtime = (buffer[3] & 15) * 3600 + buffer[4] * 60 + buffer[5];
  if (!(buffer[6] & 1))
    {
      g_btime=g_wtime=g_wturn=-1;
      if(g_debugMode == DGTNIX_DEBUG_WITH_TIME) 
	_debug("(no clock found)\n",g_wtime, g_btime);
    }
  else
    {
      if(g_debugMode == DGTNIX_DEBUG_WITH_TIME) 
	_debug("%ds, %ds\n",g_wtime, g_btime);
      if (buffer[ 6] & 8)
	{
	  g_wturn = 0;
	  if(g_debugMode == DGTNIX_DEBUG_WITH_TIME) 
	    _debug("black's turn\n");
	}
      else
	{
	  g_wturn = 1;
	  if(g_debugMode == DGTNIX_DEBUG_WITH_TIME) 
	    _debug("white's turn\n");
	}
    }
  /* If it is pertinent, send a DGTNIX_MSG_TIME to the chess engine */
  if( g_btime!=-1)
    {
      char code = DGTNIX_MSG_TIME;
      _sendMessageToEngine(&code, 1);
      if(g_debugMode ==  DGTNIX_DEBUG_WITH_TIME) 
	_debug("Sending char DGTNIX_MSG_TIME to the engine \n");
    }
}

/* 
 *  Manage the reception of the FIELD_UPDATE message 
 *  function called only by _readMessageFromBoard()
 */
static void _fieldUpdateReceived(int mposition, char mpiece)
{
  /*_debug("_fieldUpdateReceived %d %c %d\n", mposition, mpiece, mpiece); */
  /* Remove = 1 if the move is a piece removal 
     else 0 (a piece was added )  */
  int remove;
  if(_convertInternalPieceToExternal(mpiece) == ' ')
    remove = 1;
  else
    remove = 0;
  /* Explicit the piece, column and line that are 
   * concerned by the _DGTNIX_FIELD_UPDATE */
  char intern_column;
  char intern_line;
  char board_column = 'A'+ (mposition % 8); 
  char board_line = 8 - (mposition / 8);
  
  if (g_boardOrientation == DGTNIX_BOARD_ORIENTATION_CLOCKLEFT)
    {
      intern_column = 'A'+ (mposition % 8);
      intern_line = 8 - (mposition / 8);
    }
  else if(g_boardOrientation == DGTNIX_BOARD_ORIENTATION_CLOCKRIGHT)
    {
      intern_column = 'H'- (mposition % 8);
      intern_line = (mposition / 8)+1;
    }
  else
    {
      fprintf(stderr, "dgtnix critical, unrecognized board orientation\n");
      exit(-1);
    }
  char piece;
  if(remove)
    piece = _convertInternalPieceToExternal(g_board[mposition]);
  else
    piece =_convertInternalPieceToExternal(mpiece);
  
  /* A piece was removed from a square */
  if(remove)
    _debug("%c removed from %c%d on the board\n",piece, board_column, board_line);
  /* A piece was added on a square */
  else
    _debug("%c added on %c%d on the board\n", piece, board_column, board_line);
  
  /* Update the internal representation of the board */
  /* this portion is mutexed to protect the board representation  */
  pthread_mutex_lock( &g_mutex );
  g_board[mposition] = mpiece;
  g_boardUpdated=1;
  pthread_mutex_unlock( &g_mutex );
  /* End debug details */
  /* Send the message 'M' followed by the intern_column,
   * the intern_line and the piece to the chess engine */ 
  char code;
  if(remove)
    code = DGTNIX_MSG_MV_REMOVE;
  else
    code = DGTNIX_MSG_MV_ADD;
  char message[4];
  message[0] = code;
  message[1] = intern_column;
  message[2] = intern_line;
  message[3] = piece;
  _sendMessageToEngine(message, 4);
  if(remove)
    {
      _debug("Sending DGTNIX_MSG_MV_REMOVE (%c on %c%d) to the engine \n",piece, intern_column, intern_line);
      code = DGTNIX_MSG_MV_REMOVE;
    }
  else
    {
      _debug("Sending DGTNIX_MSG_MV_ADD (%c on %c%d) to the engine \n",piece, intern_column, intern_line);
      code = DGTNIX_MSG_MV_ADD;
    }
   
}


/*
 * The main read function, called by the _threadManagerFunction when there are chars to be read
 * identify the message on the port, update the intern board representation and reemit a message 
 * to the engine.
 */
static int _readMessageFromBoard()
{
  if(g_descriptorDriverBoard<0)
    {
      _debug("read(g_descriptorDriverBoard, header, 1) 0 :int readMessageFromBoard(int g_descriptorDriverBoard):invalid descriptor\n");
      return -1;
    }
  int  j = 0;
  unsigned char header[3];
  int charRead;
  unsigned int commandID;
  int messageLength=0.;
  int i=0;
  
  /* Clean the buffer */
  for(i=0; i < READBUFFERSIZE; i++)
    g_readBuffer[i] = -10;
  
  /* first character, MESSAGE ID one byte, MSB (MESSAGE BIT) always 1 */
  if( (charRead = read(g_descriptorDriverBoard, header, 1) ) < 0)
    {
      dgtnix_errno = errno;
      _debug("read(g_descriptorDriverBoard, header, 1) -1- int readMessageFromBoard(int g_descriptorDriverBoard)\n");
      return -1;
    }
  if( !(header[0] & 128) )
    {
      /* Invalid character */
      _debug("invalid message -2- int readMessageFromBoard(int g_descriptorDriverBoard) :%c %d\n", header[0], header[0]);
      return -1;
    }
  commandID = header[0] & 127;
  
  /* Second character, MSB of MESSAGE SIZE one byte, 
     MSB always 0, carrying D13 to D7 of the  total message length, 
     including the 3 header byte */
  if( (charRead = read(g_descriptorDriverBoard, &header[1], 1) ) < 0)
    {
      dgtnix_errno = errno;
      _debug("read(g_descriptorDriverBoard, header, 1) -3- int readMessageFromBoard(int g_descriptorDriverBoard)\n");
      return -1;
    }
  if( header[1] & 128 )
    {
      _debug("invalid message -4- int readMessageFromBoard(int g_descriptorDriverBoard) :%c\n", header[1]);
      return -1;
    }
  
  /* Third character, LSB of MESSAGE SIZE one byte, 
     MSB always 0, carrying  D6 to D0 of the total message length, 
     including the 3 header charRead */
  if( (charRead = read(g_descriptorDriverBoard, &header[2], 1) ) < 0)
    {
      dgtnix_errno = errno;
      _debug("read(g_descriptorDriverBoard, header, 1) -5- int readMessageFromBoard(int g_descriptorDriverBoard)\n");
      return -1;
    }
  if( header[2] & 128 )
    {
      _debug("invalid message -6- int readMessageFromBoard(int g_descriptorDriverBoard) :%c %d\n", header[2], header[2]);
      return -1;
    }
  
  messageLength=(header[1] << 7) + header[2];
  messageLength-=3;
  /* Usefull debug stuff */  
  /*
  printf("longueur:%d\nheader[1]:%d\nheader[1]<<7:%d\nheader[2]:%d\n"
	 ,messageLength
	 ,header[1]
	 ,header[1] << 7
	 ,header[2]);
  */
  int tmp1=0; 
  int tmp2;
  while(tmp1 < messageLength)
    {
      if((tmp2 = read(g_descriptorDriverBoard, g_readBuffer + tmp1, messageLength-tmp1 ) )< 0 ) 
	{
	  dgtnix_errno=errno;
	  _debug("read(g_descriptorDriverBoard, buffer + tmp, messageLength ) -7- int readMessageFromBoard(int g_descriptorDriverBoard)\n");
	  return -1;
	}
      tmp1 += tmp2; 
    }
  switch (commandID) 
    {
    case _DGTNIX_NONE:
      _debug("Received _DGTNIX_NONE from the board\n");
      break;
    case _DGTNIX_BOARD_DUMP:
      _debug("Received _DGTNIX_BOARD_DUMP from the board\n");
      pthread_mutex_lock( &g_mutex );
      for (j = 0; j < 64; j++) 
	{
	  g_board[j] = g_readBuffer[j];
	  
	}
      pthread_mutex_unlock( &g_mutex );
      if(! (g_debugMode  ==  DGTNIX_DEBUG_OFF) )
	_dumpBoard(g_board);
      g_boardUpdated=1;
      break;
    case _DGTNIX_BWTIME:
      if(g_debugMode  ==  DGTNIX_DEBUG_WITH_TIME) 
	_debug("Received _DGTNIX_BWTIME from the board\n");
      _bwtimeReceived(g_readBuffer);
      break;
    case _DGTNIX_FIELD_UPDATE:
      _debug("Received _DGTNIX_FIELD_UPDATE from the board\n");
      _fieldUpdateReceived(g_readBuffer[0], g_readBuffer[1]);
      break;
    case _DGTNIX_EE_MOVES:
      _debug("Received _DGTNIX_EE_MOVES from the board\n");
      break;
    case _DGTNIX_BUSADDRESS:
      _debug("Received _DGTNIX_BUSADDRESS from the board\n");
      snprintf(g_busadressBuffer,_DGTNIX_SIZE_BUSADDRESS, "%#x-%#x", g_readBuffer[0], g_readBuffer[1]);
      g_busadressFlag=1;
      _debug("bus address %#x-%#x\n", g_readBuffer[0], g_readBuffer[1]);
      break;  
    case _DGTNIX_SERIALNR:
      _debug("Received _DGTNIX_SERIALNR from the board\n");
      for (j = 0; j < messageLength; j++)
	g_serialBuffer[j] = g_readBuffer[j];
      g_serialBuffer[messageLength]='\0';
      g_serialFlag=1;
      _debug("serial number %s\n",  g_serialBuffer);
      break;
    case _DGTNIX_TRADEMARK:
      _debug("Received _DGTNIX_TRADEMARK from the board\n");
      for (j = 0; j < messageLength; j++)
	g_trademarkBuffer[j] = g_readBuffer[j];
      g_trademarkBuffer[messageLength]='\0';
      g_trademarkFlag=1;
      _debug("trademark %s:\n", g_trademarkBuffer);
      break;
/*
    case _DGTNIX_VERSION:
      _debug("Received _DGTNIX_VERSION from the board\n");
      g_versionBuffer[0]=g_readBuffer[0];
      g_versionBuffer[1]=g_readBuffer[1];
      float v = ((float)(g_readBuffer[0])) + 0.1 * ((float)(g_readBuffer[1]));
      snprintf(g_versionBuffer,_DGTNIX_SIZE_VERSION, "%f", v);
      g_versionFlag=1;
      _debug("version %2d.%02d\n", g_readBuffer[0], g_readBuffer[1]);
      break;
*/
    default:
      {
	perror("dgtnix critical: unknown response from the board\n");
	fprintf(stderr,"unknown response: (%x)\n", commandID);
	exit(-1);
      }
    }
  return 1;
}

/**
 * this function is called in the beginning of some of the 
 * dgtnix... functions.
 * It simply ensure that dgtnixInit(..) was sucessfully called before 
 */
static void _assertDriverInitialised(const char *message)
{
  if(g_initialised != 1)
    {
      perror("dgtnix critical: _assertDriverInitialised\n");
      fprintf(stderr
	      ,"dgtnix:Function %s was called without a successful dgtnixInit()!\n"
	      ,message);
      exit(-1);
    }
}

/**
 * Do all the low level
 * stuff to access the RS232 port
 * or the virtual COM port by ftdi
 * If executed with sucess, 
 * g_descriptorDriverBoard contain the descriptor of the port.
 */
static int _setTTY(const char *port)
{
  /* Open the tty for read/write */
  struct termios trm;
  int set, retval;
  g_descriptorDriverBoard = open(port, O_RDWR | O_NOCTTY );
  if (g_descriptorDriverBoard < 0) 
    {
      dgtnix_errno = errno;
      /* keep msg to client app */
      _debug("unable to open tty (open(%s) returns %d) in function setTTY\n", port, g_descriptorDriverBoard);
      return -1;
    }
  ioctl(g_descriptorDriverBoard, TIOCMGET, &set);
  /* DTR high */
  set |= TIOCM_DTR;    
  ioctl(g_descriptorDriverBoard, TIOCMSET, &set);
  /* flush buffers */
  tcflush(g_descriptorDriverBoard, TCIOFLUSH);    
  retval = tcgetattr(g_descriptorDriverBoard, &trm);
  /* input speed 9600 bds */
  cfsetispeed(&trm, B9600);      
  /* output speed 9600 bds */
  cfsetospeed(&trm, B9600);     
  /* These lines and below are equivalent to the BSD function cfmakeraw(trm) */
  trm.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
		   |INLCR|IGNCR|ICRNL|IXON);
  trm.c_oflag &= ~OPOST;
  trm.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
  trm.c_cflag &= ~(CSIZE|PARENB);
  trm.c_cflag |= CS8; 
  /* end cfmakeraw  */
  if((retval=tcsetattr(g_descriptorDriverBoard, TCSANOW, &trm))<0)
    {
      dgtnix_errno = errno;
      _debug("unable to set attributes for tty (tcsetattr(%s,...) return %d):setTTY\n", port, retval);
      /* keep msg to client app */
      _closeDescriptor(&g_descriptorDriverBoard);
      return -1;
    }
  tcflush(g_descriptorDriverBoard, TCIOFLUSH); 
  return g_descriptorDriverBoard;
}

/*S_ISCHR(m)*/


/*set a AF_UNIX socket as the fake board
 */
static int _setUnixSocket(const char *port)
{
  if ((g_descriptorDriverBoard = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    perror("dgtnix critical: _setFakeTTY:");
    exit(-1);
  }
  
  struct  sockaddr_un servaddr;/* address of server */
  servaddr.sun_family = AF_UNIX;
  strcpy(servaddr.sun_path, port);

/*
  if (connect(g_descriptorDriverBoard, &servaddr, sizeof( struct  sockaddr_un)) < 0) {
    dgtnix_errno = errno;
    close(g_descriptorDriverBoard);
    g_descriptorDriverBoard=-1;
    _debug("Unable to connect socket for virtual board\n");
    return -1;
  }  
*/

  return g_descriptorDriverBoard;
}

/* Close the three opened descriptors
 */
static int _closeAllDescriptors()
{
  int retval = 1;
  /* clear engine descriptor */
  if(g_pipeEngineReadSide>0 && _closeDescriptor(&g_pipeEngineReadSide) < 0 )
    {
      dgtnix_errno = errno;
      _debug("close g_pipeEngineReadSide - int dgtWriteCOMPort (int port)\n");
      retval = -1;
    }
  /* clear driver descriptor */
  if(g_pipeDriverWriteSide>0 && _closeDescriptor(&g_pipeDriverWriteSide)< 0 )
    {
      dgtnix_errno = errno;
      _debug("close g_pipeDriverWriteSide - int dgtnixWriteCOMPort (int port)\n");
      retval = -1;
    }
  /* clear g_descriptorDriverBoard */
  if(g_descriptorDriverBoard >0 && _closeDescriptor(&g_descriptorDriverBoard) < 0)
    {
      dgtnix_errno = errno;
      _debug("close g_descriptorDriverBoard - int dgtnixWriteCOMPort (int port)\n");
      retval = -1;
    }
  return retval;
}


static void _setDebugMode(unsigned int value)
{
  switch(value)
    {
    case DGTNIX_DEBUG_OFF:
      g_debugMode = DGTNIX_DEBUG_OFF;
      _debug("debug mode set to DGTNIX_DEBUG_OFF\n");
      break;
    case DGTNIX_DEBUG_ON:
      g_debugMode = DGTNIX_DEBUG_ON;
      _debug("debug mode set to DGTNIX_DEBUG_ON\n");
      break;
    case DGTNIX_DEBUG_WITH_TIME:
      g_debugMode = DGTNIX_DEBUG_WITH_TIME;
      _debug("debug mode set to DGTNIX_DEBUG_WITH_TIME\n");
      break;
    default:
      fprintf(stderr, "dgtnix :unrecognized debug mode\n");
      exit(-1);
    }
}

static void _setBoardOrientation(unsigned int orientation)
{
  pthread_mutex_lock( &g_mutex );
  switch(orientation)
    {
    case DGTNIX_BOARD_ORIENTATION_CLOCKLEFT:
      _debug("setting board orientation to DGTNIX_BOARD_ORIENTATION_CLOCKLEFT`n");
      g_boardOrientation = DGTNIX_BOARD_ORIENTATION_CLOCKLEFT;
      break;
    case DGTNIX_BOARD_ORIENTATION_CLOCKRIGHT:
      _debug("setting board orientation to DGTNIX_BOARD_ORIENTATION_CLOCKRIGHT\n");
      g_boardOrientation = DGTNIX_BOARD_ORIENTATION_CLOCKRIGHT;
      break;
    default:
      fprintf(stderr, "dgtnix critical, unrecognized board orientation\n");
      exit(-1);
    }
  pthread_mutex_unlock( &g_mutex );
}

/*********************************************************************************/
/* THE FUNCTIONS BELOW ARE PART OF THE INTERFACE AND ARE DESCRIBED IN dgtnix.h   */
/* they begin with dgtnix...                                                     */
/*********************************************************************************/

void dgtnixSetOption(unsigned long option, unsigned int value)
{
  switch(option)
    {
    case DGTNIX_DEBUG:
      _setDebugMode(value);
      break;
    case DGTNIX_BOARD_ORIENTATION:
      _setBoardOrientation(value);
      break;
    default:
      perror("dgtnix critical :dgtnixSetOption: invalid option\n");
      exit(-1);
    }
}

     
const char *dgtnixToPrintableBoard(const char *board)
{
  _assertDriverInitialised("dgtnixPrintBoard");
  static char dumped_board[256];
  int count=0;
  int square;
  count += snprintf(
		    dumped_board+count
		    , 256,
		    "   a b c d e f g h\n");
  for (square = 0; square < 64; square++)
    {
      if( !(square%8)  &&  !(square==0) )
	    {
	      count += snprintf(dumped_board+ count, 256 -  count, "|\n");
	    }
      if (!(square%8))
	{
	  count += snprintf(dumped_board + count,  256 -  count, "%d ", 8 -square/8 );
	}
      
      count += snprintf(dumped_board + count,  256 -  count, "|%c", board[square]);
    }
  count += snprintf(dumped_board + count, 256 -  count, "|\n");
  return dumped_board;
}

int dgtnixClose()
{
  _assertDriverInitialised("dgtnixClose");
  void *status;
  int rc;
  /* Kill Previous thread if exists */
  if(pthread_cancel(g_driverThread) == ESRCH)
    _debug("dgtnixClose() can not close the driver thread (no such thread)");
  rc = pthread_join(g_driverThread, &status);
  if (rc) 
    _debug("dgtnixClose() return code from pthread_join() is %d\n", rc);
  
  _closeAllDescriptors();
  g_initialised=0;
  _debug("the driver is closed\n");
  return 1;
}

int dgtnixInit(const char *port)
{
  if(g_initialised != 0)
    _debug("Close driver first\n");
  int i;
  struct stat stats;
  if(stat(port, &stats)<0)
    {
      dgtnix_errno = errno;
      _debug("fstab < 0 for port %s\n", port);
      return -1;
    }
/*  printf("**1**\n");  */
  if( S_ISCHR(stats.st_mode) )
    {
      _debug("opening driver in normal mode\n");
      g_virtualBoardMode = _DGTNIX_REAL_BOARD;

/* KK setTTY is -1 */

      if( _setTTY(port) < 0)
	return -1;
    }
  else
    {
      _debug("opening driver in virtual mode\n");
      g_virtualBoardMode = _DGTNIX_VIRTUAL_BOARD;
      if( _setUnixSocket(port) < 0)
	return -1;
    }
/* printf("**2**\n"); */
  

  /* do some initialisation stuff 
   *
   * ...
   */
  g_serialBuffer[0]='\0';
  g_versionBuffer[0]='\0';
  g_busadressBuffer[0]='\0';
  g_trademarkBuffer[0]='\0';
  for(i=0;i<64;i++)
    {
      g_board[i] = _DGTNIX_EMPTY;
      g_transmitedBoard[i]= ' ';
    }
  g_versionFlag=0;
  g_serialFlag=0;
  g_trademarkFlag=0;
  g_busadressFlag=0;
  g_boardUpdated=0;
  g_btime=-1;
  g_wtime=-1;
  g_wturn=-1;
  
  /* 
   * do a reset of the 'presumed' board
   * Send _DGTNIX_SEND_BRD to the opened port and test the answer 
   */

/* printf("**3**\n"); */

  _sendMessageToBoard(_DGTNIX_SEND_RESET);

/* printf("**4**\n"); */

  _sendMessageToBoard(_DGTNIX_SEND_BRD);
  
/* printf("**5**\n"); */

  int sv[2];
  /* communication tube between the engine and the driver
   * using a pipe instead of a socketpair. A pipe is a one way 
   * communication port for the driver to the engine communication.
   * The other way beeing handled by calls to the API functions.
   */ 
  if(pipe(sv)<0)
    {
      dgtnix_errno = errno;
      _debug("pair:void dgtnixInit(const char *port)\n");
      return -1;
    }
  g_pipeEngineReadSide = sv[0];
  g_pipeDriverWriteSide = sv[1];
  
  /* Start processing events on the port */
  if(pthread_create( &g_driverThread, NULL,_threadManagedFunc, NULL) != 0)
    {
      /* keep msg to client app */
      _debug("pthread_create:void dgtnixInit(const char *port)\n");
      return -1;
    }
  double timeout = 1.5;
 
/* printf("**6**\n"); */

  while(g_boardUpdated != 1 &&  timeout > 0.)
    {
      _mySleep(0.05);
      timeout -= 0.05;
    }

/* printf("**7**\n"); */

  if(g_boardUpdated != 1)
    /* The answer of the device is incorrect, it's not a dgt board */
    {
      _debug("%s does not respond to the init query.\n" ,port);
      dgtnixClose();
      return -2;
    }
  _debug("Board initialised\n");

/* printf("**8**\n"); */

  return g_pipeEngineReadSide;
}

int dgtnixTestBoard(const char *board)
{
  _assertDriverInitialised("dgtnixTestBoard");
  int i;
  pthread_mutex_lock( &g_mutex );
  for(i=0; i<64; i++)
    { 
      if(g_boardOrientation == DGTNIX_BOARD_ORIENTATION_CLOCKLEFT)
	{
	  if(board[i] != _convertInternalPieceToExternal(g_board[i]))
	    {
	      pthread_mutex_unlock( &g_mutex );
	      return 0;
	    }
	}
      else
	{
	  if(board[i] != _convertInternalPieceToExternal(g_board[63-i]))
	    {
	      pthread_mutex_unlock( &g_mutex );
	      return 0;
	    }
	}
    }
  pthread_mutex_unlock( &g_mutex );
  return 1;
}

const char *dgtnixGetBoard()
{
  int i;
  _assertDriverInitialised("dgtnixGetBoard");
  pthread_mutex_lock( &g_mutex );
  if (g_boardUpdated)
    {
      for(i=0; i<64;i++)
        {
	  if(g_boardOrientation == DGTNIX_BOARD_ORIENTATION_CLOCKLEFT)
	    g_transmitedBoard[i] = _convertInternalPieceToExternal(g_board[i]);
	  else
	    g_transmitedBoard[i] = _convertInternalPieceToExternal(g_board[63-i]);
        }
      g_boardUpdated=0; 
    }
  pthread_mutex_unlock( &g_mutex );
  return g_transmitedBoard;
}

const char *dgtnixQueryString(unsigned int flag)
{
  static const char *undefinedFlagString = "dgtnixQueryString error, wrong argument";
  static const char *uninitializedString = "dgtnixQueryString error, must call dgtnixInit() before";
  
  if(flag == DGTNIX_DRIVER_VERSION)
    {
      return (const char *)_DGTNIX_DRIVER_VERSION;
    }
  
  _assertDriverInitialised("dgtnixQueryString with parameter != DGTNIX_DRIVER_VERSION");
  if(_queryVendorStrings()<0)
    return uninitializedString;
  switch (flag)
    {
    case DGTNIX_SERIAL_STRING:
      return g_serialBuffer;
    case DGTNIX_BUSADDRESS_STRING:
      return g_busadressBuffer;
    case DGTNIX_VERSION_STRING: 
      return g_versionBuffer;
    case DGTNIX_TRADEMARK_STRING:
      return g_trademarkBuffer;
    default:
      return undefinedFlagString;
    }
}


int dgtnixGetClockData(int *pwhite_time, int *pblack_time, int *pwhite_turn)
{
  _assertDriverInitialised("dgtnixGetClockData");
  if(g_wtime==-1)
    return 0;
  *pwhite_time=g_wtime;
  *pblack_time=g_btime;
  if(g_wturn==1)
    *pwhite_turn=1;
  else
    *pwhite_turn=0;
  return 1;
}

int main()
{
  int a=0;
  const char * b;  

  while (a!=4) {
    a = dgtnixInit("/dev/ttyUSB0");}

  b = dgtnixGetBoard();
  printf("%s\n",dgtnixToPrintableBoard(b));

  dgtnixClose();
}


/* FOR DEBUGGING ONLY
  while (a!=4) {
    a=dgtnixInit("/dev/serial/by-id/usb-FTDI_USB__-__Serial-if00-port0");}
    printf("   %d\n",a);} 

  printf("KK: initialized %d.\n",a); 
  while (a++ < 20) { 
    b = dgtnixGetBoard();
    printf("%s\n",dgtnixToPrintableBoard(b));
    sleep(1);
  } 
  dgtnixClose();
} */
