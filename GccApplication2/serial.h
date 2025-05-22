/*
 * serial.h
 *
 * Created: 11/12/2024 16:58:04
 *  Author: melri
 */ 


#ifndef SERIAL_H_
#define SERIAL_H_

void initUART();
void SerialRecebeChars(int sizeS, char* string);
void SerialEnviaChars(int sizeS, char* string);

#endif /* SERIAL_H_ */