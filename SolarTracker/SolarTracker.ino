/*
Nome>:
Autor:
Descrição:

*/
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>



//Definição dos pinos dos LDRs
#define ldrSupEsquerda D8
#define ldrSupDireita D7
#define ldrInfEsquerda D6
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
int escala = 200; //escala de luminosidade dos sensores LDR
int luminosidade_minima = 20; //valor para que o sistema vá para a posição de espera
int sensibilidade = 5; // esse número define a diferenca entre duas medidas para que seja feito algum movimento
int angula_max_horizontal = 170; //Angulo máximo no eixo horizontal
int angula_mim_horizontal = 10; //Angulo mínimo no eixo horizontal
int angula_max_vertical = 130; //Angulo máximo no eixo vertical
int angula_mim_vertical = 10; //Angulo mínimo no eixo vertical
int tempo_delay = 3; //delay entre as atualizacoes de angulo
int passos = 5; //passos em cada atualização de angulo
bool aplicar_fator_correcao = false; // aplicar fator de correção nos valores dos LRDs
int divisor = 1; //1 para somar os valores de cada sentido de rotação ou dois para usar a média dos valores

//String ssid = "DEIILOR MESH";
//String password = "lei0204lei";
String ssid = "DEIILOR ZEN";
String password = "12345678";


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

WiFiClient client;

void setup() {
  pinMode(ldrSupEsquerda, INPUT);
  pinMode(ldrSupDireita, INPUT);
  pinMode(ldrInfEsquerda, INPUT);
  pinMode(ldrInfDireita, INPUT);    
  pinMode(pin_servo_vertical, INPUT); 
  pinMode(pin_servo_horizontal, INPUT); 
  Serial.begin(115200);
  delay(3000);
  
  servo_vertical.attach(pin_servo_vertical);  // atribui o porta ao servo
  servo_horizontal.attach(pin_servo_horizontal);  // atribui o porta ao servo

  // ###############  Conexão na rede WiFi   ###############
  conect_wifi();
  // ###############  Conexão na rede WiFi   ###############

}

void loop() {

  delay(50);
 // atualiza_ldr();
//  movimento_vertical(movimentar());
//  movimento_horizontal(movimentar());
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
  vlrLdrSupEsquerda = map(multiplex(ldrSupEsquerda), 0,1023,0,escala) * fatorLdrSupEsquerda;
  vlrLdrSupDireita =  map(multiplex(ldrSupDireita), 0,1023,0,escala) * fatorLdrSupDireita;
  vlrLdrInfEsquerda = map(multiplex(ldrInfEsquerda), 0,1023,0,escala) * fatorLdrInfEsquerda;
  vlrLdrInfDireita =  map(multiplex(ldrInfDireita), 0,1023,0,escala) * fatorLdrInfDireita;

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

void conect_wifi(){
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
