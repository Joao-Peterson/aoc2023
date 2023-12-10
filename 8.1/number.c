#include "number.h"

void getDigits(uint64_t value, uint16_t *digitsArray, uint16_t *size){
	for(int i = 0; i < 20; i++){ // uint64max has 20 digits: 18446744073709551615
		div_t d = div(value, 10);
		digitsArray[0] = d.rem;
		value = d.quot;
	}

	size_t count = 0;
	for(int i = 20 - 1; i >= 0; i--){
		if(digitsArray[i] != '\0') break;
		count++;
	}

	*size = 20 - count;
}

number_t number_from(uint64_t value, uint64_t mantissa, bool sign){
	number_t num = {0};

	if(value > 0)    getDigits(value, num.digits, &num.digitsSize);
	if(mantissa > 0) getDigits(mantissa, num.mantissa, &num.mantissaSize);
	num.sign = sign;
	num.base = 10;

	return num;
}

number_t number_from_uint(uint64_t value){
	return number_from(value, 0, true);
}

number_t number_from_int(int64_t value){
	return number_from(
		~(value - 1), // abs
		0,
		(bool)(value & 0x8000000000000000) // sign
	);
}

number_t number_from_float(double value){
	uint64_t sign = *((uint64_t*)(&value)) & 0x8000000000000000;
	// uint64_t exp = (*((uint64_t*)(&value)) & 0x7FF0000000000000) >> 52;
	// uint64_t man = *((uint64_t*)(&value)) & 0x000FFFFFFFFFFFFF + 1;

	double integer;  
	double decimal = modf(value, &integer);  

	return number_from((uint64_t)fabs(integer), (uint64_t)decimal, (bool)sign);
}

number_t number_from_string_numbers(const char *value){
	number_t num = {.base =10, 0};

	char digitsBuffer[NUMBER_CONSTRUCT_SIZE] = {0};
	size_t digits = 0;
	char decimalBuffer[NUMBER_CONSTRUCT_SIZE] = {0};
	size_t decimals = 0;

	// parse number
	char *buffer = digitsBuffer;
	size_t *count = &digits;
	for(size_t i = 0; i < (NUMBER_CONSTRUCT_SIZE*2); i++){
		// only on first '-'
		if(value[i] == '-' && num.sign == true){
			num.sign = false;
		}
		// on '+' just continue
		else if(value[i] == '+'){
			continue;
		}
		// on dot
		else if(value[i] == '.'){
			// change to decimal
			if(buffer == digitsBuffer){
				buffer = decimalBuffer;
				count = &decimals;
			}
			// only once
			else{
				continue;
			}
		}
		// numbers
		else if(value[i] >= '0' && value[i] <= '9'){
			buffer[i] = value[i] - '0';		
			(*count)++;
		}
		// any other character
		else{
			break;
		}
	}

	// digits
	for(size_t i = 0; i < digits; i++){
		num.digits[i] = digitsBuffer[digits - 1 - i];
	}
	// decimal
	for(size_t i = 0; i < decimals; i++){
		num.mantissa[i] = decimalBuffer[i];
	}

	num.digitsSize = digits;
	num.mantissaSize = decimals;

	return num;
}

number_t number_from_string_hex(const char *value){
	number_t num = {.base = 16, 0};

	char digitsBuffer[NUMBER_CONSTRUCT_SIZE] = {0};
	size_t digits = 0;
	for(size_t i = 0; i < NUMBER_CONSTRUCT_SIZE; i++){
		if(value[i] >= '0' && value[i] <= '9'){
			digitsBuffer[i] = value[i] - '0';		
			digits++;
		}
		else if(value[i] >= 'a' && value[i] <= 'z'){
			digitsBuffer[i] = value[i] - 'a' + 10;		
			digits++;
		}
		else if(value[i] >= 'A' && value[i] <= 'Z'){
			digitsBuffer[i] = value[i] - 'A' + 10;		
			digits++;
		}
		else{
			break;
		}
	}

	for(size_t i = 0; i < digits; i++){
		num.digits[i] = digitsBuffer[digits - 1 - i];
	}

	num.digitsSize = digits;

	return num;
}

number_t number_from_string_alphabet(const char *value){
	number_t num = {.base = 26, 0};

	char digitsBuffer[NUMBER_CONSTRUCT_SIZE] = {0};
	size_t digits = 0;
	for(size_t i = 0; i < NUMBER_CONSTRUCT_SIZE; i++){
		if(value[i] >= 'a' && value[i] <= 'z'){
			digitsBuffer[i] = value[i] - 'a';
			digits++;
		}
		else if(value[i] >= 'A' && value[i] <= 'Z'){
			digitsBuffer[i] = value[i] - 'A';
			digits++;
		}
		else{
			break;
		}
	}

	for(size_t i = 0; i < digits; i++){
		num.digits[i] = digitsBuffer[digits - 1 - i];
	}

	num.digitsSize = digits;

	return num;
}

number_t number_convert(number_t num, uint8_t base){
	number_t new = {.base = base, 0};
	
	uint64_t basePower = 1; 
	// every num digit
	for(size_t i = 0; i < num.digitsSize; i++){
		size_t j = 0;
		uint16_t digit = num.digits[i] * basePower;
		
		// divide until quotient == zero
		div_t d;
		for(d = div(digit, base); d.quot != 0; d = div(digit, base)){
			new.digits[j] += d.rem;

			// overflow
			if(new.digits[j] >= base){
				div_t over = div(new.digits[j], base);
				new.digits[j] = over.rem;
				new.digits[j+1] += over.quot;
				if(j + 1 > new.digitsSize) new.digitsSize = j + 1;
			} 

			j++;
			digit = d.quot;
		}
		new.digits[j] += d.rem;
		if(new.digits[j] >= base){
			div_t over = div(new.digits[j], base);
			new.digits[j] = over.rem;
			new.digits[j+1] += over.quot;
			if(j + 1 > new.digitsSize) new.digitsSize = j + 1;
		} 

		if(j > new.digitsSize) new.digitsSize = j;
		basePower *= num.base;
	}

	// one over last number
	new.digitsSize += 1;

	return new;
}

const char encodeHexNumber[256] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

char *number_to_hexstring(number_t num){
	char *value = calloc(sizeof(char), num.digitsSize + 1);

	for(size_t i = 0; i < num.digitsSize; i++){
		value[num.digitsSize - 1 - i] = encodeHexNumber[num.digits[i]];
	}
	
	return value;
}

uint64_t number_to_uint(number_t num){
	uint64_t value = 0;
	uint64_t basePower = 1; 

	for(size_t i = 0; i < num.digitsSize; i++){
		value += basePower * num.digits[i];
		basePower *= num.base;
	}
	
	return value;
}

int64_t number_to_int(number_t num){
	int64_t value = 0;
	uint64_t basePower = 1; 

	for(size_t i = 0; i < num.digitsSize; i++){
		value += basePower * num.digits[i];
		basePower *= num.base;
	}
	
	value *= (-1 * num.sign);

	return value;
}

double number_to_float(number_t num){
	double value = 0;
	uint64_t basePower = 1; 

	for(size_t i = 0; i < num.digitsSize; i++){
		value += basePower * num.digits[i];
		basePower *= num.base;
	}

	for(size_t i = 0; i < num.mantissaSize; i++){
		value += (basePower / (double)num.mantissa[i]);
		basePower *= num.base;
	}
	
	value *= (-1.0 * num.sign);

	return value;
}