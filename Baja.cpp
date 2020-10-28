/*UPDATES E DETALHES DO CÓDIGO
 * Erros para corrigir...
 * ... ICONE DE ATENCÃO BRANCO E O PRETO SÃO IGUAIS
 * ... FALTA UM DOS ICONES DO TERMOMETRO
 * ... ARRUMAR O LOGO DA CAPIBAJA
 * ... ADICIONAR LEITURA DE LUZ DE FREIO
 * ... ARRUMAR LEITURA DE TESAO NO ARDUINO DA WAVGAT
 * ... THRESHOLD PARA O FREIO NAO BUGAR O ICONE DO DISPLAY
 * ... CALIBRAR COM O CARRO O VELOCIMETRO E O ODÔMETRO
 *
 * 
 * Atualizado 22/fev/2019
 *    Foi feito o blink dos icones com o "tempo correto" sem o uso de delay, mas com a funcao millis
 *    Foi feito a tela para exibir diversas informacoes do carro Informacoes();
 * Atualizado 23/fev/2019
 *    Foi feito o ponteio que aponta para um determinado icone que tem que mudar o seu estado para fazer o blink
 *  para caso de emergencia do carro avisar o piloto
 * Atualizado 24/fev/2019
 *    Foi implementado o velocimetro, com um sensor hall, (de verdade, e não um potenciometro para apenas testes)
 *    Usa funções de interrupção para medir a velocidade, com mais precisão.
 * Atualizado 23/abr/2019
 *    Foi alterado sistema do tacometro e odometro, deixando a circunferencia da roda como constante, e o tempo
 *    como uma variável, tornando mais precisa as medias, porem ainda não testado
 * Atualizado 1/nov/2019
 *    Me perdi nas atualizações, velocimetro com 8 imas na roda, colocados (Norte,Sul, ... ,Norte,Sul), exibe 
 *    velocidade a cada 1 segundo
*/
#include <Arduino.h>
#include <EEPROM.h>
#include <math.h>
#include <U8glib.h>
#include "icones.h"

#define sensor 2 //sensor hall na roda da frente
#define sensor_tensao A1 //sensor de tensão da bateria

typedef void (funcao)(void);  //MAS O QUE É ISSO NO MEU CÓDIGO

U8GLIB_ST7920_128X64_1X u8g(6, 5, 4 ,7); //Enable, RW, RS, RESET

volatile unsigned long contador;


float odometro = 0, //limite da quilometragem é de  32767Km pq estou com preguiça de colocar long int
      odometro_E2PROM = 0,
      velocidade = 0,  //Velocidade real lida pelo sensor 
      constante_velocimetro = 0.2194, // diametro da roda = 55,88cm, circunferencia = 175,55cm, numero de ímãs = 8
                                      // constante velocimetro = 21,944 cm  
      constante_odometro = constante_velocimetro / 1000.0;

double tensao = 0;

bool estado_bateria; 

int i = 0,
    rpm = 0,
    erros = 0,
    velocidade_display = 0, //Velocidade que será impresso no display
    odometro_display = 0,
    valor_sensor_tensao = 0,
    sensor_temperatura = 0;
   
unsigned long tempo = 0;      // usado para sincronia

char buf[9],
     str_odometro[15],
     str_bateria[30],
     str_temperatura[15];
    

// Cria um ponteiro para apontar um vetor de 32 uint8_t
const uint8_t  *icone_termometro_branco [32]  = {termometro_branco},
               *icone_distancia_branco [32] = {distancia_branco},
               *icone_distancia_preto [32] = {distancia_preto},
               *icone_temperatura_branco [32] = {temperatura_branco},
               *icone_temperatura_preto [32] = {temperatura_preto},
               *icone_cronometro_preto [32] = {cronometro_preto},
               *icone_cronometro_branco [32] = {cronometro_branco},
               *icone_bateria_preto [32] = {bateria_preto},
               *icone_bateria_branco [32] = {bateria_branco},
               *icone_atencao_branco [32] = {atencao_branco},
               *icone_atencao_preto [32] = {atencao_preto},
               *icone_temp [32] = {temp},
               *icone_x [32] = {bateria_preto},
               *icone_y [32] = {distancia_branco},
               *icone_z [32] = {termometro_branco},
               *icone_w [32] = {temperatura_branco}; 

//Colocar para imprimir uma tela específica
void imprime(funcao *tela){ //passa uma função como parametro
    u8g.firstPage();
    do{
    tela();}
    while(u8g.nextPage());
};
// DrawBitmapP(x,y,bits do hexa, colunas, o que vai imprimir);

void informacoes(){  // status do carro, leitura de todos os sensores
  u8g.setFont(u8g_font_courB10r); // Escolhe a fontes
  sprintf(str_temperatura, "Temp_CVT: %dC", sensor_temperatura);
  u8g.drawStr( 0, 12,str_temperatura); // posiciona e imprime
  u8g.setPrintPos(0, 24);
  u8g.print("Bateria: "); u8g.print(tensao); u8g.print("V");
};

