/** \addtogroup cc2538-examples
 * @{
 *
 * \file
 *  /dev/sht21
 *  An example of a simple I2C use for the cc2538 and SHT21 sensor
 * \author
 *  Mehdi Migault
 */
#include <contiki.h>
#include "dev/i2c.h"

/*---------------------------------------------------------------------------*/
char*
print_float(float num, uint8_t preci)
{
	int integer=(int)num, decimal=0;
	char buf[20];
	preci = preci > 10 ? 10 : preci;
	num -= integer;
	while((num != 0) && (preci-- > 0)) {
		decimal *= 10;
		num *= 10;
		decimal += (int)num;
		num -= (int)num;
	}
	sprintf(buf, "%d.%d", integer, decimal);
	printf("%s", buf);
        return buf;
}

uint8_t
check_crc_SHT21(uint8_t data[], uint8_t dataSize, uint8_t chksm)
{
	uint8_t crc=0, i, j;
	for(i=0; i<dataSize; ++i) {
		crc ^= data[i];
		for(j=8; j>0; --j) {
			if(crc & 0x80) {
				crc = (crc<<1) ^ SHT21_CRC_POLYNOMIAL;
			} else {
				crc = (crc<<1);
			}
		}
	}
	if(crc != chksm) {
		return -1;
	} else {
		return I2C_MASTER_ERR_NONE;
	}
}
/*---------------------------------------------------------------------------*/
uint8_t
read_temp_SHT21(uint16_t * data)
{
	uint8_t dataByte[2];

	/* Ask sensor for temperature measurement */
	i2c_master_set_slave_address(SHT21_SLAVE_ADDRESS, I2C_SEND);
	i2c_master_data_put(SHT21_TEMP_REGISTER);
	i2c_master_command(I2C_MASTER_CMD_BURST_SEND_START);
	while(i2c_master_busy()){
	}
	if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
		/*
		 * Temp max measurement time for SHT21 is 85ms
		 * Made in 10 times because the function can't handle a 85000 value
		 */
		for(temp=0; temp<10; temp++) {
			clock_delay_usec(8500);	//Maybe cause of problem with watchdog
		}

		/* Get the 3 bytes of data*/
		/* Data MSB */
		i2c_master_set_slave_address(SHT21_SLAVE_ADDRESS, I2C_RECEIVE);
		i2c_master_command(I2C_MASTER_CMD_BURST_RECEIVE_START);
		while(i2c_master_busy()) {
		}
		if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
			*data = i2c_master_data_get() << 8;

			/* Data LSB */
			i2c_master_command(I2C_MASTER_CMD_BURST_RECEIVE_CONT);
			while(i2c_master_busy()) {
			}
			if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
				*data |= i2c_master_data_get() &~0x0003;

				/* Checksum */
				i2c_master_command(I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
				while(i2c_master_busy()) {
				}
				if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
					dataByte[0] = (*data)>>8;
					dataByte[1] = (*data)&0xFF;
					if(check_crc_SHT21(dataByte, 2, i2c_master_data_get()) == I2C_MASTER_ERR_NONE){
						return I2C_MASTER_ERR_NONE;
					}
				}
			}
		}
	}
	return i2c_master_error();
}

/*---------------------------------------------------------------------------*/
uint8_t
read_hum_SHT21(uint16_t * data)
{
	uint8_t dataByte[2];

	/* Ask sensor for humidity measurement */
	i2c_master_set_slave_address(SHT21_SLAVE_ADDRESS, I2C_SEND);
	i2c_master_data_put(SHT21_HUM_REGISTER);
	i2c_master_command(I2C_MASTER_CMD_BURST_SEND_START);
	while(i2c_master_busy()){
	}
	if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
		/*
		 * Hum max measurement time for SHT21 is 85ms
		 * Made in 10 times because the function can't handle a 85000 value
		 */
		for(hum=0; hum<10; hum++) {
			clock_delay_usec(8500);	//Maybe cause of problem with watchdog
		}

		/* Get the 3 bytes of data*/
		/* Data MSB */
		i2c_master_set_slave_address(SHT21_SLAVE_ADDRESS, I2C_RECEIVE);
		i2c_master_command(I2C_MASTER_CMD_BURST_RECEIVE_START);
		while(i2c_master_busy()) {
		}
		if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
			*data = i2c_master_data_get() << 8;

			/* Data LSB */
			i2c_master_command(I2C_MASTER_CMD_BURST_RECEIVE_CONT);
			while(i2c_master_busy()) {
			}
			if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
				*data |= i2c_master_data_get() &~0x0003;

				/* Checksum */
				i2c_master_command(I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
				while(i2c_master_busy()) {
				}
				if(i2c_master_error() == I2C_MASTER_ERR_NONE) {
					dataByte[0] = (*data)>>8;
					dataByte[1] = (*data)&0xFF;
					if(check_crc_SHT21(dataByte, 2, i2c_master_data_get()) == I2C_MASTER_ERR_NONE){
						return I2C_MASTER_ERR_NONE;
					}
				}
			}
		}
	}
	return i2c_master_error();
}

void 
read_temperature(char* ctemp) {
    uint16_t * temp_read;
    float temp;
    ctemp = malloc(10 * sizeof(char));
    /* We must init I2C each time, because the module lose his state when enter PM2 */
    i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN, I2C_SCL_NORMAL_BUS_SPEED);
    if(read_temp_SHT21(&temp_read) == I2C_MASTER_ERR_NONE) {
        temp = (((float)temp_read)/65536)*175.72-46.85;
        ctemp = print_float(temp, SHT21_DECIMAL_PRECISION);
    }
    else {
        printf("\nError reading temperature sensor");
    }
}

void 
read_humidity(char* chum) {
    uint16_t * hum_read;
    float hum;
    char* chum;
    chum = malloc(10 * sizeof(char));
    /* We must init I2C each time, because the module lose his state when enter PM2 */
    i2c_init(I2C_SDA_PORT, I2C_SDA_PIN, I2C_SCL_PORT, I2C_SCL_PIN, I2C_SCL_NORMAL_BUS_SPEED);
    if(read_hum_SHT21(&hum_read) == I2C_MASTER_ERR_NONE) {
        hum = (((float)hum_read)/65536)*125.0-6.0;
        chum = print_float(hum, SHT21_DECIMAL_PRECISION);
    }
    else {
        printf("\nError reading humidity sensor");
    }
}
