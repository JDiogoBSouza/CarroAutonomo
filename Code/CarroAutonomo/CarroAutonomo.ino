// Includes Necessários para o funcionamento das funções especificas
// do servo motor e do sensor UltraSonico.

#include <Servo.h>
#include <Ultrasonic.h>

/***************** Definições do Servo motor ******************/

// Definições de Pinos

#define LDR1        A0    // Pino A0 - Entrada do LDR1.
#define VCCLDR1     A1    // Pino A1 - Alimenta o LDR1.
#define LEDLDR1     A2    // Pino A2 - Alimenta o LED que emite luz para o LDR1

#define FAROIS      2     // Pino 2 - Alimenta os LEDs dos Farois dianteiros.

#define SERVO       6     // Pino 6 - Controla o servo motor.

#define MOTORDIANTEIROA 8   // Pino 8 - Controla a direção do motor dianteiro.
#define MOTORDIANTEIROB 9   // Pino 9 - Controla a direção do motor dianteiro.

#define VELOCIDADEMOTOR 11    // Pino 10 - Controla a velocidade do motor traseiro.
#define MOTORTRASEIROA  10    // Pino 11 - Controla a direção do motor traseiro.
#define MOTORTRASEIROB  12    // Pino 12 - Controla a direção do motor traseiro.

#define ECHO        4   // Pino 4 - Echo do Sensor UltraSonico.
#define TRIGGER     5   //ino 5 - Trigger do Sensor UltraSonico.

// Constantes para controle de posição do servo.
#define ESQUERDA  175   // 175 graus - "Esquerda"
#define CENTRO    95    // 95 graus  - "Centro" 
#define DIREITA   20    // 20 graus  - "Direita"

// Constantes com valores utilizados no programa
#define DISTANCIALDR      530   // Valor da distancia minima de um obstaculo "visto" pelo LDR.
#define DISTANCIAULTRA    60   // Valor da distancia minima de um obstaculo "visto" pelo Ultrasom.

#define VELOCIDADE        200   // Valor da velocidade padrão em que o carro se movimenta.

#define TEMPOSERVO        1000  // Tempo em ms que se deve esperar para o servo deixar o ultrasom na posição correta para leitura.
#define TEMPOCURVA        45  // Tempo em ms que o carro leva para dar uma curva a direita/esquerda.
#define TEMPOBRECAGEM     30   // Valor da duração da "brecagem" do carro.

#define ESQ      -1     //
#define NONE      0     //
#define DIR       1     //

/**************************************************************/

Servo servo;              // Variável do Servo Motor.
Ultrasonic ultrasonic(ECHO, TRIGGER); // Variável do Sensor UltraSonico.

// Variavel para identificação do sentido do carro. 
//  -1 - Trás/Ré
//   0 - Parado
//   1 - Frente
int sentidoAtual = 0;

int tempoAndar = 1000;
bool fazendoCurva = false;
int curvaAtual = 0;

void setup ()
{
  servo.attach(SERVO);  // Inicializa o servo na porta escolhida.

  // LDR1
  pinMode(LDR1, INPUT);   
  pinMode(VCCLDR1, OUTPUT);
  pinMode(LEDLDR1, OUTPUT);

  /*digitalWrite(VCCLDR1, HIGH);
  digitalWrite(LEDLDR1, HIGH);
  digitalWrite(VCCLDR2, HIGH);
  digitalWrite(LEDLDR2, HIGH);*/

  // Motor Traseiro
  pinMode(MOTORTRASEIROA, OUTPUT); 
  pinMode(MOTORTRASEIROB, OUTPUT);
  pinMode(VELOCIDADEMOTOR, OUTPUT);

  // Motor Dianteiro/Direção
  pinMode(MOTORDIANTEIROA, OUTPUT);
  pinMode(MOTORDIANTEIROB, OUTPUT);

  // Farois
  pinMode(FAROIS, OUTPUT);
  
  stop_back(10);
  servo.write(CENTRO);  // Inicia motor posição central.
 
  Serial.begin(9600);
  /*piscaFarois();
  estadoFarol(true);*/
  
  delay(2000);
  randomSeed(analogRead(A6));
  onOffLdr(true);
}

int lado;

void loop()
{  
  
  /*int a = analogRead(LDR1);
  Serial.print("LDR: ");
  Serial.println(a);*/
  anda_frente();
  
  lado = NONE;
  
  while( lado == NONE )
  {
    lado = verificaLados();
    escolheCaminho( lado ); 
  }
}

