#include <ESP8266WiFi.h>
#include <espnow.h>
#include <ESP8266WebServer.h>
#define PINO_RELE 5        
#define PINO_RELE_1 5 
#define PINO_RELE_2 4 
bool modo_edicao = false;

float limite_high_1 = 18.0;   
float limite_low_1 = 15.0;    
bool estado_rele_1 = false;   


int modo_operacao_2 = 1; 



int acao_condicional_2 = 1; 
// PIN D1
int modo_operacao_1 = 1;      
int acao_manual_1 = 0;       
int acao_condicional_1 = 1; 
int condicao_op_1_1 = 0;     
float condicao_val_1_1 = 20.0;
int condicao_op_2_1 = 2;  
float condicao_val_2_1 = 10.0;

int condicao_op_1 = 0; 
float condicao_val_1 = 20.0;
int condicao_op_1_2 = 0;    
float condicao_val_1_2 = 25.0;
int condicao_op_2_2 = 2;     
float condicao_val_2_2 = 15.0;
bool estado_rele_2 = false;

bool estado_rele = false;
const char* ssid_ap = "Termometro_UCE2";
const char* password_ap = "12345678";


ESP8266WebServer server(80);
typedef struct __attribute__((packed)) {
  float temperatura;
  uint16_t adc;
} pacote_t;

#define HISTORY_SIZE 10
float temp_history[HISTORY_SIZE] = {0.0};
float ultima_temp = 0.0;
uint16_t ultimo_adc = 0;
float ultima_tensao = 0.0;
unsigned long ultimo_recebido = 0;
int contador = 0;

void add_to_history(float temp) {
  for (int i = 0; i < HISTORY_SIZE - 1; i++) {
    temp_history[i] = temp_history[i + 1];
  }
  temp_history[HISTORY_SIZE - 1] = temp;
}
void verificarTemperatura() {

  aplicarLogicaPino(PINO_RELE_1, modo_operacao_1, acao_manual_1, acao_condicional_1, 
                    condicao_op_1_1, condicao_val_1_1, condicao_op_2_1, condicao_val_2_1, 
                    estado_rele_1);


  aplicarLogicaPino(PINO_RELE_2, modo_operacao_2, acao_manual_2, acao_condicional_2, 
                    condicao_op_1_2, condicao_val_1_2, condicao_op_2_2, condicao_val_2_2, 
                    estado_rele_2);
}
void aplicarLogicaPino(int pino, int modo_operacao, int acao_manual, int acao_condicional, 
                        int condicao_op_1, float condicao_val_1, 
                        int condicao_op_2, float condicao_val_2, 
                        bool &estado_rele) {
  
  if (modo_operacao == 0) { 

    if (acao_manual == 1) {
      digitalWrite(pino, HIGH);
      estado_rele = true;
    } else {
      digitalWrite(pino, LOW);
      estado_rele = false;
    }
    
  } else { 

    bool condicao_1_ok = false;
    bool condicao_2_ok = false;

    if (condicao_op_1 == 0) {
      condicao_1_ok = (ultima_temp >= condicao_val_1);
    } else if (condicao_op_1 == 1) { 
      condicao_1_ok = (ultima_temp <= condicao_val_1);
    } else { 
      condicao_1_ok = true;
    }


    if (condicao_op_2 == 0) {
      condicao_2_ok = (ultima_temp >= condicao_val_2);
    } else if (condicao_op_2 == 1) {
      condicao_2_ok = (ultima_temp <= condicao_val_2);
    } else { 
      condicao_2_ok = true;
    }


    bool condicoes_ok = condicao_1_ok && condicao_2_ok;

    if (condicoes_ok) {
  
      if (acao_condicional == 1) { 
        digitalWrite(pino, HIGH);
        estado_rele = true;
      } else { 
        digitalWrite(pino, LOW);
        estado_rele = false;
      }
    } else {

      if (acao_condicional == 1) { 

        digitalWrite(pino, LOW);
        estado_rele = false;
      } else { 

        digitalWrite(pino, HIGH);
        estado_rele = true;
      }
    }
  }
}

