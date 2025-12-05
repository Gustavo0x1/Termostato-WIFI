# Termostato-WIFI

Arquivos INO do projeto de termostato Wi-Fi, feito no IFNMG campus Montes Claros.

Visão Geral do Projeto

O projeto consiste em um transmissor (ESP32) acoplado a um sensor LM35 que realiza medições da temperatura ambiente e envia os dados via ESP-NOW (wireless) para uma placa receptora (ESP8266). A placa ESP8266 atua como ponto de acesso Wi-Fi e hospeda um servidor web que permite ao usuário monitorar a temperatura e configurar gatilhos para manipulação de portas no receptor com base nos dados recebidos.


Pré-requisitos

Hardware
1x Placa ESP32 ( utilizado: esp32 Wemos D1 R32 ) 
1x Placa ESP8266 ( utilizado: esp8266 NodeMCU v3 ) 
1x Sensor LM35
1x Módulo Relé (opcional)

Conexões:
Transmissor:
Por padrão o terminal de saída do LM35 (central) é conectado ao IO34 no esp32

Receptor:
Por padrão as portas digitais D1 e D2 são as portas configuraveis. 
