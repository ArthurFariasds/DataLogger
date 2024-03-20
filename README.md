# Especificações Técnicas

| Parâmetro               | Especificação                                      |
|-------------------------|----------------------------------------------------|
| Faixa de temperatura    | 0°C a 50°C                                          |
| Precisão (temperatura)  | ±4% RH | ±2°C                                       |
| Faixa de umidade        | 20% a 90% UR                                        |
| Precisão (umidade)      | ±5% UR                                             |
| LDR                     | Resistência no escuro: 1 MΩ (Lux 0) <br> Resistência na luz: 10-20 KΩ (Lux 10) |
| Área visível LCD        | 64,5 x 16mm (2 linhas x 16 caracteres)             |
| Alimentação             | Bateria 9V com suporte para bateria                |

# Manual de Operação do Data Logger

## 1. Introdução
O Data Logger baseado na plataforma Arduino UNO R3 foi desenvolvido para monitorar e registrar os valores médios de luminosidade, temperatura e umidade de um ambiente. Este manual fornece instruções detalhadas sobre como operar o equipamento de forma eficaz e segura.

## 2. Componentes do Equipamento
- Arduino UNO R3
- LDR 
- DHT-11
- Display de Cristal Líquido (LCD) – 16x2 – I2C
- Botão de controle
- LED verde e vermelho
- Buzzer (alarme sonoro)
- Bateria 9V

## 3. Inicialização
- Conecte com o cabo USB na energia na bateria ou na computador.
- Para verificar os logs, conecte o arduino com cabo USB no computador e utilizando o Arduino IDE verifique a saída no console.
- Certifique-se de que todos os sensores estão corretamente conectados aos pinos correspondentes.

## 4. Interface do Display de Cristal Líquido (LCD)
- Ao ligar o dispositivo, o nome da empresa será exibido no display.
- O display mostrará continuamente os valores médios de luminosidade, temperatura e umidade do ambiente.
- Caso a faixa de temperatura, luminosidade ou umidade esteja fora dos limites especificados, um alarme será acionado e o LED vermelho ficará acesso.

## 5. Botão de Controle
- Ao pressionar o botão de controle, o log de dados armazenado na memória EEPROM será exibido no console do Arduino IDE.
- Os dados armazenados incluem valores de luminosidade, temperatura, umidade.

## 6. Alarme Sonoro e Luminoso
- Se os valores medidos estiverem fora das faixas especificadas, um alarme sonoro será acionado através do buzzer.
- Além disso, o LED correspondente (vermelho ou verde) indicará o status do ambiente:
  - LED verde aceso: Valores dentro da faixa especificada.
  - LED vermelho aceso: Valores fora da faixa especificada.
- Os níveis dos triggers estão programados da seguinte maneira:
  - Faixa de temperatura: Menores que 15°C e maiores que 25°C.
  - Faixa de luminosidade: Menores que 0% e maiores que 30%.
  - Faixa de umidade: Menores que 30% e maiores que 50%.

## 7. Procedimento de Desligamento
- Para desligar o equipamento, desconecte-o da fonte de energia.

## 8. Manutenção
- Mantenha os sensores limpos e livres de obstruções para garantir medições precisas.
- Verifique regularmente a funcionalidade do dispositivo e substitua qualquer componente danificado conforme necessário.

## 9. Segurança
- Este dispositivo contém componentes eletrônicos sensíveis. Evite exposição a choques mecânicos ou eletricidade estática.
- Não opere o equipamento em condições ambientais extremas que possam comprometer sua integridade ou funcionalidade.
