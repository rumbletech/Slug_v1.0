/*
 * file_sys.h
 *
 *  Created on: Apr 20, 2025
 *      Author: rumbl
 */

#ifndef INC_FILE_SYS_H_
#define INC_FILE_SYS_H_


#define FSYS_SUCCESS       0x0000
#define FSYS_FAIL          0x0001
#define FSYS_INVALID_FILE  0x0002

typedef uint16_t _fsys_err;


extern _fsys_err Fsys_Init( void );
extern _fsys_err Fsys_GetConfig ( char** sptr , uint32_t* lptr );
extern _fsys_err Fsys_LoadConfig ( void );


#endif /* INC_FILE_SYS_H_ */
