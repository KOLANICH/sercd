/*
 * sercd UNIX support
 * Copyright 2008 Peter Åstrand <astrand@cendio.se> for Cendio AB
 * see file COPYING for license details
 */

#ifndef WIN32
#include "sercd.h"
#include "unix.h"

#include <termios.h>
#include <termio.h>
#include <syslog.h>

extern Boolean BreakSignaled;

/* Retrieves the port speed from PortFd */
unsigned long int
GetPortSpeed(PORTHANDLE PortFd)
{
    struct termios PortSettings;
    speed_t Speed;

    tcgetattr(PortFd, &PortSettings);
    Speed = cfgetospeed(&PortSettings);

    switch (Speed) {
    case B50:
	return (50UL);
    case B75:
	return (75UL);
    case B110:
	return (110UL);
    case B134:
	return (134UL);
    case B150:
	return (150UL);
    case B200:
	return (200UL);
    case B300:
	return (300UL);
    case B600:
	return (600UL);
    case B1200:
	return (1200UL);
    case B1800:
	return (1800UL);
    case B2400:
	return (2400UL);
    case B4800:
	return (4800UL);
    case B9600:
	return (9600UL);
    case B19200:
	return (19200UL);
    case B38400:
	return (38400UL);
    case B57600:
	return (57600UL);
    case B115200:
	return (115200UL);
    case B230400:
	return (230400UL);
    case B460800:
	return (460800UL);
    default:
	return (0UL);
    }
}

/* Retrieves the data size from PortFd */
unsigned char
GetPortDataSize(PORTHANDLE PortFd)
{
    struct termios PortSettings;
    tcflag_t DataSize;

    tcgetattr(PortFd, &PortSettings);
    DataSize = PortSettings.c_cflag & CSIZE;

    switch (DataSize) {
    case CS5:
	return ((unsigned char) 5);
    case CS6:
	return ((unsigned char) 6);
    case CS7:
	return ((unsigned char) 7);
    case CS8:
	return ((unsigned char) 8);
    default:
	return ((unsigned char) 0);
    }
}

/* Retrieves the parity settings from PortFd */
unsigned char
GetPortParity(PORTHANDLE PortFd)
{
    struct termios PortSettings;

    tcgetattr(PortFd, &PortSettings);

    if ((PortSettings.c_cflag & PARENB) == 0)
	return ((unsigned char) 1);

    if ((PortSettings.c_cflag & PARENB) != 0 && (PortSettings.c_cflag & PARODD) != 0)
	return ((unsigned char) 2);

    return ((unsigned char) 3);
}

/* Retrieves the stop bits size from PortFd */
unsigned char
GetPortStopSize(PORTHANDLE PortFd)
{
    struct termios PortSettings;

    tcgetattr(PortFd, &PortSettings);

    if ((PortSettings.c_cflag & CSTOPB) == 0)
	return ((unsigned char) 1);
    else
	return ((unsigned char) 2);
}

/* Retrieves the flow control status, including DTR and RTS status,
from PortFd */
unsigned char
GetPortFlowControl(PORTHANDLE PortFd, unsigned char Which)
{
    struct termios PortSettings;
    int MLines;

    /* Gets the basic informations from the port */
    tcgetattr(PortFd, &PortSettings);
    ioctl(PortFd, TIOCMGET, &MLines);

    /* Check wich kind of information is requested */
    switch (Which) {
	/* Com Port Flow Control Setting (outbound/both) */
    case 0:
	if (PortSettings.c_iflag & IXON)
	    return ((unsigned char) 2);
	if (PortSettings.c_cflag & CRTSCTS)
	    return ((unsigned char) 3);
	return ((unsigned char) 1);
	break;

	/* BREAK State  */
    case 4:
	if (BreakSignaled == True)
	    return ((unsigned char) 5);
	else
	    return ((unsigned char) 6);
	break;

	/* DTR Signal State */
    case 7:
	if (MLines & TIOCM_DTR)
	    return ((unsigned char) 8);
	else
	    return ((unsigned char) 9);
	break;

	/* RTS Signal State */
    case 10:
	if (MLines & TIOCM_RTS)
	    return ((unsigned char) 11);
	else
	    return ((unsigned char) 12);
	break;

	/* Com Port Flow Control Setting (inbound) */
    case 13:
	if (PortSettings.c_iflag & IXOFF)
	    return ((unsigned char) 15);
	if (PortSettings.c_cflag & CRTSCTS)
	    return ((unsigned char) 16);
	return ((unsigned char) 14);
	break;

    default:
	if (PortSettings.c_iflag & IXON)
	    return ((unsigned char) 2);
	if (PortSettings.c_cflag & CRTSCTS)
	    return ((unsigned char) 3);
	return ((unsigned char) 1);
	break;
    }
}