// Função para fazer o carro andar para frente.
void anda_frente(int Speed, int tempo)
{  
  int i = 0;
  
  analogWrite(VELOCIDADEMOTOR, Speed);
  digitalWrite(MOTORTRASEIROA, LOW);
  digitalWrite(MOTORTRASEIROB, HIGH);

  while( i < tempo/2 )
  {
    if( verificaFrente() )
    {
      if(Speed != 255)
        sentidoAtual = 1;
    
      //Serial.print( "I= " );
      //Serial.println( i );
      i++;
      
      delay(1);
    }
    else if( fazendoCurva )
    {
      centro_direcao();
      breakes(1);
      //anda_tras(VELOCIDADE, 100);

      fazendoCurva = false;

      return;
    }
    else
    {
      break;
    }
  }


  if( i > 0 && Speed != 255)
  {
    Serial.println("BREAKIIIIIIING");
    breakes(1);
  }
  else
  {
    Serial.println("STOPIIIIIIING");
    stop_back(100);
  }
    
}

// Função para fazer o carro andar para frente.
void anda_frente()
{    
  bool andou = false;

  sentidoAtual = 1;
  
  analogWrite(VELOCIDADEMOTOR, VELOCIDADE);
  digitalWrite(MOTORTRASEIROA, LOW);
  digitalWrite(MOTORTRASEIROB, HIGH);
  
  while( verificaFrente() )
  {
    Serial.println("FREEEEEENTE");
    andou = true;
  }
  
  if( andou )
  {
    Serial.println("brake frente");
    breakes(1);
  }
  else
    stop_back(100);
}

// Função para fazer o carro dar ré.
// @param Speed : Velocidade que se deseja que o carro ande.
// @param tempo : Tempo que o carro deve andar.
// @return : Caso o retorno seja true, o Carro conseguiu andar para tras o tempo determinado sem obstaculos,
// caso contrario o carro encontrou algum obstaculo antes ou depois de começar a andar e "brecou".
bool anda_tras(int Speed, int tempo)
{  
  delay(5);   // Para garantir um tempo para ligar os leds e ldrs.
  
  // Liga os LDRs e LEDs para verificar se o carro não vai bater.
  onOffLdr(true);
    
  int a = analogRead(LDR1);
      
  //delay(200);

  if( !(a >= DISTANCIALDR) )
  {
    if(Speed != 255)
      sentidoAtual = -1;
  
    analogWrite(VELOCIDADEMOTOR, VELOCIDADE);
    digitalWrite(MOTORTRASEIROA, HIGH);
    digitalWrite(MOTORTRASEIROB, LOW);
  
    for( int i = 0; i < tempo; i++ )
    {
      a = analogRead(LDR1);
  
      if( a >= DISTANCIALDR )
      {
        Serial.println("Encontrou um obstaculo enquanto dava Re");
        breakes(-1);
        onOffLdr(false);
        return false;
      }
  
      Serial.print("I = ");
      Serial.print(i);
      Serial.print("A = ");
      Serial.print(a);
  
      delay(1);
    }
  }
  else
  {
    Serial.println("Nao conseguiu iniciar a Re");
    onOffLdr(false);
    return false;
  }
  
  // Desliga os LDRs e LEDs.
  breakes(-1);
  onOffLdr(false);

  return true;
}

// Função para fazer o carro frear, enviando um pulso contrario para o motor traseiro.
void breakes( int sentido )
{
  if (sentido == 1) 
  {
    anda_tras(255, TEMPOBRECAGEM);
    stop_back(0.03*TEMPOBRECAGEM);    // 30% do tempo de brecagem
  }
  else if (sentido == -1)
  {
    anda_frente(255, 10);
    stop_back(0.03*10);
  }

  sentidoAtual = 0;
}

void escolheCaminho(int dir)
{
  if( dir == 1 )
  {
    Serial.println( "Curva a direita" );
    direita_direcao();
    fazerCurva( true );
  }
  else if( dir == -1 )
  {
    Serial.println( "Curva a esquerda" );
    esquerda_direcao();
    fazerCurva( false );
  }
  else
  {
    Serial.println( "Dando Re" );
    anda_tras(VELOCIDADE, 50);
  }
}

