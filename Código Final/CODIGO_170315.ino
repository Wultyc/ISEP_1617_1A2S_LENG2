//Bibliotecas
#include <PID_v1.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <LiquidCrystal.h>

//Comandos
#define ON 1      //Liga
#define OFF 0     //Desliga
#define PLUS 255  //Aumenta
#define LESS 0    //Diminui

// Pinos de entrada/saida da informação
#define TEMP_SENSOR_PIN 12  //Sensor de Temperatura
#define HUMD_SENSOR_PIN 13  //Sensor de Humidade
#define DHTTYPE DHT22       //Modelo do Sensor
#define BOMBA_AGIT_PIN 8    //Relé Bomba de Extração
#define RESISTENCIA_PIN 9   //Relé resistencia
#define VENTILADOR_PIN 11  //MD Ventilador
#define LOGIN_PIN A0  //MD Ventilador

//Valores pretendidos
#define HUMIDADE 80 //Valor relativo da humidade
#define HUMD_INC 2  //Valor da incerteza da humidade
#define TEMPERATURA 25 //Valor da temperatura
#define TEMP_INC 0.3   //Valor da incerteza da temperatura

//Variaveis de Controlo
int controlo = 0;
int contHumd, contTemp;

//Variaveis de informação
float h_humd, h_temp;//Leituras do sensor de humidade
float temp; //leitura do sensor de temperatura

//Variaveis PID
double  pidInput, pidOutput, Setpoint = HUMIDADE;
float KP = 20.25;//100
float KI = 7.175;//7.175
float KD = 0;//0

//Contagem das leituras
int resNum = 0;
double actTime = 0, lastTime = -120;

OneWire oneWire(TEMP_SENSOR_PIN); //Incialização do sesnsor de tempartura
DallasTemperature sensors(&oneWire);//Passa a referencia do oneWire para o Dallas
DHT dht(HUMD_SENSOR_PIN, DHTTYPE); //Incializa o sensor de Humidade
LiquidCrystal lcd(7, 6, 5, 4, 3, 2); //Iniclialização do LCD
PID epPID(&pidInput, &pidOutput, &Setpoint, KP, KI, KD, REVERSE); //Inicialização PID

void setup(void){
  //Preparação dos sensores
  sensors.begin(); //temperatura
  dht.begin();     //humidade
  
  //Inicia o PID
  epPID.SetMode(AUTOMATIC);
  //epPID.SetOutputLimits(25, 255);
  lcd.begin(16, 2); //LCD

  //Modo dos Pinos
  pinMode(LOGIN_PIN, INPUT);
  pinMode(TEMP_SENSOR_PIN, INPUT);
  pinMode(HUMD_SENSOR_PIN, INPUT_PULLUP); 
  pinMode(RESISTENCIA_PIN, OUTPUT);
  pinMode(BOMBA_AGIT_PIN, OUTPUT);
  pinMode(VENTILADOR_PIN, OUTPUT);

  //Imprime a mesma mensagem no LCD
  lcd.setCursor(0, 0);
  lcd.print("Pool Control +");
  lcd.setCursor(0, 1);
  lcd.print("Everithing + 3.0");
  delay(2000);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Andre Oliveira");
  lcd.setCursor(0, 1);
  lcd.print("Barbara Santos");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gabriel Azevedo");
  lcd.setCursor(0, 1);
  lcd.print("M Ines Almeida");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tiago Moura");
  lcd.setCursor(0, 1);
  lcd.print("We Are Good ....");
  delay(2000);

  //faz login
  lcd.clear();
  while(digitalRead(LOGIN_PIN) == LOW){
    lcd.setCursor(2, 0);
    lcd.print("Introduza a");
    lcd.setCursor(0, 1);
    lcd.print("chave p/ iniciar");
  }
  //Comunicação Serial
  Serial.begin(9600); //Incia a cominuicação serial
  Serial.println("(C) Everithing Plus 3.0"); //Mensagem de Incialização
  Serial.println("Pool Control Plus");
  Serial.println("Produzido por: \n\t\tAndre Oliveira (1161260),\n\t\tBarbara Santos (1161033),\n\t\tJorge Gabriel Azevedo (1160929),\n\t\tM Ines Almeida (1160631),\n\t\tTiago Moura (1160715)");
  Serial.println("ISEP - Licenciatura em Engenharia de Sistemas (2016/17)");
  
  Serial.println("\n#\tt(s)\tT(*C)\tH(%)\tPID\tObs");
  //Liga a bomba de agitação
  digitalWrite(BOMBA_AGIT_PIN, HIGH);
}

