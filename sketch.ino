//Inclusão das bibliotecas para uso do WIFI e uso do HTTP
#include <WiFi.h>
#include <HTTPClient.h>

#define led_verde 2 // Pino utilizado para controle do led verda
#define led_vermelho 40 // Pino utilizado para controle do led vermelho
#define led_amarelo 9 // Pino utilizado para controle do led azul

const int botaoPino = 18;  // Pino utilizado para controle do Botão
int botaoEstado = 0;  // Variável para ler o estado do botão

const int ldrPino = 4;  // Pino utilizado para controle do LDR
int luminosidade = 600; // Variável inteira para controle da luminosidade do LDR

int contagem = 0;

const char* estado = "A";

unsigned long ultimoBounce = 0; // Último tempo de mudança
unsigned long debouceTempo = 50;  // Delay para debounce (50 ms)
bool ultimoEstadoBotao = false;

void setupWifi() {
  WiFi.begin("Wokwi-GUEST", ""); // Conexão à rede WiFi aberta com SSID Wokwi-GUEST

  // Tenta se conectar com a rede WIFI
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("Conectado ao WiFi com sucesso!"); // Considerando que saiu do loop acima, o ESP32 agora está conectado ao WiFi (outra opção é colocar este comando dentro do if abaixo)

}

// modo que pisca o led amarelo
void modoNoturno() {
  digitalWrite(led_verde, LOW);
  digitalWrite(led_vermelho, LOW);
  digitalWrite(led_amarelo, LOW);
  digitalWrite(led_amarelo, HIGH);
  delay(500);
  digitalWrite(led_amarelo, LOW);
  delay(500);
}

// modo que liga o semáforo
void modoConvencional() {
  digitalWrite(led_vermelho, LOW);
  digitalWrite(led_verde, HIGH);
  estado = "A"; // estado do semaforo está aberto
  delay(3000);
  digitalWrite(led_verde, LOW);
  digitalWrite(led_amarelo, HIGH);
  estado = "E"; // estado do semaforo está no modo espera
  delay(2000);
  digitalWrite(led_amarelo, LOW);
  digitalWrite(led_vermelho, HIGH);
  estado = "F"; //estado do semaforo esté no modo fechado
  delay(5000);
}

// função para enviar a requisição
void enviaRequisicao() {
  if (WiFi.status() == WL_CONNECTED) { // Se o ESP32 estiver conectado à Internet
    HTTPClient http;

    String serverPath = "http://www.google.com.br/"; // Endpoint da requisição HTTP

    http.begin(serverPath.c_str()); // Inicializa o HTTP

    int httpResponseCode = http.GET(); // Código do Resultado da Requisição HTTP

    if (httpResponseCode > 0) {
      Serial.print("HTTP codigo de resposta: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
      Serial.println("Alerta enviado!!");
    }
    else {
      Serial.print("Codigo do erro: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }

  else {
    Serial.println("WiFi Desconectado");
    Serial.println("Alerta enviado!! (Offline)");
  }
}



void setup() {

  // Configuração inicial dos pinos para controle dos leds como OUTPUTs (saídas) do ESP32
  pinMode(led_verde, OUTPUT);
  pinMode(led_vermelho, OUTPUT);
  pinMode(led_amarelo, OUTPUT);

  // Inicialização das entradas
  pinMode(botaoPino, INPUT); // Configura o pino do botão como INPUT (Entrada)

  // Inicializa todos os LEDS como LOW, ou seja, desligado.
  digitalWrite(led_verde, LOW);
  digitalWrite(led_vermelho, LOW);
  digitalWrite(led_amarelo, LOW);

  Serial.begin(115200); // Configuração para debug por interface serial entre ESP e computador com baud rate de 115200.

  setupWifi(); // Inicializa a conexão com o wifi
}

void loop() {
  
  // Lê o estado analogico do LDR
  int ldrstatus = analogRead(ldrPino);
  bool leitura = digitalRead(botaoPino);


  if (ldrstatus <= luminosidade) {
    Serial.print("Esta escuro liga luzes");
    Serial.println(ldrstatus);
    modoNoturno();

  }

  else if (ldrstatus >= luminosidade && *estado == 'A') {
    Serial.print("Esta claro desliga luzes");
    Serial.println(ldrstatus);
    modoConvencional();
  }

  else if (ldrstatus >= luminosidade && *estado == 'F') {


    // Verifica se o botão mudou de estado (para debounce)
    if (leitura != ultimoEstadoBotao) {
      ultimoBounce = millis(); // Atualiza o tempo de debounce
    }

    // Verifica se o tempo de debounce passou
    if ((millis() - ultimoBounce) > debouceTempo) {
      // Se o estado do botão mudou
      if (leitura != botaoEstado) {
        botaoEstado = leitura;

        // Verifica se o botão está pressionado
        if (botaoEstado == HIGH) { // Se o estado for 1, botão foi pressionado
          contagem += 1;
          Serial.println(contagem);

          if (contagem == 1) {
            //volta para o semaforo normal
            modoConvencional();
            estado = "A";
          }

          if (contagem == 3) {
            Serial.println("Envio requisição!");
            enviaRequisicao();
            estado = "A";
          }
        } else { // Se não, botão não foi pressionado
          Serial.println("Botão não pressionado!");
          estado = "A";
        }
      }
    }


  }

  

  // Salva o estado atual do botão como o estado anterior
  ultimoEstadoBotao = leitura;
}