String gerarHtmlControle() {
  String html = "";


  if (server.hasArg("salvar_d1")) {

    modo_operacao_1 = server.arg("p1_modo_operacao").toInt(); 
    acao_manual_1 = server.arg("p1_acao_manual").toInt();
    acao_condicional_1 = server.arg("p1_acao_condicional").toInt(); 
    
    condicao_op_1_1 = server.arg("p1_condicao_op_1").toInt(); 
    condicao_val_1_1 = server.arg("p1_condicao_val_1").toFloat();
    condicao_op_2_1 = server.arg("p1_condicao_op_2").toInt(); 
    condicao_val_2_1 = server.arg("p1_condicao_val_2").toFloat();
    
    verificarTemperatura();
    modo_edicao = false; 
  }


  if (server.hasArg("salvar_d2")) {

    modo_operacao_2 = server.arg("p2_modo_operacao").toInt();
    acao_manual_2 = server.arg("p2_acao_manual").toInt(); 
    acao_condicional_2 = server.arg("p2_acao_condicional").toInt(); 
    
    condicao_op_1_2 = server.arg("p2_condicao_op_1").toInt(); 
    condicao_val_1_2 = server.arg("p2_condicao_val_1").toFloat();
    condicao_op_2_2 = server.arg("p2_condicao_op_2").toInt(); 
    condicao_val_2_2 = server.arg("p2_condicao_val_2").toFloat();
    
    verificarTemperatura();
    modo_edicao = false;
  }
  

  if (server.hasArg("toggle_edicao")) {
    modo_edicao = !modo_edicao;
  }
  

  if (!modo_edicao) {
    html += "<div class='card'>";
    html += "<h3>Status de automação</h3>";
    

    String corStatus1 = estado_rele_1 ? "green" : "gray";
    String textoStatus1 = estado_rele_1 ? "LIGADO (HIGH)" : "DESLIGADO (LOW)";
    html += "<p>Status Porta D1: <strong style='color:" + corStatus1 + "'>" + textoStatus1 + "</strong></p>";
    

    String corStatus2 = estado_rele_2 ? "green" : "gray";
    String textoStatus2 = estado_rele_2 ? "LIGADO (HIGH)" : "DESLIGADO (LOW)";
    html += "<p>Status Porta D2: <strong style='color:" + corStatus2 + "'>" + textoStatus2 + "</strong></p>";


    html += "<form action='/' method='POST'><input type='hidden' name='toggle_edicao' value='1'>";
    html += "<input type='submit' value='Entrar em Modo de CONFIGURAÇÃO' style='background:#f90;color:#fff;border:none;padding:8px 16px;border-radius:4px;cursor:pointer;margin-top:10px;'>";
    html += "</form>";
    html += "</div>";
    
    return html;
  }


  html += "<div class='card' style='border-color:#f90;'>";
  html += "<h3>&#x1F527; Modo de Edição Ativo</h3>";
  html += "<form action='/' method='POST'><input type='hidden' name='toggle_edicao' value='1'>";
  html += "<input type='submit' value='Sair do Modo de Edição' style='background:#555;color:#fff;border:none;padding:8px 16px;border-radius:4px;cursor:pointer;margin-top:5px;margin-bottom:10px;'>";
  html += "</form>";
  html += "</div>";


  auto gerarBlocoControle = [&](int pino_num, int &modo_op, int &acao_man, int &acao_cond, int &cond_op_1, float &cond_val_1, int &cond_op_2, float &cond_val_2) {
    String pinoStr = String(pino_num);
    String prefixo = "p" + pinoStr + "_";
    
    String bloco = "";
    bloco += "<div class='card' style='margin-top:10px;'>";
    bloco += "<h3>&#x2699; Configuração PINO D" + pinoStr + "</h3>";
    
    bloco += "<form action='/' method='POST'>";
    

    bloco += "<label style='display:block;margin-bottom:8px;'>Modo de Operação:</label>";
    bloco += "<select name='" + prefixo + "modo_operacao' style='padding:5px;width:100%;margin-bottom:15px;' onchange='document.getElementById(\"" + prefixo + "manual\").style.display=this.value===\"0\"?\"block\":\"none\";document.getElementById(\"" + prefixo + "condicional\").style.display=this.value===\"1\"?\"block\":\"none\";'>";
    bloco += "<option value='1'" + String(modo_op == 1 ? " selected" : "") + ">Condicional (Termostato)</option>";
    bloco += "<option value='0'" + String(modo_op == 0 ? " selected" : "") + ">Manual (Ligar/Desligar)</option>";
    bloco += "</select>";


    bloco += "<div id='" + prefixo + "manual' style='display:" + String(modo_op == 0 ? "block" : "none") + ";border:1px dashed #ccc;padding:10px;margin-bottom:15px;'>";
    bloco += "<label style='display:block;margin-bottom:5px;'>Ação Manual:</label>";
    bloco += "<select name='" + prefixo + "acao_manual' style='padding:5px;'>";
    bloco += "<option value='1'" + String(acao_man == 1 ? " selected" : "") + ">LIGAR (HIGH)</option>";
    bloco += "<option value='0'" + String(acao_man == 0 ? " selected" : "") + ">DESLIGAR (LOW)</option>";
    bloco += "</select>";
    bloco += "</div>";

    bloco += "<div id='" + prefixo + "condicional' style='display:" + String(modo_op == 1 ? "block" : "none") + ";border:1px dashed #ccc;padding:10px;margin-bottom:15px;'>";
    bloco += "<label style='display:block;margin-bottom:10px;'>Ação a Executar se Condições OK:</label>";
    bloco += "<select name='" + prefixo + "acao_condicional' style='padding:5px;margin-bottom:15px;'>";
    bloco += "<option value='1'" + String(acao_cond == 1 ? " selected" : "") + ">LIGAR (HIGH)</option>";
    bloco += "<option value='0'" + String(acao_cond == 0 ? " selected" : "") + ">DESLIGAR (LOW)</option>";
    bloco += "</select>";


    bloco += "<p>Condição 1 (Temperatura):</p>";
    bloco += "<select name='" + prefixo + "condicao_op_1' style='padding:5px;'>";
    bloco += "<option value='0'" + String(cond_op_1 == 0 ? " selected" : "") + ">MAIOR OU IGUAL (>=)</option>";
    bloco += "<option value='1'" + String(cond_op_1 == 1 ? " selected" : "") + ">MENOR OU IGUAL (<=)</option>";
    bloco += "<option value='2'" + String(cond_op_1 == 2 ? " selected" : "") + ">Desabilitada</option>";
    bloco += "</select>";
    bloco += " que <input type='number' step='0.1' name='" + prefixo + "condicao_val_1' style='width:60px;padding:5px;' value='" + String(cond_val_1, 1) + "'> °C";
    
    bloco += "<hr style='margin:10px 0;'>";
    

    bloco += "<p>Condição 2 (Temperatura):</p>";
    bloco += "<select name='" + prefixo + "condicao_op_2' style='padding:5px;'>";
    bloco += "<option value='0'" + String(cond_op_2 == 0 ? " selected" : "") + ">MAIOR OU IGUAL (>=)</option>";
    bloco += "<option value='1'" + String(cond_op_2 == 1 ? " selected" : "") + ">MENOR OU IGUAL (<=)</option>";
    bloco += "<option value='2'" + String(cond_op_2 == 2 ? " selected" : "") + ">Desabilitada</option>";
    bloco += "</select>";
    bloco += " que <input type='number' step='0.1' name='" + prefixo + "condicao_val_2' style='width:60px;padding:5px;' value='" + String(cond_val_2, 1) + "'> °C";
    bloco += "</div>"; 

    bloco += "<input type='submit' name='salvar_d" + pinoStr + "' value='Salvar Configuração D" + pinoStr + " e Sair' style='background:#d93025;color:#fff;border:none;padding:8px 16px;border-radius:4px;cursor:pointer;'>";
    bloco += "</form>";
    bloco += "</div>";
    
    return bloco;
  };
  

  html += gerarBlocoControle(1, modo_operacao_1, acao_manual_1, acao_condicional_1, condicao_op_1_1, condicao_val_1_1, condicao_op_2_1, condicao_val_2_1);
  html += gerarBlocoControle(2, modo_operacao_2, acao_manual_2, acao_condicional_2, condicao_op_1_2, condicao_val_1_2, condicao_op_2_2, condicao_val_2_2);

  return html;
}
void handleRoot() {
  contador++;
  
  String pagina = "<!DOCTYPE html><html><head>";
  pagina += "<meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>";
  pagina += "<title>Termostato Wi-fi</title>";
  
  pagina += "<style>";
  pagina += "body{font-family:'Arial',sans-serif;background-color:#f4f7f6;color:#333;text-align:center;padding:10px;margin:0;}";
  pagina += ".container{width:100%;max-width:600px;margin:0 auto;padding:15px;background-color:#ffffff;border-radius:8px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}";
  pagina += "h1{color:#1a73e8;font-size:24px;margin-bottom:5px;}";
  pagina += "hr{border:0;height:1px;background-color:#ddd;margin:10px 0;}";
  pagina += ".data-display{padding:10px 0;margin-bottom:10px;}";
  pagina += ".temperature{font-size:60px;font-weight:bold;color:#d93025;display:block;margin:5px 0;}";
  pagina += ".label{font-size:16px;color:#555;}";
  pagina += "p{margin:3px 0;font-size:14px;}";
  pagina += "small{color:#777;display:block;margin-top:8px;font-size:12px;}";
  pagina += ".card{border-radius:6px;box-shadow:0 2px 8px rgba(0,0,0,0.08);padding:10px;background:#fff;margin-top:15px;text-align:left;border:1px solid #eee;}";
  pagina += ".info-content{font-size:13px;line-height:1.6;color:#444;}";
  pagina += ".info-content ul{list-style-type:none;padding-left:0;}";
  pagina += ".info-content li:before{content:\"•\";color:#1a73e8;font-weight:bold;display:inline-block;width:1em;margin-left:-1em;}";
  pagina += "canvas{background:#fff;display:block;width:100% !important;height:auto !important;}";
  pagina += ".legend{margin-top:8px;font-size:12px;}";
  pagina += ".tooltip{position:absolute;pointer-events:none;background:#222;color:#fff;padding:4px 6px;border-radius:3px;font-size:11px;transform:translate(-50%,-120%);white-space:nowrap;display:none;}";
  pagina += "@media (min-width: 600px) {";
  pagina += "h1 { font-size: 28px; }";
  pagina += ".temperature { font-size: 80px; }";
  pagina += ".label { font-size: 18px; }";
  pagina += ".card { padding: 16px; }";
  pagina += "canvas { width: 700px !important; height: 360px !important; }";
  pagina += ".info-content { font-size: 14px; }";
  pagina += "}";
  pagina += "</style>";
  
  pagina += "</head><body>";
  pagina += "<div class='container'>";
  pagina += "<h1>Termostato Wi-fi</h1><hr>";

  pagina += "<div class='data-display'>";
  if (ultimo_recebido == 0) {
    pagina += "<p class='label' style='color:#f00;'>AGUARDANDO DADOS DO ESP32...</p>";
  } else {
    pagina += "<span class='temperature'>" + String(ultima_temp, 1) + " °C</span>";
    pagina += "<p class='label'>Temperatura Atual</p>";
    pagina += "<p>Pacotes recebidos: " + String(contador) + "</p>";
    
    unsigned long segundos_atras = (millis() - ultimo_recebido) / 1000;
    pagina += "<p>Última atualização: " + String(segundos_atras) + " segundos atrás</p>";
  }
  pagina += "</div>";
  
  pagina += "<small>Atualização automática a cada 5 segundos</small>";

  pagina += "<div class='card'>";
  pagina += "<h3>Sobre o Projeto:</h3>";
  pagina += "<div class='info-content'>";
  pagina += "<p>Este é um termômetro remoto utilizando a comunicação ESP-NOW entre dois microcontroladores ESP (Geralmente um ESP32 como transmissor e um ESP8266 como receptor/servidor web).</p>";
  pagina += "<p>O ESP8266 atua como um ponto de acesso Wi-Fi (`Termometro_UCE2`) e um servidor web que exibe os dados recebidos.</p>";
  pagina += "<p>A comunicação ESP-NOW é rápida e eficiente, ideal para transmissão simples de dados.</p>";
  pagina += "<p>Você pode saber mais sobre o projeto no repositório do github <a href='https://github.com/Gustavo0x1/Termostato-WIFI'>através deste link</a>, lembre-se que você está conectado à rede do ESP866, e o site será inacessível até que retorne à rede wifi habitual! </p>";
  pagina += "</div></div>";
  


pagina += "<div class='card'>"; 
  pagina += "<h3> Integrantes do Projeto</h3>";
  

  pagina += "<ul style='list-style-type:disc; padding-left: 20px; margin: 10px 0; text-align:left;'>"; // list-style-type:disc adiciona bolinhas
  

  pagina += "<li style='font-size:16px; margin-bottom: 8px;'>";
  pagina += "<strong>Gustavo Soares Rodrigues</strong><br>";
  pagina += "<small style='color:#555;'>gsr21@aluno.ifnmg.edu.br</small></li>";

  pagina += "<li style='font-size:16px; margin-bottom: 8px;'>";
  pagina += "<strong>Eduardo Alves de Oliveira</strong><br>";
  pagina += "<small style='color:#555;'>eao7@aluno.ifnmg.edu.br</small></li>";

  pagina += "<li style='font-size:16px; margin-bottom: 8px;'>";
  pagina += "<strong>Gustavo Alves de Oliveira</strong><br>";
  pagina += "<small style='color:#555;'>gao7@aluno.ifnmg.edu.br</small></li>";
  
  pagina += "<li style='font-size:16px; margin-bottom: 8px;'>";
  pagina += "<strong>Milena Soares Silva</strong><br>";
  pagina += "<small style='color:#555;'>mss49@aluno.ifnmg.edu.br</small></li>";
  
  pagina += "</ul>";

  pagina += "<small style='color:#777; display:block; padding-left: 5px;'>Trabalho desenvolvido para a disciplina de UCE 2 (2025).</small>";
  pagina += "</div>";

  pagina += "<div class='card'>";
  pagina += "<h3>Instruções de Uso</h3>";
  pagina += "<div class='info-content'>";
  pagina += "<ul>";
  pagina += "<li>Conecte-se à rede Wi-Fi criada pelo ESP8266:" + String(ssid_ap) + "</li>";
  pagina += "<li>A senha para esta rede é:" + String(password_ap) + "</li>";
  pagina += "<li>Após conectar, acesse o endereço no seu navegador:http://" + WiFi.softAPIP().toString() + "</li>";
  pagina += "<li>Utilizando o configurador, é possível definir o que acontece com as portas D1  e D2 do arduino com base na temperatura medida !</li>";
  pagina += "<li>O site é atualizado automaticamente a cada 5 segundos, <strong>exceto quando está acessando as CONFIGURAÇÕES, quando as atualizações são DESATIVADAS <strong/>.</li>";

  
  pagina += "</ul>";
  pagina += "</div></div>";
  
  pagina += gerarHtmlControle();

  pagina += "<div class='card'>";
  pagina += "<h3>Histórico de Temperatura </h3>";
  pagina += "<canvas id='chart' width='700' height='360'></canvas>";
  pagina += "<div class='legend'>Últimas " + String(HISTORY_SIZE) + " leituras (Atualizado a cada recarregamento)</div>";
  pagina += "</div>";
  
  pagina += "</div>";
  
  pagina += "<div id='tooltip' class='tooltip'></div>";

  pagina += "<script>";
  pagina += "const canvas=document.getElementById('chart');";
  pagina += "const ctx=canvas.getContext('2d');";
  pagina += "const tooltip=document.getElementById('tooltip');";
  
  pagina += "let data=[";
  for (int i = 0; i < HISTORY_SIZE; i++) {
    pagina += String(temp_history[i], 1);
    if (i < HISTORY_SIZE - 1) {
      pagina += ",";
    }
  }
  pagina += "];"; 
  
  pagina += "const margin={top:30,right:20,bottom:40,left:44};";
  
  pagina += "function drawChart(data){";
  pagina += "ctx.clearRect(0,0,canvas.width,canvas.height);";
  pagina += "const W=canvas.width-margin.left-margin.right;";
  pagina += "const H=canvas.height-margin.top-margin.bottom;";
  pagina += "const origin={x:margin.left,y:margin.top+H};";

  pagina += "const max=Math.max(...data)*1.08;";
  pagina += "const min=Math.min(...data)*0.95;";
  pagina += "const xStep=W/Math.max(1,data.length-1);";

  pagina += "ctx.font='12px Arial';ctx.strokeStyle='#eee';";
  pagina += "const rows=5;";
  pagina += "for(let i=0;i<=rows;i++){";
  pagina += "const y=margin.top+(H*i/rows);";
  pagina += "ctx.beginPath();ctx.moveTo(margin.left,y);ctx.lineTo(margin.left+W,y);ctx.stroke();";
  pagina += "const val=(max-(max-min)*i/rows).toFixed(1);";
  pagina += "ctx.fillStyle='#666';ctx.fillText(val,6,y+4);";
  pagina += "}";

  pagina += "ctx.strokeStyle='#333';ctx.lineWidth=1.2;";
  pagina += "ctx.beginPath();ctx.moveTo(origin.x,origin.y);ctx.lineTo(origin.x+W,origin.y);ctx.stroke();";
  pagina += "ctx.beginPath();ctx.moveTo(origin.x,origin.y);ctx.lineTo(origin.x,margin.top);ctx.stroke();";

  pagina += "ctx.lineWidth=2;ctx.strokeStyle='#1f77b4';ctx.beginPath();";
  pagina += "const points=[];";
  pagina += "data.forEach((v,i)=>{";
  pagina += "const x=origin.x+xStep*i;";
  pagina += "const y=margin.top+H*(1-(v-min)/(max-min));";
  pagina += "points.push({x,y,v,i});";
  pagina += "if(i===0)ctx.moveTo(x,y);else ctx.lineTo(x,y);";
  pagina += "});";
  pagina += "ctx.stroke();";
  pagina += "canvas._points=points;";
  pagina += "}";

  pagina += "canvas.addEventListener('mousemove',ev=>{";
  pagina += "const r=canvas.getBoundingClientRect();";
  pagina += "const mx=ev.clientX-r.left,my=ev.clientY-r.top;";
  pagina += "let hit=null;";
  pagina += "for(const p of canvas._points){";
  pagina += "if(Math.hypot(p.x-mx,p.y-my)<8){hit=p;break;}";
  pagina += "}";
  pagina += "if(hit){";
  pagina += "tooltip.style.display='block';";
  pagina += "tooltip.textContent='#'+(hit.i+1)+': '+hit.v;";
  pagina += "tooltip.style.left=(hit.x+r.left)+'px';";
  pagina += "tooltip.style.top=(hit.y+r.top)+'px';";
  pagina += "}else tooltip.style.display='none';";
  pagina += "});";
  pagina += "drawChart(data);";

if (!modo_edicao) {
    pagina += "setInterval(()=>{ location.reload(); },5000);"; 
  } else {
    pagina += "// Auto-refresh desabilitado: Modo de Edição ativo.";
  }
  pagina += "</script>";

  pagina += "</body></html>";
  server.send(200, "text/html", pagina);
  
  Serial.printf("[WEB] Página enviada | Visitas: %d | Temp: %.1f°C\n", contador, ultima_temp);
}