bool verificaFrente()
{
  float leitura;
  // Suponhamos que o servo ja estará no centro :)
  //servo.write(CENTRO);  // Posiciona o servo na posição inicial/central.
  //delay(TEMPOSERVO);    // Tempo para mover o sensor para a posição de leitura.

  leitura = leituraUltrasom();
  
  //Exibe informacoes no serial monitor
  Serial.print("Centro - Distancia em cm: ");
  Serial.println(leitura);

  if( fazendoCurva == true )
  {
    Serial.println( "FAZENDO CURVA" );
    if( leitura > 30 )
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {  
    
    Serial.println( "INDO RETO" );
    
    if( leitura > DISTANCIAULTRA )
    {
      Serial.println( "leitura > DISTANCIAULTRA" );
      return true;
    }
    else
    {
      Serial.println( "leitura < DISTANCIAULTRA" );
      return false;
    }
  }
}

int verificaLados()
{
  float leituraEsq, leituraDir;
  
  // -------------- Verifica ultra som pela esquerda
  
  servo.write(ESQUERDA);
  delay(TEMPOSERVO);    // Tempo para mover o sensor para a posição de leitura.

  leituraEsq = leituraUltrasom();
  
  //Exibe informacoes no serial monitor
  Serial.print("Esquerda - Distancia em cm: ");
  Serial.println(leituraEsq);

  // -------------- Verifica ultra som pela direita

  servo.write(DIREITA);
  delay(TEMPOSERVO);    // Tempo para mover o sensor para a posição de leitura.
  
  leituraDir = leituraUltrasom();

  Serial.print("Direita - Distancia em cm: ");
  Serial.println(leituraDir);
  Serial.println();

  // -------------------------------------------------------------------
  
  servo.write(CENTRO);  // Posiciona o servo na posição inicial/central

  // -------------- Verifica qual o melhor lado para fazer a curva
  
  if( leituraEsq > DISTANCIAULTRA && leituraDir > DISTANCIAULTRA )
  {
    //int randNumber = random(300);

    //if( randNumber % 2 == 0 )
    //{
      return DIR;
    /*}
    else
    {
      return ESQ;
    }*/
    
  }
  else if( leituraEsq > DISTANCIAULTRA )
  {
    return ESQ;
  }
  else if( leituraDir > DISTANCIAULTRA )
  {
    return DIR;
  }
  else
  {
    return NONE;
  }
  
}

float leituraUltrasom()
{
  float cmMsec;
  long microsec = ultrasonic.timing();
  
  // Le as informacoes do sensor, em cm
  cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);
  
  return cmMsec;
}

// Função para fazer o carro realizar uma curva para direita ou esquerda.
// @param direcao : True - Curva a direita, False - Curva a esquerda
void fazerCurva( bool direcao )
{
  if( direcao )
  {
    Serial.println( "Fazendo curva para a Direita" );
    direita_direcao();
    delay(40);
    curvaAtual = 1;
  }
  else
  {
    Serial.println( "Fazendo curva para a Esquerda" );
    esquerda_direcao();
    delay(40);
    curvaAtual = -1;
  }

  fazendoCurva = true;
  anda_frente(VELOCIDADE, TEMPOCURVA);
  breakes(1);
  
  centro_direcao();
  curvaAtual = 0;
  fazendoCurva = false;
  Serial.println("Acabou a Curva");
  delay(40);
}

void piscaFarois()
{
  digitalWrite(FAROIS, HIGH);
  delay(80);
  digitalWrite(FAROIS, LOW);
  delay(80);
  
  digitalWrite(FAROIS, HIGH);
  delay(80);
  digitalWrite(FAROIS, LOW);
  delay(80);
}

void estadoFarol(bool estado)
{
  if(estado)
    digitalWrite(FAROIS, HIGH);
  else
    digitalWrite(FAROIS, LOW);
  
}

// Função para fazer o carro parar por um determinado tempo.
void stop_back(int tempo)
{ 
    analogWrite(VELOCIDADEMOTOR, 0);
    digitalWrite(MOTORTRASEIROA, LOW);
    digitalWrite(MOTORTRASEIROB, LOW);
    delay(tempo);
  
}


// Função para deixar direcao centralizada.
void centro_direcao()
{
  digitalWrite(MOTORDIANTEIROA, LOW);
  digitalWrite(MOTORDIANTEIROB, LOW);
}

// Função para deixar direcao virada para a direita.
void direita_direcao()
{  
  digitalWrite(MOTORDIANTEIROA, LOW);
  digitalWrite(MOTORDIANTEIROB, HIGH); 
}     

// Função para deixar direcao virada para a esquerda.
void esquerda_direcao()
{  
  digitalWrite(MOTORDIANTEIROA, HIGH);
  digitalWrite(MOTORDIANTEIROB, LOW);
} 

void onOffLdr(bool liga)
{
  if( liga )
  {
    digitalWrite(VCCLDR1, HIGH);  // Liga o LDR1
    digitalWrite(LEDLDR1, HIGH);  // Liga o LED do LDR1
  }
  else
  {
    digitalWrite(VCCLDR1, LOW);  // Liga o LDR1
    digitalWrite(LEDLDR1, LOW);  // Liga o LED do LDR1
  }
}