void loop(void){ 

  actTime = millis()/1000;
  
  //Recebe informações dos parametros desajustados
  controlo = temperatura() + humidade();

  //Output da informação
  if((actTime - lastTime >= 1) && controlo != 1000 && controlo != 2000){ //25seg + tempo de processamento ~ 30seg e não há erros!
    resNum++;
    Serial.print(resNum);
    Serial.print("\t");
    Serial.print(actTime);
    Serial.print("\t");
    Serial.print(temp);
    Serial.print("\t");
    Serial.print(h_humd);
    Serial.print("\t");
    Serial.println(pidOutput);

    lastTime = actTime;
  }

  

  //Segue o caminho mais adquado
  switch(controlo){
    case 0:
      showInfoLCD(1);
      break;
    case 1:
      aquecimento(ON); //Temperatura Baixa
      showInfoLCD(1);
      break;
    case 2:
      aquecimento(OFF); //Temperatura Alta
      showInfoLCD(1);
      break;
    case 4:
      //Humidade Baixa
      ventilador();
      showInfoLCD(1);
      break;
    case 5:
      //Temperatura Baixa e Humidade Baixa
      aquecimento(ON);
      ventilador();
      showInfoLCD(1);
      break;
    case 6:
      //Temperatura Alta e Humidade Baixa
      aquecimento(OFF);
      ventilador();
      showInfoLCD(1);
      break;
    case 8:
      //Humidade Alta
      ventilador();
      showInfoLCD(1);
      break;
    case 9:
      //Temperatura Baixa e Humidade Alta
      aquecimento(ON);
      ventilador();
      showInfoLCD(1);
      break;
    case 10:
      //Temperatura Alta e Humidade Alta
      aquecimento(OFF);
      ventilador();
      showInfoLCD(1);
      break;
    default : //Valor com significado impossivel
      Serial.println("\t\t\t\t\tERRO: Leituras sem significado! Opa temos algum erro mas o que importa e' ter saude");
      showInfoLCD(0);
      
      break;
  }
  delay(1500);
}

int humidade(){ //Analiza a informação do sensor humidade
  h_humd = dht.readHumidity(); //Leitura da humidade do sensor de humidade
  h_temp = dht.readTemperature(); //Leitura da temperatura do sensor de humidade

  //Se acontecer algum erro com o sensor de humidade faz restart
  if (isnan(h_humd) || isnan(h_temp)) {
    return 1000;
  }

  //Analize dos valores
  if(h_humd < (HUMIDADE - HUMD_INC)){
    //Humidade baixa
    contHumd = 4;
  }else if (h_humd > (HUMIDADE + HUMD_INC)){
    //Humidade alta
    contHumd = 8;
  }else{
    //Humidade ok
    contHumd = 0;
  }

  return contHumd;
}

int temperatura(){ //Analiza a informação do sensor temperatura
    
  //Envia o request de temperatura
  sensors.requestTemperatures();  
  temp = sensors.getTempCByIndex(0);
  //Saida do controlo 

  //Analize dos valores
  if(temp < (TEMPERATURA - TEMP_INC)){
    //Temperatura baixa
    contTemp = 1;
  }else if (temp > (TEMPERATURA + TEMP_INC)){
    //Temperatura alta
    contTemp = 2;
  }else if (temp == -127){
    //Erro
    return 1000;
  }else{
    //Temperatura ok
    contTemp = 0;
  }
  
  //Temperatura ok
  return contTemp;
}

void ventilador(){
  //analogWrite(VENTILADOR_PIN, vel);
  pidInput = h_humd;
  epPID.Compute();
  analogWrite(VENTILADOR_PIN,pidOutput);
  //analogWrite(VENTILADOR_PIN,255);
}

void aquecimento(int pwr){
  digitalWrite(RESISTENCIA_PIN, pwr);
}

void showInfoLCD(int printFlag){
  if(printFlag == 1){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temp);
    lcd.print(" *C");
  
    lcd.setCursor(0, 1);
    lcd.print("Humd: ");
    lcd.print(h_humd);
    lcd.print(" %");
  
    delay(2000);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PID: ");
    lcd.print(pidOutput);
  
    lcd.setCursor(0, 1);
    lcd.print("Tempo: ");
    lcd.print((int)actTime);
    lcd.print(" seg");
  }else{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Erro Sensores");
    lcd.setCursor(0, 1);
    lcd.print("Tempo: ");
    lcd.print((int)actTime);
    lcd.print(" seg");
  }
}

