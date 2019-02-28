/*
 * twi_master.h
 *  Created on: 18.06.2017
 */

#ifndef TWI_MASTER_H
#define TWI_MASTER_H


#define TWI_READ_BIT                 (0x01)        //!< If this bit is set in the address field, transfer direction is from slave to master.

#define TWI_ISSUE_STOP               ((bool)true)  //!< Parameter for @ref twi_master_transfer_alpar
#define TWI_DONT_ISSUE_STOP          ((bool)false) //!< Parameter for @ref twi_master_transfer_alpar


bool twi_master_init(void);


bool twi_master_transfer(uint8_t address, uint8_t *data, uint8_t data_length, bool issue_stop_condition);


bool twi_master_transfer_alpar(uint8_t address, uint8_t *data, uint16_t data_length, bool issue_stop_condition);

#endif
