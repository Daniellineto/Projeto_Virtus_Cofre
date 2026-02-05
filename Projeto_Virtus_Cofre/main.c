
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

// Segmentos: PC6=a, PC5=b, PC4=c, PC3=d, PC2=e, PC1=f, PC0=g
const uint8_t digitos[10] = {
	0b01111110, // 0
	0b00110000, // 1
	0b01101101, // 2
	0b01111001, // 3
	0b00110011, // 4
	0b01011011, // 5
	0b01011111, // 6
	0b01110000, // 7
	0b01111111, // 8
	0b01111011  // 9
};

// ---------------- VARIÁVEIS GLOBAIS ----------------
volatile uint8_t senha[3] = {0, 0, 0};
const uint8_t senha_correta[3] = {1, 2, 3};

volatile uint8_t display_atual = 0;
volatile uint8_t cofre_aberto = 0;

volatile uint8_t beep_count = 0;
volatile uint16_t beep_timer = 0;
volatile uint8_t buzzer_state = 0; // 0 = parado, 1 = tocando, 2 = pausa


// ---------------- DISPLAY ----------------
void ativa_display(uint8_t d)
{
	PORTD &= ~(1 << PD5);
	PORTB &= ~((1 << PB6) | (1 << PB7));

	if (d == 0) PORTB |= (1 << PB6);
	if (d == 1) PORTB |= (1 << PB7);
	if (d == 2) PORTD |= (1 << PD5);
}

// ---------------- TIMER0 (1 ms) ----------------
ISR(TIMER0_COMPA_vect)
{
	PORTC = digitos[ senha[display_atual] ];
	ativa_display(display_atual);

	display_atual++;
	if (display_atual > 2)
	display_atual = 0;

	// -------- CONTROLE DOS BIPS --------
	if (beep_count > 0)
	{
		beep_timer++;

		if (buzzer_state == 0)
		{
			// inicia bip
			TCCR1A |= (1 << COM1A1);   // liga PWM
			buzzer_state = 1;
			beep_timer = 0;
		}

		else if (buzzer_state == 1 && beep_timer >= 200)
		{
			// fim do bip
			TCCR1A &= ~(1 << COM1A1); // desliga PWM
			buzzer_state = 2;
			beep_timer = 0;
		}

		else if (buzzer_state == 2 && beep_timer >= 150)
		{
			// fim da pausa
			buzzer_state = 0;
			beep_timer = 0;
			beep_count--;
		}
	}
	else
	{
		// garante buzzer desligado
		TCCR1A &= ~(1 << COM1A1);
		buzzer_state = 0;
	}
}

// ---------------- BOTÕES (PCINT) ----------------
ISR(PCINT2_vect)
{
	static uint8_t last = 0xFF;
	uint8_t atual = PIND;

	if (!(atual & (1 << PD1)) && (last & (1 << PD1)))
	senha[0] = (senha[0] + 1) % 10;

	if (!(atual & (1 << PD2)) && (last & (1 << PD2)))
	senha[1] = (senha[1] + 1) % 10;

	if (!(atual & (1 << PD3)) && (last & (1 << PD3)))
	senha[2] = (senha[2] + 1) % 10;

	// CONFIRMAR
	if (!(atual & (1 << PD4)) && (last & (1 << PD4)))
	{
		if (senha[0] == senha_correta[0] &&
		senha[1] == senha_correta[1] &&
		senha[2] == senha_correta[2])
		{
			cofre_aberto ^= 1;

			if (cofre_aberto)
			beep_count = 2; // abriu
			else
			beep_count = 1; // fechou
		}
	}

	last = atual;
}

// ---------------- MAIN ----------------
int main(void)
{
	// Segmentos
	DDRC |= 0b01111111;

	// Displays
	DDRD |= (1 << PD5);
	DDRB |= (1 << PB6) | (1 << PB7);

	// Botões
	DDRD &= ~((1 << PD1)|(1 << PD2)|(1 << PD3)|(1 << PD4));
	PORTD |=  (1 << PD1)|(1 << PD2)|(1 << PD3)|(1 << PD4);

	// LED RGB
	DDRB |= (1 << PB3) | (1 << PB4);

	// BUZZER PWM PB1 (OC1A)
	DDRB |= (1 << PB1);

	// LED inicial
	PORTB |= (1 << PB4);
	PORTB &= ~(1 << PB3);

	// TIMER0 – multiplexação
	TCCR0A = (1 << WGM01);
	OCR0A  = 249;
	TIMSK0 = (1 << OCIE0A);
	TCCR0B = (1 << CS01) | (1 << CS00);

	// TIMER1 – PWM buzzer (~2 kHz)
	TCCR1A = (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
	ICR1   = 1499;    // frequência
	OCR1A  = 750;    // duty 50%

	// PCINT
	PCICR  |= (1 << PCIE2);
	PCMSK2 |= (1 << PD1)|(1 << PD2)|(1 << PD3)|(1 << PD4);

	sei();

	while (1)
	{
		if (cofre_aberto)
		{
			PORTB |= (1 << PB3);
			PORTB &= ~(1 << PB4);
		}
		else
		{
			PORTB |= (1 << PB4);
			PORTB &= ~(1 << PB3);
		}
	}
}