void capibaja(){ //Logo Capibaja 
  u8g.drawBitmapP( 0, 0, 16, 128, capibaja_logo);
};

void escreve_velo(){ // escreve a velocidade do carro
  velocidade_display = round(velocidade); //Arredonda a velocidade para um INT para poder ser impressa

  int x = 10, y = 46;
  sprintf(buf, "%d", velocidade_display); // vai saber, mas tem que ter para funcionar
  u8g.setFont(u8g_font_fur42n);   // Fonte Grande para velocidade
  u8g.setFontPosBaseline();

  if(velocidade_display <= 9){            // Essa Gambiarra é para fazer o scroll da direita para a esquerda
  u8g.drawStr(x+27,y, buf);}

  else{
  u8g.drawStr(x,y, buf);}       // Escreve valor da velocidade

  u8g.setFont(u8g_font_courB14r); // Fonte Pequena para velocidade
  u8g.setFontPosBaseline();
  u8g.drawStr(70,46," Km/h");     // Desenha texto
};

void escreve_odometro(){// Escreve o valor da kilometragem rodado pelo carro
  u8g.setPrintPos(12, 64);
  odometro_display = round(odometro);
  u8g.print(odometro_display);
  u8g.drawStr( 70, 64," Km");
}

void escreve_icones(){
  u8g.drawBitmapP(  81,  0, 2, 16, *icone_bateria_preto);
  u8g.drawBitmapP(  81, 16, 2, 16, *icone_termometro_branco);
  u8g.drawBitmapP( 103,  0, 2, 16, *icone_atencao_branco);
  u8g.drawBitmapP( 103, 16, 2, 16, *icone_bateria_preto);
};

float mede_tensao_bateria(){
  valor_sensor_tensao = analogRead(sensor_tensao);
  tensao = (valor_sensor_tensao * 15.0) / 3968.0;
  return tensao;
};

int verifica(){
  //"0" para caso esteja tudo certo
  //"2" para caso a bateria estiver descarregada, menos de 11,5V, 13,8 - 10v
  //"4" para caso CVT quente acima de XXºC
  //"8" para caso Freio precionado

  erros = 0;

  if(mede_tensao_bateria() <= 11.0){ //igual a 11,05V
    estado_bateria = true;}
  if(mede_tensao_bateria() > 12.6 //igual a 12,0V
  ){
    estado_bateria = false;}
  if(estado_bateria == true){
    erros = erros + 2;}

/*
  if(temperatura() <= 11.5){  
  erros = erros + 4;}

  if(freio() <= 11.5){  
  erros = erros + 8;}
*/
  
  return erros;

}

void telaPrincipal(){ // Mostra tela principal
  escreve_velo();
  escreve_odometro();
};

void telaPrincipal_alerta_A(){ // Mostra tela principal
  escreve_velo();
  escreve_odometro();
};

void telaPrincipal_alerta_B(){  // Mostra tela principal + Avisos e alertas
  escreve_velo();
  escreve_odometro();

  if(verifica() == 2){  //Baixa tensão na bateria
  u8g.drawBitmapP(  81,  0, 2, 16, *icone_bateria_preto);}

  if(verifica() == 4 ){ //CVT quente
  u8g.drawBitmapP(  81, 16, 2, 16, *icone_termometro_branco);}

  if(verifica() == 6){  //Baixa tensão na bateria e CVT quente
  u8g.drawBitmapP(  81,  0, 2, 16, *icone_bateria_preto);
  u8g.drawBitmapP(  81, 16, 2, 16, *icone_termometro_branco);}
  
  if(verifica() == 8){  //Freio
  u8g.drawBitmapP( 103,  0, 2, 16, *icone_atencao_branco);}

  if(verifica() == 10){  //Freio e Baixa tensão na bateria
  u8g.drawBitmapP(  81,  0, 2, 16, *icone_bateria_preto);
  u8g.drawBitmapP( 103,  0, 2, 16, *icone_atencao_branco);}

  if(verifica() == 12){  //Freio e CVT quente
  u8g.drawBitmapP(  81, 16, 2, 16, *icone_termometro_branco);
  u8g.drawBitmapP( 103,  0, 2, 16, *icone_atencao_branco);}

  if(verifica() == 14){  //Freio, Baixa tensão na bateria e CVT quente
  u8g.drawBitmapP( 103,  0, 2, 16, *icone_atencao_branco);
  u8g.drawBitmapP(  81,  0, 2, 16, *icone_bateria_preto);
  u8g.drawBitmapP(  81, 16, 2, 16, *icone_termometro_branco);}
};

void contagem(){
  contador++;
}

void setup(){
  tensao =  mede_tensao_bateria();
  EEPROM.get(0, odometro);
  u8g.setColorIndex(1);     // Liga pixels
  Serial.begin(9600);       // Inicia a comunicação serial
  attachInterrupt(digitalPinToInterrupt(sensor), contagem, CHANGE); // inicia o velocimetro e odometro
  imprime(capibaja);        // logo do capibaja
  delay(1000);
  
}

