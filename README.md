# 🔐 Cofre Digital com Microcontrolador

Este projeto consiste no desenvolvimento de um **cofre digital eletrônico**, implementado no microcontrolador AtMega328, com interface de usuário baseada em **displays de 7 segmentos**, botões físicos, **LED RGB** e **buzzer**.  
O sistema foi desenvolvido como um mini projeto acadêmico, com foco na aplicação prática de periféricos como GPIO, Timers e Interrupções.

---

## 🎯 Objetivo
Aplicar conceitos de sistemas embarcados utilizando:
- GPIO
- Timers
- Interrupções
- PWM

O projeto simula um **cofre digital**, permitindo a inserção de senha e fornecendo feedback visual e sonoro ao usuário.

---

## ⚙️ Funcionalidades
- Inserção de senha de **3 dígitos**
- Displays de 7 segmentos multiplexados
- Incremento de cada dígito por botão dedicado
- Confirmação da senha via botão
- LED RGB:
  - 🔴 Vermelho: cofre fechado
  - 🟢 Verde: cofre aberto
- Buzzer com PWM:
  - 🔊 2 bips ao abrir o cofre
  - 🔊 1 bip ao fechar o cofre
  - 🔊 bip constante quando erra a senha com o cofre fechado
- Uso de interrupções para leitura de botões e controle de tempo

---

## 🧩 Componentes Utilizados
- Microcontrolador ATmega328
- 3 Displays de 7 segmentos (catodo comum)
- Botões de pressão
- LED RGB
- Buzzer passivo
- Resistores
- Transistor NPN
- SimulIDE (simulação)
- Atmel Studio 7 (desenvolvimento)

---

## ⏱️ Periféricos Utilizados
- **GPIO**: controle de LEDs, displays, botões e buzzer
- **Timer0**: multiplexação dos displays e controle de tempo
- **Timer1**: geração de PWM para o buzzer
- **Interrupções**:
  - Interrupção por comparação (Timer)
  - Interrupção por mudança de pino (PCINT)

---

## 🖥️ Simulação
O circuito foi simulado no **SimulIDE**, contendo:
- Displays de 7 segmentos
- Botões de entrada
- LED RGB
- Buzzer PWM

O arquivo de simulação encontra-se disponível no repositório.

---

## ▶️ Demonstração
Vídeo de demonstração do funcionamento do sistema:
- 📹 [Video de Demonstração]()


---

## 👨‍💻 Autor
Projeto desenvolvido por **Daniel Neto**.
