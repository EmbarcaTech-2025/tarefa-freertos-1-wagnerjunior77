# 🎯 Tarefa: Roteiro de FreeRTOS #1 - EmbarcaTech 2025

**Autor:** Wagner Junior
**Curso:** Residência Tecnológica em Sistemas Embarcados  
**Instituição:** EmbarcaTech - HBr  
**Local:** Brasília, 13 de Junho de 2025  

---

## 🧠 Descrição

Projeto embarcado desenvolvido na placa **BitDogLab (RP2040)** com o sistema operacional **FreeRTOS**, controlando três periféricos de forma concorrente:

- 🔴 **LED RGB** alternando entre vermelho, verde e azul a cada 500 ms.
- 🔊 **Buzzer** emitindo um beep curto a cada 1 segundo.
- 🔘 **Dois botões**:
  - Botão A: pausa/retoma a tarefa do LED.
  - Botão B: pausa/retoma a tarefa do buzzer.
- 🖥️ **Display OLED I²C** exibindo o estado atual das tarefas (RUN/PAUSE).

---

## 🛠️ Funcionalidades

- Uso de **FreeRTOS com semáforos binários** para controle de tarefas.
- **Prioridades diferenciadas**: botões têm prioridade mais alta que o LED e buzzer.
- Sistema multitarefa **eficiente e responsivo**.
- Atualização visual em tempo real no display.

---

## 📐 Pinos utilizados (BitDogLab)

| Função     | GPIO |
|------------|------|
| LED Verm.  | 11   |
| LED Verde  | 12   |
| LED Azul   | 13   |
| Buzzer     | 21   |
| Botão A    | 5    |
| Botão B    | 6    |
| OLED SDA   | 14   |
| OLED SCL   | 15   |

---

## 🧪 Reflexões

**O que acontece se todas as tarefas tiverem a mesma prioridade?**  
O sistema usa *round-robin*, e a resposta ao botão pode ficar levemente atrasada.

**Qual tarefa consome mais CPU?**  
A tarefa do OLED, por causa da escrita I²C frequente.

**Quais os riscos de polling sem prioridades?**  
Maior latência, uso excessivo da CPU e perda de responsividade.

---

## 📜 Licença

Distribuído sob os termos da licença **GNU General Public License v3.0**.  
Consulte o arquivo [`LICENSE`](https://www.gnu.org/licenses/gpl-3.0.html) para mais detalhes.
