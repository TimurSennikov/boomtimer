#include <stdio.h>
#include <iostream>

#include "pico/stdlib.h"
#include "pico/bootrom.h"

#define OUT_PIN 29
#define COUNTPP_AMOUNT 70

#if OUT_PIN == 0 || COUNTPP_AMOUNT == 0
	#warning "Check preprocessor variables, code might break because of them!"
#endif

using namespace std;

void ledInit(){
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

/*class Microphone{
	private:
		int pin;
		uint16_t lastMeasure;
	public:
		Microphone(int bpin){
			adc_init();
			adc_gpio_init(bpin);

			pin = bpin;
		}

		uint16_t measure(){
			adc_select_input(26 + pin);

			return adc_read();
		}
};*/

class Sound{
	public:
		int duration;
		int nextDelay;

		Sound(int d, int n){
			duration = d;
			nextDelay = n;
		}
};

class piLed{ // встроенный светодиод (класс не статичный просто потому что)
	private:
		int pin = PICO_DEFAULT_LED_PIN;
	public:
		piLed(){
			try{
				gpio_put(pin, 0);
			}
			catch(...){
				gpio_init(pin);
				gpio_set_dir(pin, GPIO_OUT);
			}
		}

		void on(){
			gpio_put(pin, 1);
		}
		void off(){
			gpio_put(pin, 0);
		}

		void single(int duration){
			gpio_put(pin, 1);
			sleep_ms(duration);
			gpio_put(pin, 0);
			sleep_ms(duration);
		}

		void repeat(int times, int duration){
			if(times < 0){single(300); return;}

			for(int i = 0; i < times; i++){
				single(duration / 2);
			}
		}

		void beat(int* data, int length){
			for(int i = 0; i < length; i++){
				int s = (*(data + i));

				on();
				sleep_ms(s);
				off();
				sleep_ms(s);
			}
		}
};

class Button{ // кнопка
	private:
		int pin;
	public:
		Button(int bpin){
			try{
				gpio_get(bpin);
			}
			catch(...){
				gpio_init(bpin);
				gpio_set_dir(bpin, GPIO_IN);
			}
			pin = bpin;
		}

		bool isPressed(){
			if(gpio_get(pin) != 0){return true;}
			else{return false;}
		}
};

class Buzzer/*: public SomeCoolPinClass*/ { // пищалка (на самом деле можно было бы сделать род. класс и extend`ить его в однотипных классах, но мне лень).
	private:
		int pin;
	public:
		Buzzer(int bpin){
			try{
				gpio_put(bpin, 0);
			}
			catch(...){
				gpio_init(bpin);
				gpio_set_dir(bpin, GPIO_OUT);
			}

			pin = bpin;
		}

		void beep(int duration){
			gpio_put(pin, 1);
			sleep_ms(duration);
			gpio_put(pin, 0);
			sleep_ms(duration);
		}
};

class display_4c7s{
	private:
		uint16_t numTable[10] = {
			0b1111110, // 0
			0b1111000, // 1
			0b1100101, // 2
			0b1111001, // 3
			0b0110011, // 4
			0b1011011, // 5
			0b1011111, // 6
			0b1111000, // 7
			0b1111111, // 8
			0b1111011  // 9
		};

		int D1, D2, D3, D4, A, B, C, D, E, F, G, DP;

		int digitC = 3; // starting from 0.
		int* digits[4] = {&D1, &D2, &D3, &D4};

		int pinC = 7;
		int* pins[8] = {&A, &B, &C, &D, &E, &F, &G, &DP};
	public:
		display_4c7s(int d1, int d2, int d3, int d4, int a, int b, int c, int d, int e, int f, int g, int dp){
			D1 = d1;
			D2 = d2;
			D3 = d3;
			D4 = d4;

			A = a;
			B = b;
			C = c;
			D = d;
			E = e;
			F = f;
			G = g;
			DP = dp;

			for(int i = 0; i < pinC; i++){
				int pin = *(pins[i]);

				try{
					gpio_init(pin);
					gpio_set_dir(pin, GPIO_OUT);
				}
				catch(...){}
			}

			for(int i = 0; i < digitC; i++){
				int digit = *(digits[i]);

				try{
					gpio_init(digit);
					gpio_set_dir(digit, GPIO_OUT);
				}
				catch(...){}
			}
		}

		void showChar(int pos, uint16_t code){
			gpio_put(*(digits[pos]), 1);

			uint16_t c = 0b0000001;

			for(int i = 0; i < 7; i++){
				if((c << i) & (code)){gpio_put(*(pins[i]), 1);}
			}
		}

		void showNum(int number){ // не уверен что работает, не проверял, будьте готовы ловить сегфолты
			int num = number > 9999 ? 9999 : number;
			char snum[1024];

			sprintf(snum, "%d", num);

			for(int i = 0; i < 3; i++){
				if(!stoi(&snum[i])){continue;}

				this->showChar(i, numTable[stoi(&snum[i])]);
			}
		}

		void reset(){
			for(int i = 0; i < pinC; i++){
				gpio_put(*(pins[i]), 0);
			}

			for(int i = 0; i < digitC; i++){
				gpio_put(*(digits[i]), 0);
			}
		}
};

// туду: может LCD?

Button add = *(new Button(3));
Button start = *(new Button(17));
Button res = *(new Button(9));
Buzzer speaker = *(new Buzzer(15));
//display_4c7s display = *(new display_4c7s(18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 15)); // йоу этот йоу класс йоу еще йоу не йоу готов йоу

void cooldown(int* cd, piLed* led){
	int cooldown;

	while(1){
		cooldown = *cd < COUNTPP_AMOUNT ? *cd * 10 : 500;

		(*led).single(cooldown / 4);
		sleep_ms(cooldown / 2);
		(*led).single(cooldown / 4);
		sleep_ms(cooldown / 2);

		(*cd)--;
		if(*cd <= 0){
			break;
		}
	}

	gpio_put(OUT_PIN, 1);
	(*led).on();
}

void devMode(){
	reset_usb_boot(0, 0);
}

int main() {
	stdio_init_all();

	ledInit();

	gpio_init(OUT_PIN);
	gpio_set_dir(OUT_PIN, GPIO_OUT);

	piLed led = *(new piLed()); // в rpi-sdk почему-то при создании через new возвращается адрес класса в памяти, а не сам класс, поэтому нужно дереференсировать.

	led.single(1000);

	int count = 0;
	int startInARow = 0;

	while(1){ // програм луп
		if(add.isPressed() && start.isPressed()){
			sleep_ms(100);
			led.single(10);
			break;
		}
		else if(add.isPressed()){
			count += COUNTPP_AMOUNT;
			led.repeat(count / COUNTPP_AMOUNT, 150);
			sleep_ms(300);
		}
		else if(start.isPressed()){
			if(startInARow >= 3){led.single(1000); devMode();}

			else if(count <= 0){startInARow++; led.single(100); continue;}
			else{led.repeat(3, 100);}

			startInARow = 0;

			cooldown(&count, &led);

			while(1){if(res.isPressed() || (add.isPressed() && start.isPressed())){sleep_ms(1000); break;}}
			led.repeat(5, 150);
			continue;
		}
		else if(res.isPressed()){
			gpio_put(OUT_PIN, 0);

			count = 0;

			led.single(500);
			sleep_ms(100);
		}
	}

	while(1){
		led.repeat(100, 2);
		sleep_ms(500);
		led.repeat(100, 2);
		sleep_ms(500);
	}
}
