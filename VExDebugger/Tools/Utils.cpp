#include "Utils.h"

int Utils::_IsCallInstruction( uint8_t* PointInstruction )
{
	switch ( PointInstruction[ 0 ] ) {

	case 0xE8:
		return 5;

	case 0x9A:
		return 7;

	case 0xFF:
		switch ( PointInstruction[ 1 ] ) {

		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x16:
		case 0x17:
		case 0xD0:
		case 0xD1:
		case 0xD2:
		case 0xD3:
		case 0xD4:
		case 0xD5:
		case 0xD6:
		case 0xD7:
			return 2;

		case 0x14:
		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57:
			return 3;

		case 0x15:
		case 0x90:
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x95:
		case 0x96:
		case 0x97:
			return 6;

		case 0x94:
			return 7;
		}

	default:
		return 0;
	}
}