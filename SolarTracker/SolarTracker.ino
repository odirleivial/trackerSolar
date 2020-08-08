/*
Nome>:
Autor:
Descrição:
*/
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

//Definição dos pinos dos LDRs
#define ldrSupEsquerda D6
#define ldrSupDireita D7
#define ldrInfEsquerda D8
#define ldrInfDireita D5
#define pin_analog A0

#define pin_servo_vertical D0
#define pin_servo_horizontal D3

Servo servo_vertical; 
Servo servo_horizontal; 

/*#### Declaração das funcoes ####*/
bool movimentar(); // funcao para decidir se haverá uma atualizalçai das posiçoes
void movimento_vertical();
void movimento_horizontal();
void posicao_espera();
String to_str(int);

/*#### Declaração das funcoes ####*/
 
/*#### Configurações ####*/
int escala = 3000; //escala de luminosidade dos sensores LDR
int luminosidade_minima = 50; //valor para que o sistema vá para a posição de espera
int sensibilidade = 10; // esse número define a diferenca entre duas medidas para que seja feito algum movimento
int divisor = 2; //1 para somar os valores de cada sentido de rotação ou dois para usar a média dos valores
int range_ini = 0;
int tangr_fin = 1024;
int angula_max_horizontal = 170; //Angulo máximo no eixo horizontal
int angula_mim_horizontal = 10; //Angulo mínimo no eixo horizontal
int angula_max_vertical = 130; //Angulo máximo no eixo vertical
int angula_mim_vertical = 10; //Angulo mínimo no eixo vertical
int tempo_delay = 2; //delay entre as atualizacoes de angulo
int passos = 1; //passos em cada atualização de angulo
bool aplicar_fator_correcao = false; // aplicar fator de correção nos valores dos LRDs
String ssid = "DEIILOR MESH";
String password = "lei0204lei";
//String ssid = "DEIILOR ZEN";
//String password = "12345678";
IPAddress server_addr(85, 10, 205, 173);  // IP dobanco de dados -- 85.10.205.173:3306
int porta = 3306; // Porta do banco de dados
char db_user[] = "deiilor_solar";               // Usuário MySQL
char db_password[] = "RGY6Pt6weimkuJa5iw85";    // Senha MySQL
char db_schema[] = "solar_analytics";           // Nome da base de dados

/*#### Configurações ####*/

//Variaveis para armazenar os valores dos sensores LDR
int vlrLdrSupEsquerda;
int vlrLdrSupDireita;
int vlrLdrInfEsquerda;
int vlrLdrInfDireita;
int vlrLdrSuperior;
int vlrLdrInferior;
int vlrLdrDireita;
int vlrLdrEsquerda;

//Variaveis do fator de correção para calibração dos LDRs
double fatorLdrSupEsquerda = 1.0;
double fatorLdrSupDireita = 1.0;
double fatorLdrInfEsquerda = 1.0;
double fatorLdrInfDireita = 1.0;

//Criação dos objetos do banco de dados
WiFiClient client;
MySQL_Connection conn((Client *)&client);
MySQL_Cursor cur = MySQL_Cursor(&conn);
char query[2000];

// Sensor de corrente
Adafruit_INA219 Sensor_A(0x44);
Adafruit_INA219 Sensor_B(0x40);
float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float loadvoltage = 0;
float power_mW = 0;

int aux = 0;

void setup() {
    Serial.begin(115200);

    pinMode(ldrSupEsquerda, OUTPUT);
    pinMode(ldrSupDireita, OUTPUT);
    pinMode(ldrInfEsquerda, OUTPUT);
    pinMode(ldrInfDireita, OUTPUT);   
    pinMode(pin_analog, INPUT); 

    pinMode(pin_servo_vertical, INPUT); 
    pinMode(pin_servo_horizontal, INPUT); 
    servo_vertical.attach(pin_servo_vertical);  // atribui o porta ao servo
    servo_horizontal.attach(pin_servo_horizontal);  // atribui o porta ao servo

    delay(1000);



   // conecta_wifi(); // Conexão na rede WiFi 
   // conecta_mysql(); // Conexão no servidor MySQL

    //   inicia_sensor_corrente();

}

