/*UPDATES E DETALHES DO CÓDIGO
 * Erros para corrigir...
 * ... ICONE DE ATENCÃO BRANCO E O PRETO SÃO IGUAIS
 * ... FALTA UM DOS ICONES DO TERMOMETRO
 * ... ARRUMAR O LOGO DA CAPIBAJA
 * ... CRIAR UMA BIBLIOTECA SEPARADA
 * ... ADICIONAR LEITURA DE LUZ DE FREIO
 * ... TESTES DE MEMORIA COM USO DE PONTEIROS E SEM O USO
 *
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
 *
 *
*/

#include <U8glib.h>
#include "icones.h"
#define sensor 2

typedef void (funcao)(void);

U8GLIB_ST7920_128X64_1X u8g(6, 5, 4 ,7); //Enable, RW, RS, RESET

volatile int contador = 0;
int i = 0,
    rpm = 0, 
    odometro = 0,
    sensor_bateria = 0,
    sensor_temperatura = 0;
    

unsigned long tempo = 0;      // usado para sincronia

char buf[9],
     str_odometro[15],
     str_bateria[15],
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

void imprime(funcao *tela){
    u8g.firstPage();
    do{
    tela();}
    while(u8g.nextPage());
}

void curiosidadeCapivaras(){
  u8g.setFont(u8g_font_5x8);
  u8g.drawStr( 0, 8, "        A CAPIVARA        ");
  u8g.drawStr( 0, 16, " O MAIOR ROEDOR DO MUNDO ");
  u8g.drawStr( 0, 24, "OS BAJEIROS SÃO CAPIVARAS");
  u8g.drawStr( 0, 32, " BAJEIROS VIVEM NO BARRO ");
  u8g.drawStr( 0, 40, " BAJA INSPIRA AS CAPIVARA");
  u8g.drawStr( 0, 48, "  O BAJA SÓ TEM CAPIVARA ");
  u8g.drawStr( 0, 56, "      VAI CAPIBAJA!      ");}

// DrawBitmapP(x,y,bits do hexa, colunas, o que vai imprimir);
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

void informacoes(){  // Carrega todas as informacoes
  u8g.setFont(u8g_font_courB10r); // Escolhe a fontes
  sprintf(str_temperatura, "Temp: %d C", sensor_temperatura);
  u8g.drawStr( 0, 12,str_temperatura); // posiciona e imprime
  sprintf(str_bateria, "Tensao: %d V", sensor_bateria);
  u8g.drawStr( 0, 24, str_bateria);
  u8g.drawStr( 0, 36, "Freios:     OK");
  sprintf(str_odometro, "Dist: %d km", odometro);
  u8g.drawStr( 0, 48,str_odometro);
  u8g.drawStr( 0, 60, "Alguma info :)");
};

void capibaja(){
  u8g.drawBitmapP( 0, 0, 16, 128, capibaja_logo);
};

void escreve_velo(){
  int x = 2, y = 38;
  sprintf(buf, "%d", rpm);        // vai saber, mas tem que ter para funcionar
  u8g.setFont(u8g_font_fur35n);   // Fonte Grande para velocidade
  u8g.setFontPosBaseline();

  if(rpm <= 9){                     // Essa Gambiarra é para fazer o scroll da direita para a esquerda
  u8g.drawStr(x+27,y, buf);}

  else{
    u8g.drawStr(x,y, buf);}      // Escreve valor da velocidade

  u8g.setFont(u8g_font_courB14r);// Fonte Pequena para velocidade
  u8g.setFontPosBaseline();
  u8g.drawStr(56,38,"Km/h");     // Desenha texto
};

void escreve_icones(){
  u8g.drawBitmapP( 5, 44, 2, 16,  *icone_bateria_preto);
  u8g.drawBitmapP( 39, 44, 2, 16, *icone_distancia_branco);
  u8g.drawBitmapP( 73, 44, 2, 16, *icone_termometro_branco);
  u8g.drawBitmapP( 107, 44, 2, 16,*icone_temperatura_branco);
};

void escreve_icones_alerta(){
  //alerta_teste();
  u8g.drawBitmapP( 5, 44, 2, 16,  *icone_x);   // mostra onde o ponteiro está apontando
  u8g.drawBitmapP( 39, 44, 2, 16, *icone_y);
  u8g.drawBitmapP( 73, 44, 2, 16, *icone_z);
  u8g.drawBitmapP(107, 44, 2, 16, *icone_w);
};

void telaPrincipal(){ // Mostra tela principal
  escreve_icones();
  escreve_velo();
};

void telaPrincipal_2(){ // Mostra tela principal
  escreve_icones_alerta();
  escreve_velo();
};

void contagem(){ //Funcao para o velocimetro
  contador++;
}

int verifica(){
  i = analogRead(A0)/10;
  sensor_bateria = analogRead(A1);
  sensor_temperatura = analogRead(A2);

    if(i>5 && i <= 30){           //checa o potenciometro e troca os icones
    *icone_x = *icone_bateria_branco;
    *icone_y = *icone_distancia_branco;
    *icone_z = *icone_termometro_branco;
    return false;}

    else if (i>30 && i <= 50){
    *icone_x = *icone_bateria_preto;
    *icone_y = *icone_distancia_preto;
    *icone_z = *icone_termometro_branco;
    return false;}

    else if (i>50 && i <= 95){
    *icone_x = *icone_bateria_preto;
    *icone_y = *icone_distancia_branco;
    *icone_z = *icone_temperatura_branco;
    return false;}

    else if (i>95 && i <= 120){
    return 55;}

    else
      return 1;
}




void setup(){
   analogReference(EXTERNAL);
   pinMode(A0, INPUT);       // Leitura da velocidade
   u8g.setColorIndex(1);     // Liga pixels
   Serial.begin(9600);       // Inicia a comunicação serial
   pinMode(sensor, INPUT_PULLUP); //Inicia o Sensor de velocidade
   attachInterrupt(digitalPinToInterrupt(sensor), contagem, CHANGE); //aciona a interrupção
   imprime(capibaja);
   delay(1000);
   imprime(curiosidadeCapivaras);
   delay(7000);

}

void loop(){
  if((millis() - tempo) >= 1000){  // calculo da velocidade
    noInterrupts();
    rpm = contador;
    odometro = odometro + contador;
    contador = 0;
    tempo = millis();}
  interrupts();
  if(verifica() == 1){   //se for igual a 1 mostra a telaPrincipal
    imprime(telaPrincipal);
  }
  else if(verifica() == 55){  //se for igual a 55 mostra a tela de informacoes
      imprime(informacoes);
  }
  else{                //Se acontecer algum problema entra nessa parte do loop
    if((millis() - tempo) >= 500){
    imprime(telaPrincipal);
    }
    if((millis() - tempo) < 500){
    imprime(telaPrincipal_2);
    }
  }
}



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
*/

/*
void curiosidadeCapivaras(){
  u8g.setFont(u8g_font_5x8);
  u8g.drawStr( 0, 8, "        A CAPIVARA        ");
  u8g.drawStr( 0, 16, "É o maior roedor do mundo");
  u8g.drawStr( 0, 24, "    pesando até 91 kg    ");
  u8g.drawStr( 0, 32, "    medindo até 1,2 m    ");
  u8g.drawStr( 0, 40, "A CAPIVARA EXPIROU O BAJA");
  u8g.drawStr( 0, 48, "  O BAJA SÓ TEM CAPIVARA ");
  u8g.drawStr( 0, 56, "   CAPIVARAS SÃO DAORA   ");
};
*/
