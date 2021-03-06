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
 * @file   logs_out.h
 * @brief  Log output functions
 * @author Jean-François DEL NERO <Jean-Francois.DELNERO@viveris.fr>
 */

#ifndef _INC_DEBUG_OUT_H_
#define _INC_DEBUG_OUT_H_

#ifdef USE_SYSLOG

#include <syslog.h>

#else

#ifdef DEBUG
#include <stdio.h>
#endif

#endif


#ifdef USE_SYSLOG // Syslog usage

#define PRINT_MSG(fmt, args...) syslog(LOG_NOTICE, "[Info] " fmt "\n", \
                                         ## args)
#define PRINT_ERROR(fmt, args...)   syslog(LOG_ERR, "[Error] " fmt "\n", \
                                         ## args)
#define PRINT_WARN(fmt, args...)    syslog(LOG_WARNING, "[Warning] " fmt "\n", \
                                         ## args)
#ifdef DEBUG

#define PRINT_DEBUG(fmt, args...)   syslog(LOG_DEBUG, "[Debug] " fmt "\n",  \
                                         ## args)
#else

#define PRINT_DEBUG(fmt, args...)

#endif

#else // Stdout usage

#define PRINT_MSG(fmt, args...)   {                               \
                                    fprintf(stdout,               \
                                            "[Info] " fmt "\r\n", \
                                            ## args);             \
                                    fflush(stdout);               \
                                  }
#define PRINT_ERROR(fmt, args...) {                                 \
                                    fprintf(stderr,                 \
                                            "[Error] " fmt "\r\n",  \
                                            ## args);               \
                                    fflush(stderr);                 \
                                  }
#define PRINT_WARN(fmt, args...)  {                                 \
                                    fprintf(stdout,                 \
                                            "[Warning] " fmt "\r\n",\
                                            ## args);               \
                                    fflush(stdout);                 \
                                  }

#ifdef DEBUG
#define PRINT_DEBUG(fmt, args...) {                                 \
                                    fprintf(stdout,                 \
                                            "[Debug] " fmt "\r\n",  \
                                            ## args);               \
                                    fflush(stdout);                 \
                                  }
#else

#define PRINT_DEBUG(fmt, args...)

#endif

#endif

#ifdef DEBUG

#define PRINT_DEBUG_BUF(x, y) printbuf( x, y );
void printbuf(void * buf,int size);

#else

#define PRINT_DEBUG_BUF(x, y)

#endif

#endif