void loop(){

if((millis() - tempo) >= 1000){  // calculo da velocidade circunferencia = 168cm
  detachInterrupt(digitalPinToInterrupt(sensor));
  odometro = odometro + (contador * constante_odometro);
  velocidade  = contador * constante_velocimetro * 3.6;
  contador = 0;
  tempo = millis();}
  Serial.println(mede_tensao_bateria());
  if((odometro - odometro_E2PROM) >= 1){
    odometro_E2PROM = odometro;
    EEPROM.put(0, odometro);
  }

  attachInterrupt(digitalPinToInterrupt(sensor), contagem, CHANGE);
  Serial.println(mede_tensao_bateria());
  Serial.println(analogRead(sensor_tensao));

  if(verifica() == 0){   //se for igual a 1 mostra a telaPrincipal
    imprime(telaPrincipal);}
  else if(verifica() == 55){  //se for igual a 55 mostra a tela de informacoes
    imprime(informacoes);}

  else{                //Se acontecer algum problema entra nessa parte do loop
    if((millis() - tempo) >= 500){
    imprime(telaPrincipal_alerta_A);
    }
    if((millis() - tempo) < 500){
    imprime(telaPrincipal_alerta_B);
    }
  }
}

// TÁRTAGO DAS FUNÇÕES DO CÓDIGO

/*
void alerta_teste(){
  char alerta;
  if(Serial.available() != 0){
    alerta = Serial.read();
  }
  if(alerta == 'B'){
   *icone_x = termometro_branco;                //Icone_x aponta para a distancia Branco
  }
  else{
   *icone_x = bateria_branco;
  }
};

void curiosidadeCapivaras(){
  u8g.setFont(u8g_font_5x8);
  u8g.drawStr( 0, 8, "        A CAPIVARA        ");
  u8g.drawStr( 0, 16, "É o maior roedor do mundo");
  u8g.drawStr( 0, 24, "    pesando até 91 kg    ");
  u8g.drawStr( 0, 32, "    medindo até 1,2 m    ");
  u8g.drawStr( 0, 40, "A CAPIVARA EXPIROU O BAJA");
  u8g.drawStr( 0, 48, "  O BAJA SÓ TEM CAPIVARA ");
  u8g.drawStr( 0, 56, "   CAPIVARAS SÃO DAORA   ");};  


void curiosidadeCapivaras(){

  u8g.setFont(u8g_font_5x8);
  u8g.drawStr( 0, 8, "        A CAPIVARA        ");
  u8g.drawStr( 0, 16, " O MAIOR ROEDOR DO MUNDO ");
  u8g.drawStr( 0, 24, "OS BAJEIROS SÃO CAPIVARAS");
  u8g.drawStr( 0, 32, " BAJEIROS VIVEM NO BARRO ");
  u8g.drawStr( 0, 40, " BAJA INSPIRA AS CAPIVARA");
  u8g.drawStr( 0, 48, "  O BAJA SÓ TEM CAPIVARA ");
  u8g.drawStr( 0, 56, "      VAI CAPIBAJA!      ");
  };
*/
/*

void todosIcones() {      // funcao para testa todos os icones
 u8g.drawBitmapP( 0, 0, 2, 16, *icone_cronometro_preto);
 u8g.drawBitmapP( 0, 16, 2, 16, *icone_cronometro_branco);
 u8g.drawBitmapP( 0, 32, 2, 16, *icone_bateria_preto);
 u8g.drawBitmapP( 0, 48, 2, 16, *icone_bateria_branco);
 u8g.drawBitmapP( 16, 0, 2, 16, *icone_atencao_branco);
 u8g.drawBitmapP( 16, 16, 2, 16, *icone_atencao_preto);
 u8g.drawBitmapP( 16, 32, 2, 16, *icone_distancia_branco);
 u8g.drawBitmapP( 16, 48, 2, 16, *icone_distancia_preto);
 u8g.drawBitmapP( 32, 0, 2, 16, *icone_temperatura_branco);
 u8g.drawBitmapP( 32, 16, 2, 16, *icone_temperatura_preto);
 u8g.drawBitmapP( 32, 32, 2, 16, *icone_termometro_branco);
};
*/

/*
  u8g.drawBitmapP( 87, 16, 2, 16,  *icone_bateria_preto);
  u8g.drawBitmapP( 107, 16, 2, 16,  *icone_bateria_preto);
  u8g.drawBitmapP( 87, 0, 2, 16,  *icone_bateria_preto);
  u8g.drawBitmapP( 107, 0, 2, 16,  *icone_bateria_preto);
*/

/*
void escreve_icones_alerta(){
  //alerta_teste();
  u8g.drawBitmapP( 87,  0, 2, 16, *icone_x);   // mostra onde o ponteiro está apontando
  u8g.drawBitmapP( 87, 16, 2, 16, *icone_y);
  u8g.drawBitmapP( 107, 0, 2, 16, *icone_z);
  u8g.drawBitmapP( 107,16, 2, 16, *icone_w);
};
*/