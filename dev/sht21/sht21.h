#ifndef __SHT21_H__
#define __SHT21_H__

/** \name I2C pins
 * @{
 */
#define I2C_SDA_PORT	GPIO_B_NUM
#define I2C_SDA_PIN		 3
#define I2C_SCL_PORT	GPIO_B_NUM
#define I2C_SCL_PIN		 2
/** @} */

/** \name Sensor's specific
 * @{
 */
#define SHT21_SLAVE_ADDRESS		0x40
#define SHT21_TEMP_REGISTER		0xF3
#define SHT21_HUM_REGISTER		0xF5
#define SHT21_CRC_POLYNOMIAL		0x131
#define SHT21_DECIMAL_PRECISION		2
/** @} */

char* print_float(float num, uint8_t preci);
uint8_t check_crc_SHT21(uint8_t data[], uint8_t dataSize, uint8_t chksm);
uint8_t read_temp_SHT21(uint16_t * data);
uint8_t read_hum_SHT21(uint16_t * data);
void read_temperature(char * ctemp);
void read_humidity(char * chum);

#endif /*__SHT21_H__*/