void OnDataRecv(uint8_t *mac, uint8_t *data, uint8_t len) {
  pacote_t recebido;
  memcpy(&recebido, data, sizeof(recebido));

  Serial.println(recebido.temperatura);
  ultima_temp = recebido.temperatura;
  ultimo_recebido = millis();
  contador++;
  verificarTemperatura();
  add_to_history(ultima_temp);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n=== TERMOMETRO WIFI - MODO TEXTO PLANO ===");

  pinMode(PINO_RELE_1, OUTPUT);  
  digitalWrite(PINO_RELE_1, LOW);
  
  pinMode(PINO_RELE_2, OUTPUT);  
  digitalWrite(PINO_RELE_2, LOW); 

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid_ap, password_ap, 1, 0);
  WiFi.channel(1);

  Serial.printf("[HOTSPOT] Criado: %s\n", ssid_ap);
  Serial.printf("[HOTSPOT] Senha: %s\n", password_ap);
  Serial.printf("[HOTSPOT] Acesse: http://%s\n", WiFi.softAPIP().toString().c_str());
  if (esp_now_init() != 0) {
    Serial.println("[ERRO] ESP-NOW FALHOU!");
    while(1);
  }
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("[ESP-NOW] Aguardando pacotes do ESP32...");

  server.on("/", handleRoot);
  server.onNotFound([]() {
    server.send(404, "text/plain", "404 - Não encontrado");
    Serial.println("[WEB] 404 - Página não encontrada");
  });
  server.begin();
  Serial.println("[WEB] Servidor ativo - http://192.168.4.1");
  Serial.println("=== TUDO PRONTO! CONECTE NO WIFI E ABRA O LINK ===\n");
}

void loop() {
  server.handleClient();
  delay(2);
}