/*
 * uMTP Responder
 * Copyright (c) 2018 Viveris Technologies
 *
 * uMTP Responder is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * uMTP Responder is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 3 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with uMTP Responder; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file   mtp_cfg.c
 * @brief  Configuration file parser.
 * @author Jean-François DEL NERO <Jean-Francois.DELNERO@viveris.fr>
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <errno.h>

#include <sys/stat.h>

#include <sys/types.h>
#include <dirent.h>

#include "logs_out.h"

#include "fs_handles_db.h"

#include "mtp.h"
#include "mtp_cfg.h"

#include "usbstring.h"

#include "default_cfg.h"

typedef int (* KW_FUNC)(mtp_ctx * context, char * line, int cmd);

enum
{
	STORAGE_CMD = 0,
	USBVENDORID_CMD,
	USBPRODUCTID_CMD,
	USBCLASS_CMD,
	USBSUBCLASS_CMD,
	USBPROTOCOL_CMD,
	USBDEVVERSION_CMD,
	USBMAXPACKETSIZE_CMD,

	USB_DEV_PATH_CMD,
	USB_EPIN_PATH_CMD,
	USB_EPOUT_PATH_CMD,
	USB_EPINT_PATH_CMD,

	MANUFACTURER_STRING_CMD,
	PRODUCT_STRING_CMD,
	SERIAL_STRING_CMD,
	INTERFACE_STRING_CMD
};

typedef struct kw_list_
{
	char * keyword;
	KW_FUNC func;
	int cmd;
}kw_list;

int is_end_line(char c)
{
	if( c == 0 || c == '#' || c == '\r' || c == '\n' )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int is_space(char c)
{
	if( c == ' ' || c == '\t' )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
int get_next_word(char * line, int offset)
{
	while( !is_end_line(line[offset]) && ( line[offset] == ' ' ) )
	{
		offset++;
	}

	return offset;
}

int copy_param(char * dest, char * line, int offs)
{
	int i,insidequote;

	i = 0;
	insidequote = 0;
	while( !is_end_line(line[offs]) && ( insidequote || !is_space(line[offs]) ) )
	{
		if(line[offs] != '"')
		{
			if(dest)
				dest[i] = line[offs];

			i++;
		}
		else
		{
			if(insidequote)
				insidequote = 0;
			else
				insidequote = 1;
		}

		offs++;
	}

	if(dest)
		dest[i] = 0;

	return offs;
}

int get_param_offset(char * line, int param)
{
	int param_cnt, offs;

	offs = 0;
	offs = get_next_word(line, offs);

	param_cnt = 0;
	do
	{
		offs = copy_param(NULL, line, offs);

		offs = get_next_word( line, offs );

		if(line[offs] == 0 || line[offs] == '#')
			return -1;

		param_cnt++;
	}while( param_cnt < param );

	return offs;
}

int get_param(char * line, int param_offset,char * param)
{
	int offs;

	offs = get_param_offset(line, param_offset);

	if(offs>=0)
	{
		offs = copy_param(param, line, offs);

		return 1;
	}

	return -1;
}

int extract_cmd(char * line, char * command)
{
	int offs,i;

	i = 0;
	offs = 0;

	offs = get_next_word(line, offs);

	if( !is_end_line(line[offs]) )
	{
		while( !is_end_line(line[offs]) && !is_space(line[offs]) )
		{
			command[i] = line[offs];
			offs++;
			i++;
		}

		command[i] = 0;

		return i;
	}

	return 0;
}

int get_storage_params(mtp_ctx * context, char * line,int cmd)
{
	int i, j;
	char storagename[MAX_CFG_STRING_SIZE];
	char storagepath[MAX_CFG_STRING_SIZE];

	i = get_param(line, 2,storagename);
	j = get_param(line, 1,storagepath);

	if( i >= 0 && j >= 0 )
	{
		PRINT_MSG("Add storage %s - Root Path: %s", storagename, storagepath);

		mtp_add_storage(context, storagepath, storagename);
	}

	return 0;
}

int get_hex_param(mtp_ctx * context, char * line,int cmd)
{
	int i;
	char tmp_txt[MAX_CFG_STRING_SIZE];
	unsigned long param_value;

	i = get_param(line, 1,tmp_txt);

	if( i >= 0 )
	{
		param_value = strtol(tmp_txt,0,16);
		switch(cmd)
		{
			case USBVENDORID_CMD:
				context->usb_cfg.usb_vendor_id = param_value;
			break;

			case USBPRODUCTID_CMD:
				context->usb_cfg.usb_product_id = param_value;
			break;

			case USBCLASS_CMD:
				context->usb_cfg.usb_class = param_value;
			break;

			case USBSUBCLASS_CMD:
				context->usb_cfg.usb_subclass = param_value;
			break;

			case USBPROTOCOL_CMD:
				context->usb_cfg.usb_protocol = param_value;
			break;

			case USBDEVVERSION_CMD:
				context->usb_cfg.usb_dev_version = param_value;
			break;

			case USBMAXPACKETSIZE_CMD:
				context->usb_cfg.usb_max_packet_size = param_value;
			break;
		}
	}

	return 0;
}

int get_str_param(mtp_ctx * context, char * line,int cmd)
{
	int i;
	char tmp_txt[MAX_CFG_STRING_SIZE];

	i = get_param(line, 1,tmp_txt);

	if( i >= 0 )
	{
		switch(cmd)
		{
			case USB_DEV_PATH_CMD:
				strncpy(context->usb_cfg.usb_device_path,tmp_txt,MAX_CFG_STRING_SIZE);
			break;

			case USB_EPIN_PATH_CMD:
				strncpy(context->usb_cfg.usb_endpoint_in,tmp_txt,MAX_CFG_STRING_SIZE);
			break;

			case USB_EPOUT_PATH_CMD:
				strncpy(context->usb_cfg.usb_endpoint_out,tmp_txt,MAX_CFG_STRING_SIZE);
			break;

			case USB_EPINT_PATH_CMD:
				strncpy(context->usb_cfg.usb_endpoint_intin,tmp_txt,MAX_CFG_STRING_SIZE);
			break;

			case MANUFACTURER_STRING_CMD:
				strncpy(context->usb_cfg.usb_string_manufacturer,tmp_txt,MAX_CFG_STRING_SIZE);
			break;

			case PRODUCT_STRING_CMD:
				strncpy(context->usb_cfg.usb_string_product,tmp_txt,MAX_CFG_STRING_SIZE);
			break;

			case SERIAL_STRING_CMD:
				strncpy(context->usb_cfg.usb_string_serial,tmp_txt,MAX_CFG_STRING_SIZE);
			break;

			case INTERFACE_STRING_CMD:
				strncpy(context->usb_cfg.usb_string_interface,tmp_txt,MAX_CFG_STRING_SIZE);
			break;
		}
	}

	return 0;
}

kw_list kwlist[] =
{
	{"storage",             get_storage_params, STORAGE_CMD},
	{"usb_vendor_id",       get_hex_param,      USBVENDORID_CMD},
	{"usb_product_id",      get_hex_param,      USBPRODUCTID_CMD},
	{"usb_class",           get_hex_param,      USBCLASS_CMD},
	{"usb_subclass",        get_hex_param,      USBSUBCLASS_CMD},
	{"usb_protocol",        get_hex_param,      USBPROTOCOL_CMD},
	{"usb_dev_version",     get_hex_param,      USBDEVVERSION_CMD},
	{"usb_max_packet_size", get_hex_param,      USBMAXPACKETSIZE_CMD},

	{"usb_dev_path",        get_str_param,      USB_DEV_PATH_CMD},
	{"usb_epin_path",       get_str_param,      USB_EPIN_PATH_CMD},
	{"usb_epout_path",      get_str_param,      USB_EPOUT_PATH_CMD},
	{"usb_epint_path",      get_str_param,      USB_EPINT_PATH_CMD},
	{"manufacturer",        get_str_param,      MANUFACTURER_STRING_CMD},
	{"product",             get_str_param,      PRODUCT_STRING_CMD},
	{"serial",              get_str_param,      SERIAL_STRING_CMD},
	{"interface",           get_str_param,      INTERFACE_STRING_CMD},
	{ 0, 0, 0 }
};

int exec_cmd(mtp_ctx * context, char * command,char * line)
{
	int i;

	i = 0;
	while(kwlist[i].func)
	{
		if( !strcmp(kwlist[i].keyword,command) )
		{
			kwlist[i].func(context, line, kwlist[i].cmd);
			return 1;
		}

		i++;
	}


	return 0;
}

int execute_line(mtp_ctx * context,char * line)
{
	char command[MAX_CFG_STRING_SIZE];

	command[0] = 0;

	if( extract_cmd(line, command) )
	{
		if(strlen(command))
		{
			if( !exec_cmd(context, command,line))
			{
				PRINT_ERROR("Line syntax error : %s",line);

				return 0;
			}
		}

		return 1;
	}

	return 0;
}

int mtp_load_config_file(mtp_ctx * context)
{
	int err = 0;
	FILE * f;
	char line[MAX_CFG_STRING_SIZE];

	// Set default config
	strncpy(context->usb_cfg.usb_device_path,         USB_DEV,                MAX_CFG_STRING_SIZE);
	strncpy(context->usb_cfg.usb_endpoint_in,         USB_EPIN,               MAX_CFG_STRING_SIZE);
	strncpy(context->usb_cfg.usb_endpoint_out,        USB_EPOUT,              MAX_CFG_STRING_SIZE);
	strncpy(context->usb_cfg.usb_endpoint_intin,      USB_EPINTIN,            MAX_CFG_STRING_SIZE);
	strncpy(context->usb_cfg.usb_string_manufacturer, MANUFACTURER,           MAX_CFG_STRING_SIZE);
	strncpy(context->usb_cfg.usb_string_product,      PRODUCT,                MAX_CFG_STRING_SIZE);
	strncpy(context->usb_cfg.usb_string_serial,       SERIALNUMBER,           MAX_CFG_STRING_SIZE);
	strncpy(context->usb_cfg.usb_string_interface,    "MTP",                  MAX_CFG_STRING_SIZE);

	context->usb_cfg.usb_vendor_id       = USB_DEV_VENDOR_ID;
	context->usb_cfg.usb_product_id      = USB_DEV_PRODUCT_ID;
	context->usb_cfg.usb_class           = USB_DEV_CLASS;
	context->usb_cfg.usb_subclass        = USB_DEV_SUBCLASS;
	context->usb_cfg.usb_protocol        = USB_DEV_PROTOCOL;
	context->usb_cfg.usb_dev_version     = USB_DEV_VERSION;
	context->usb_cfg.usb_max_packet_size = MAX_PACKET_SIZE;

	f = fopen(UMTPR_CONF_FILE,"r");
	if(f)
	{
		do
		{
			fgets(line,sizeof(line),f);
			if(feof(f))
				break;
			execute_line(context, line);
		}while(1);

		fclose(f);
	}
	else
	{
		PRINT_ERROR("Can't open %s ! Using default settings...", UMTPR_CONF_FILE);
	}

	PRINT_MSG("USB Device path : %s",context->usb_cfg.usb_device_path);
	PRINT_MSG("USB In End point path : %s",context->usb_cfg.usb_endpoint_in);
	PRINT_MSG("USB Out End point path : %s",context->usb_cfg.usb_endpoint_out);
	PRINT_MSG("USB Event End point path : %s",context->usb_cfg.usb_endpoint_intin);
	PRINT_MSG("USB Max packet size : 0x%X bytes",context->usb_cfg.usb_max_packet_size);

	PRINT_MSG("Manufacturer string : %s",context->usb_cfg.usb_string_manufacturer);
	PRINT_MSG("Product string : %s",context->usb_cfg.usb_string_product);
	PRINT_MSG("Serial string : %s",context->usb_cfg.usb_string_serial);
	PRINT_MSG("Interface string : %s",context->usb_cfg.usb_string_interface);

	PRINT_MSG("USB Vendor ID : 0x%.4X",context->usb_cfg.usb_vendor_id);
	PRINT_MSG("USB Product ID : 0x%.4X",context->usb_cfg.usb_product_id);
	PRINT_MSG("USB class ID : 0x%.2X",context->usb_cfg.usb_class);
	PRINT_MSG("USB subclass ID : 0x%.2X",context->usb_cfg.usb_subclass);
	PRINT_MSG("USB Protocol ID : 0x%.2X",context->usb_cfg.usb_protocol);
	PRINT_MSG("USB Device version : 0x%.4X",context->usb_cfg.usb_dev_version);

	return err;
}