/* Return the status of the modem control lines (DCD, CTS, DSR, RNG) */
unsigned char
GetModemState(PORTHANDLE PortFd, unsigned char PMState)
{
    int MLines;
    unsigned char MState = (unsigned char) 0;

    ioctl(PortFd, TIOCMGET, &MLines);

    if ((MLines & TIOCM_CAR) != 0)
	MState += (unsigned char) 128;
    if ((MLines & TIOCM_RNG) != 0)
	MState += (unsigned char) 64;
    if ((MLines & TIOCM_DSR) != 0)
	MState += (unsigned char) 32;
    if ((MLines & TIOCM_CTS) != 0)
	MState += (unsigned char) 16;
    if ((MState & 128) != (PMState & 128))
	MState += (unsigned char) 8;
    if ((MState & 64) != (PMState & 64))
	MState += (unsigned char) 4;
    if ((MState & 32) != (PMState & 32))
	MState += (unsigned char) 2;
    if ((MState & 16) != (PMState & 16))
	MState += (unsigned char) 1;

    return (MState);
}

/* Set the serial port data size */
void
SetPortDataSize(PORTHANDLE PortFd, unsigned char DataSize)
{
    struct termios PortSettings;
    tcflag_t PDataSize;

    switch (DataSize) {
    case 5:
	PDataSize = CS5;
	break;
    case 6:
	PDataSize = CS6;
	break;
    case 7:
	PDataSize = CS7;
	break;
    case 8:
	PDataSize = CS8;
	break;
    default:
	PDataSize = CS8;
	break;
    }

    tcgetattr(PortFd, &PortSettings);
    PortSettings.c_cflag &= ~CSIZE;
    PortSettings.c_cflag |= PDataSize & CSIZE;
    tcsetattr(PortFd, TCSADRAIN, &PortSettings);
}

/* Set the serial port parity */
void
SetPortParity(PORTHANDLE PortFd, unsigned char Parity)
{
    struct termios PortSettings;

    tcgetattr(PortFd, &PortSettings);

    switch (Parity) {
    case 1:
	PortSettings.c_cflag = PortSettings.c_cflag & ~PARENB;
	break;
    case 2:
	PortSettings.c_cflag = PortSettings.c_cflag | PARENB | PARODD;
	break;
    case 3:
	PortSettings.c_cflag = (PortSettings.c_cflag | PARENB) & ~PARODD;
	break;
	/* There's no support for MARK and SPACE parity so sets no parity */
    default:
	LogMsg(LOG_WARNING, "Requested unsupported parity, set to no parity.");
	PortSettings.c_cflag = PortSettings.c_cflag & ~PARENB;
	break;
    }

    tcsetattr(PortFd, TCSADRAIN, &PortSettings);
}

/* Set the serial port stop bits size */
void
SetPortStopSize(PORTHANDLE PortFd, unsigned char StopSize)
{
    struct termios PortSettings;

    tcgetattr(PortFd, &PortSettings);

    switch (StopSize) {
    case 1:
	PortSettings.c_cflag = PortSettings.c_cflag & ~CSTOPB;
	break;
    case 2:
	PortSettings.c_cflag = PortSettings.c_cflag | CSTOPB;
	break;
    case 3:
	PortSettings.c_cflag = PortSettings.c_cflag & ~CSTOPB;
	LogMsg(LOG_WARNING, "Requested unsupported 1.5 bits stop size, set to 1 bit stop size.");
	break;
    default:
	PortSettings.c_cflag = PortSettings.c_cflag & ~CSTOPB;
	break;
    }

    tcsetattr(PortFd, TCSADRAIN, &PortSettings);
}

