#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "..\fscc_serialp.h"

/*******************************************************************************

Sample program to set the flow control.

*******************************************************************************/

void set_flow_ctrl(HANDLE fscc, unsigned value);
unsigned get_flow_ctrl(HANDLE fscc);

int main(int argc, char *argv[])
{
	DCB com_settings = {0};
	char string_buffer[80];
	HANDLE fscc;
	unsigned old_flow_value, new_flow_value, potential_new_value;
	
	if (argc < 3) {
		fprintf(stderr, "%s COM? [0-3] \n", argv[0]);
		return -1;
	}

	sprintf(string_buffer, "\\\\.\\%s", argv[1]);

	potential_new_value = 0;
	potential_new_value = (unsigned)atoi(argv[2]);

	if (potential_new_value > 3) {
		fprintf(stderr, "Flow control value out of range [0-3].\n");
		return -1;
	}

	fscc = CreateFile(string_buffer, GENERIC_READ | GENERIC_WRITE, 0, NULL, 
							         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (fscc == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Error opening FSCC.\n");
		return -1;
	}

	old_flow_value = get_flow_ctrl(fscc);
	
	if (old_flow_value == potential_new_value) {
		fprintf(stdout, "Flow control already set to %i.\n", old_flow_value);
		return 0;
	}

	set_flow_ctrl(fscc, potential_new_value);

	new_flow_value = get_flow_ctrl(fscc);

	if (new_flow_value != potential_new_value) {
		fprintf(stderr, "Error setting flow control to %i.\n",potential_new_value);
		return -1;
	}

	fprintf(stdout, "Flow control changed from %i -> %i.\n", old_flow_value, new_flow_value);

	return 0;
}

void set_flow_ctrl(HANDLE fscc, unsigned value)
{
	DWORD ret = 0, retsz = 0;

	if (!DeviceIoControl(fscc, IOCTL_SERIALP_SET_HARDWARE_RTSCTS, &value, 
		sizeof(value), &ret, sizeof(ret), &retsz, NULL)) {
		fprintf(stderr, "Error setting flow control. System Error Code %i\n", GetLastError());
	}
}

unsigned get_flow_ctrl(HANDLE fscc)
{	
	DWORD ret = 0, retsz = 0;

	if (!DeviceIoControl(fscc, IOCTL_SERIALP_GET_HARDWARE_RTSCTS, NULL, 
		0, &ret, sizeof(ret), &retsz, NULL)) {
		fprintf(stderr, "Error getting flow control.\n");
		return 0;
	}

	return (unsigned)ret;
}