void loop() {

  timer(0);
  delay(10);
//    atualiza_ldr();
  movimento_vertical(movimentar());
  movimento_horizontal(movimentar());

//  if (aux == 0){
//    insertMedicao(1, 3.3, "teste") ;
//    aux = 1;
//  }
// monitor_corrente_tensao();


// Serial.print("Corrente sensor A: ");
// Serial.print(String(monitor_corrente(Sensor_A)));
// Serial.println(" - media - " + String(random(1024,4096)));

//  Serial.print("Corrente sensor B: ");
//  Serial.print(String(monitor_corrente(Sensor_B)));
//  Serial.println(" - media - " + String(random(1024,4096)));
}

bool movimentar(){
  bool mover;
  
  atualiza_ldr();
  int diferenca = abs(vlrLdrSuperior - vlrLdrInferior);
  
  if (luminosidade_minima > (vlrLdrSuperior + vlrLdrInferior )){
    posicao_espera();
    delay(10000);
    mover = false;
  } else if (diferenca >= sensibilidade){
    mover = true;
  }else{
    mover = false;
  }

  return mover;
}

void movimento_vertical(bool mover){

  int angulo = servo_vertical.read();
   
  if((mover) && (vlrLdrSuperior > vlrLdrInferior )){
    //Move para cima
    angulo +=passos;
    servo_vertical.write(constrain(angulo, angula_mim_vertical, angula_max_vertical));
    delay(tempo_delay);
  }else if((mover) && (vlrLdrInferior > vlrLdrSuperior)){
    //Move para baixo
    angulo -=passos;
    servo_vertical.write(constrain(angulo, angula_mim_vertical, angula_max_vertical));
    delay(tempo_delay);
  }else{
    //Fica parado
    delay(tempo_delay);
  }
 
}

void movimento_horizontal(bool mover){

  int angulo = servo_horizontal.read();

  if((mover) && (vlrLdrDireita > vlrLdrEsquerda )){
    //Move para direita
    angulo +=passos;
    servo_horizontal.write(constrain(angulo, angula_mim_horizontal, angula_max_horizontal));
    delay(tempo_delay);    
  }else if((mover) && (vlrLdrEsquerda > vlrLdrDireita)){
    //Move para esquerda
    angulo -=passos;
    servo_horizontal.write(constrain(angulo, angula_mim_horizontal, angula_max_horizontal));
    delay(tempo_delay);    
  }else{
    //Fica parado
    delay(tempo_delay);
  }
} 

void posicao_espera(){
  servo_vertical.write(20);
  servo_horizontal.write(90);
  delay(100);
}

void atualiza_ldr(){

    for (int i=0; i < 10; i++){
        vlrLdrSupEsquerda = map(multiplex(ldrSupEsquerda), range_ini,tangr_fin,0,escala) * fatorLdrSupEsquerda;
        vlrLdrSupDireita =  map(multiplex(ldrSupDireita),  range_ini,tangr_fin,0,escala) * fatorLdrSupDireita;
        vlrLdrInfEsquerda = map(multiplex(ldrInfEsquerda), range_ini,tangr_fin,0,escala) * fatorLdrInfEsquerda;
        vlrLdrInfDireita =  map(multiplex(ldrInfDireita),  range_ini,tangr_fin,0,escala) * fatorLdrInfDireita;
    }

    vlrLdrSupEsquerda = vlrLdrSupEsquerda / 10;
    vlrLdrSupDireita = vlrLdrSupDireita / 10;
    vlrLdrInfEsquerda = vlrLdrInfEsquerda / 10;
    vlrLdrInfDireita = vlrLdrInfDireita / 10;

    vlrLdrSuperior = (vlrLdrSupEsquerda + vlrLdrSupDireita) / divisor;
    vlrLdrInferior = (vlrLdrInfEsquerda + vlrLdrInfDireita) / divisor;
    vlrLdrDireita  = (vlrLdrSupDireita + vlrLdrInfDireita) / divisor;
    vlrLdrEsquerda = (vlrLdrSupEsquerda + vlrLdrInfEsquerda ) / divisor;

    Serial.println();
    Serial.println(to_str(vlrLdrSupEsquerda) + " | " + to_str(vlrLdrSupDireita));
    Serial.println("--- ---");
    Serial.println(to_str(vlrLdrInfEsquerda) + " | " + to_str(vlrLdrInfDireita));
    Serial.println();

    Serial.println("\n\nValor vlrLdrSuperior: " + String(vlrLdrSuperior));
    Serial.println("Valor vlrLdrInferior : " + String(vlrLdrInferior));
    Serial.println("Valor vlrLdrDireita: " + String(vlrLdrDireita));
    Serial.println("Valor vlrLdrEsquerda : " + String(vlrLdrEsquerda));
}

