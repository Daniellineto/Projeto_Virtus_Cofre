#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>

/*
 * Mapeamento dos segmentos do display de 7 segmentos
 * PC6 = segmento A
 * PC5 = segmento B
 * PC4 = segmento C
 * PC3 = segmento D
 * PC2 = segmento E
 * PC1 = segmento F
 * PC0 = segmento G
 */
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

/* =====================================================
 * VARIÁVEIS GLOBAIS
 * ===================================================== */

// Senha digitada pelo usuário
volatile uint8_t senha[3] = {0, 0, 0};

// Senha correta do cofre
const uint8_t senha_correta[3] = {1, 2, 3};

// Controle da multiplexação dos displays
volatile uint8_t display_atual = 0;

// Estado do cofre
// 0 = fechado | 1 = aberto
volatile uint8_t cofre_aberto = 0;

// Controle do buzzer
volatile uint8_t  beep_count  = 0;   // quantidade de bips restantes
volatile uint16_t beep_timer  = 0;   // temporizador de cada bip
volatile uint8_t  buzzer_state = 0;  // 0 = parado | 1 = tocando | 2 = pausa


/* =====================================================
 * FUNÇÃO DE ATIVAÇÃO DOS DISPLAYS (MULTIPLEXAÇÃO)
 * ===================================================== */
void ativa_display(uint8_t d)
{
	// Desliga todos os displays
	PORTD &= ~(1 << PD5);
	PORTB &= ~((1 << PB6) | (1 << PB7));

	// Ativa apenas o display selecionado
	if (d == 0) PORTB |= (1 << PB6);
	if (d == 1) PORTB |= (1 << PB7);
	if (d == 2) PORTD |= (1 << PD5);
}


/* =====================================================
 * INTERRUPÇÃO DO TIMER0 (1 ms)
 * - Multiplexação dos displays
 * - Controle de bips do buzzer
 * ===================================================== */
ISR(TIMER0_COMPA_vect)
{
	// Atualiza segmentos do display atual
	PORTC = digitos[ senha[display_atual] ];
	ativa_display(display_atual);

	// Avança para o próximo display
	display_atual++;
	if (display_atual > 2)
		display_atual = 0;

	/* -------- CONTROLE DOS BIPS DO BUZZER -------- */
	if (beep_count > 0)
	{
		beep_timer++;

		if (buzzer_state == 0)
		{
			// Início do bip
			TCCR1A |= (1 << COM1A1);   // habilita PWM
			buzzer_state = 1;
			beep_timer = 0;
		}
		else if (buzzer_state == 1 && beep_timer >= 200)
		{
			// Fim do bip
			TCCR1A &= ~(1 << COM1A1);  // desabilita PWM
			buzzer_state = 2;
			beep_timer = 0;
		}
		else if (buzzer_state == 2 && beep_timer >= 150)
		{
			// Fim da pausa entre bips
			buzzer_state = 0;
			beep_timer = 0;
			beep_count--;
		}
	}
	else
	{
		// Garante que o buzzer permaneça desligado
		TCCR1A &= ~(1 << COM1A1);
		buzzer_state = 0;
	}
}


/* =====================================================
 * INTERRUPÇÃO DE MUDANÇA DE PINO (PCINT)
 * - Leitura dos botões
 * - Incremento da senha
 * - Confirmação da senha
 * ===================================================== */
ISR(PCINT2_vect)
{
	static uint8_t last = 0xFF;
	uint8_t atual = PIND;

	// Incrementa cada dígito da senha
	if (!(atual & (1 << PD1)) && (last & (1 << PD1)))
		senha[0] = (senha[0] + 1) % 10;

	if (!(atual & (1 << PD2)) && (last & (1 << PD2)))
		senha[1] = (senha[1] + 1) % 10;

	if (!(atual & (1 << PD3)) && (last & (1 << PD3)))
		senha[2] = (senha[2] + 1) % 10;

	// Botão de confirmação
	if (!(atual & (1 << PD4)) && (last & (1 << PD4)))
	{
		// SENHA CORRETA
		if (senha[0] == senha_correta[0] &&
		senha[1] == senha_correta[1] &&
		senha[2] == senha_correta[2])
		{
			cofre_aberto ^= 1;

			beep_timer = 0;
			buzzer_state = 0;

			if (cofre_aberto)
			beep_count = 2;   // abriu
			else
			beep_count = 1;   // fechou
		}
		// SENHA ERRADA E COFRE FECHADO
		else if (!cofre_aberto)
		{
			beep_timer = 0;
			buzzer_state = 0;
			beep_count = 10;  // alarme de erro
		}
	}


	last = atual;
}


/* =====================================================
 * FUNÇÃO PRINCIPAL
 * ===================================================== */
int main(void)
{
	/* -------- CONFIGURAÇÃO DE I/O -------- */

	// Segmentos do display
	DDRC |= 0b01111111;

	// Controle dos displays (transistores)
	DDRD |= (1 << PD5);
	DDRB |= (1 << PB6) | (1 << PB7);

	// Botões com pull-up
	DDRD &= ~((1 << PD1)|(1 << PD2)|(1 << PD3)|(1 << PD4));
	PORTD |=  (1 << PD1)|(1 << PD2)|(1 << PD3)|(1 << PD4);

	// LED RGB
	DDRB |= (1 << PB3) | (1 << PB4);

	// Buzzer (PWM - OC1A)
	DDRB |= (1 << PB1);

	// Estado inicial: cofre fechado (vermelho)
	PORTB |= (1 << PB4);
	PORTB &= ~(1 << PB3);

	/* -------- TIMER0 (1 ms) -------- */
	TCCR0A = (1 << WGM01);
	OCR0A  = 249;
	TIMSK0 = (1 << OCIE0A);
	TCCR0B = (1 << CS01) | (1 << CS00);

	/* -------- TIMER1 (PWM do buzzer ~2 kHz) -------- */
	TCCR1A = (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
	ICR1   = 1499;
	OCR1A  = 750;

	/* -------- INTERRUPÇÕES DE PINO -------- */
	PCICR  |= (1 << PCIE2);
	PCMSK2 |= (1 << PD1)|(1 << PD2)|(1 << PD3)|(1 << PD4);

	sei();

	/* -------- LOOP PRINCIPAL -------- */
	while (1)
	{
		// Atualiza LED RGB conforme estado do cofre
		if (cofre_aberto)
		{
			PORTB |= (1 << PB3);   // verde ON
			PORTB &= ~(1 << PB4);  // vermelho OFF
		}
		else
		{
			PORTB |= (1 << PB4);   // vermelho ON
			PORTB &= ~(1 << PB3);  // verde OFF
		}
	}
}