/* Set the port flow control and DTR and RTS status */
void
SetPortFlowControl(PORTHANDLE PortFd, unsigned char How)
{
    struct termios PortSettings;
    int MLines;

    /* Gets the base status from the port */
    tcgetattr(PortFd, &PortSettings);
    ioctl(PortFd, TIOCMGET, &MLines);

    /* Check which settings to change */
    switch (How) {
	/* No Flow Control (outbound/both) */
    case 1:
	PortSettings.c_iflag = PortSettings.c_iflag & ~IXON;
	PortSettings.c_iflag = PortSettings.c_iflag & ~IXOFF;
	PortSettings.c_cflag = PortSettings.c_cflag & ~CRTSCTS;
	break;
	/* XON/XOFF Flow Control (outbound/both) */
    case 2:
	PortSettings.c_iflag = PortSettings.c_iflag | IXON;
	PortSettings.c_iflag = PortSettings.c_iflag | IXOFF;
	PortSettings.c_cflag = PortSettings.c_cflag & ~CRTSCTS;
	break;
	/* HARDWARE Flow Control (outbound/both) */
    case 3:
	PortSettings.c_iflag = PortSettings.c_iflag & ~IXON;
	PortSettings.c_iflag = PortSettings.c_iflag & ~IXOFF;
	PortSettings.c_cflag = PortSettings.c_cflag | CRTSCTS;
	break;
	/* BREAK State ON */
    case 5:
	tcsendbreak(PortFd, 1);
	BreakSignaled = True;
	break;
	/* BREAK State OFF */
    case 6:
	/* Should not send another break */
	/* tcsendbreak(PortFd,0); */
	BreakSignaled = False;
	break;
	/* DTR Signal State ON */
    case 8:
	MLines = MLines | TIOCM_DTR;
	break;
	/* DTR Signal State OFF */
    case 9:
	MLines = MLines & ~TIOCM_DTR;
	break;
	/* RTS Signal State ON */
    case 11:
	MLines = MLines | TIOCM_RTS;
	break;
	/* RTS Signal State OFF */
    case 12:
	MLines = MLines & ~TIOCM_RTS;
	break;

	/* INBOUND FLOW CONTROL is ignored */
	/* No Flow Control (inbound) */
    case 14:
	/* XON/XOFF Flow Control (inbound) */
    case 15:
	/* HARDWARE Flow Control (inbound) */
    case 16:
	LogMsg(LOG_WARNING, "Inbound flow control ignored.");
	break;
    default:
	LogMsg(LOG_WARNING, "Requested unsupported flow control.");
	break;
    }

    tcsetattr(PortFd, TCSADRAIN, &PortSettings);
    ioctl(PortFd, TIOCMSET, &MLines);
}

/* Set the serial port speed */
void
SetPortSpeed(PORTHANDLE PortFd, unsigned long BaudRate)
{
    struct termios PortSettings;
    speed_t Speed;

    switch (BaudRate) {
    case 50UL:
	Speed = B50;
	break;
    case 75UL:
	Speed = B75;
	break;
    case 110UL:
	Speed = B110;
	break;
    case 134UL:
	Speed = B134;
	break;
    case 150UL:
	Speed = B150;
	break;
    case 200UL:
	Speed = B200;
	break;
    case 300UL:
	Speed = B300;
	break;
    case 600UL:
	Speed = B600;
	break;
    case 1200UL:
	Speed = B1200;
	break;
    case 1800UL:
	Speed = B1800;
	break;
    case 2400UL:
	Speed = B2400;
	break;
    case 4800UL:
	Speed = B4800;
	break;
    case 9600UL:
	Speed = B9600;
	break;
    case 19200UL:
	Speed = B19200;
	break;
    case 38400UL:
	Speed = B38400;
	break;
    case 57600UL:
	Speed = B57600;
	break;
    case 115200UL:
	Speed = B115200;
	break;
    case 230400UL:
	Speed = B230400;
	break;
    case 460800UL:
	Speed = B460800;
	break;
    default:
	LogMsg(LOG_WARNING, "Unknwon baud rate requested, setting to 9600.");
	Speed = B9600;
	break;
    }

    tcgetattr(PortFd, &PortSettings);
    cfsetospeed(&PortSettings, Speed);
    cfsetispeed(&PortSettings, Speed);
    tcsetattr(PortFd, TCSADRAIN, &PortSettings);
}


#endif /* WIN32 */