String to_str(int vlr){
  if (vlr < 10){
    return "0" + String(vlr);
  }else{
    return String(vlr);
  }
}

int multiplex(int pin){
 
  digitalWrite(pin, HIGH);
  analogRead(pin_analog);
  int valor = analogRead(pin_analog);
  digitalWrite(pin, LOW);

  return valor;
  
}

void conecta_wifi(){
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.print("\n\n\nConectando a ");
  Serial.println(ssid);  
  if (!WL_CONNECTED){
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
  }

  //Obtem endereço IP
  String ip_add = WiFi.localIP().toString().c_str();
  while (ip_add == "(IP unset)") {
    delay(500);
    ip_add = WiFi.localIP().toString().c_str();
    Serial.print(".");
  }

  // Mostra o endereco IP
  Serial.println("");
  Serial.println("WiFi conectado!");
  Serial.print("IP Local: ");
  Serial.println(WiFi.localIP());  
}

void conecta_mysql(){
  // ###############  CONECTA NO MySQL   ###############
  Serial.println("\n\n\nCONECTANDO AO MySQL\n\n\n");
  
  while (!conn.connect(server_addr, porta, db_user, db_password)) {
    Serial.println("Conexão SQL falhou.");
    conn.close();
    delay(1000);
    Serial.println("Conectando SQL novamente.");
  }
  Serial.println("Conectado ao servidor SQL.");
  // ###############  CONECTA NO MySQL   ###############
}

void insertMedicao(int id_placa, double valor, String tipo) {
  
  String query_medicao = "INSERT INTO solar_analytics.tb_medicao (id_placa, valor, tipo, dt_medicao) VALUES (" + String(id_placa) + ", " + valor + " , '" + tipo + "', now());";
  
  Serial.println(query_medicao);
  query_medicao.toCharArray(query, 2000);

  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  // Execute the query
  if( cur_mem->execute(query)){
    Serial.println("Registro inserido com sucesso!");
  } else{
    Serial.println("Erro Fatal!");
  }
  delete cur_mem;
}

double monitor_corrente(Adafruit_INA219 ina219){
  float aux = 0;
  int count = 0;
  current_mA = 0;
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  float _current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  for (int i=0; i < 10; i++ ){
    count++;
    aux = ina219.getCurrent_mA();
   // Serial.println("AUX:   " + String(aux));
    if (aux > 0){
      current_mA += aux;
    }else{
      i--;
    }
    
    if (count == 20){
      return 0.0;
    }
  }


    
//  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
//  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
//  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
//  Serial.print("Current:       "); Serial.print(_current_mA); Serial.println(" mA");
//  Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
//
//  delay(50);
  return current_mA = current_mA/10;
  
}

void inicia_sensor_corrente(){
//       while (! Sensor_A.begin()) {
//     Serial.println("Failed to find INA219 chip A");
//         delay(50);
//   }
//   delay(100);  
    
    while (! Sensor_B.begin()) {
    Serial.println("Failed to find INA219 chip B");
    delay(50);
  }
}

void timer(int segundos){
    delay(segundos*1000);
}
