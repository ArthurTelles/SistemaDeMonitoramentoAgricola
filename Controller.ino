#include <ESP8266WiFi.h> // essa biblioteca já vem com a IDE. Portanto, não é preciso baixar nenhuma biblioteca adicional
#include <ThingSpeak.h>

// defines
#define SSID_REDE "nome-rede"   // coloque aqui o nome da rede que se deseja conectar
#define SENHA_REDE "senha-wifi" // coloque aqui a senha da rede que se deseja conectar

#define CHANNEL_ID COLE_SEU_ID_AQUI
#define CHANNEL_API_KEY "COLOQUE_SUA_CHAVE_AQUI"
#define leitor 4 // gpio4 = D2

// constantes e variáveis globais
char EnderecoAPIThingSpeak[] = "api.thingspeak.com";
String ChaveEscritaThingSpeak = "COLOQUE_SUA_CHAVE_AQUI";

WiFiClient client;

// protótipos
void EnviaInformacoesThingspeak(String StringDados);
void FazConexaoWiFi(void);
float FazLeituraUmidade(void);

/*
 * Implementações
 */

// Função: envia informações ao ThingSpeak
// Parâmetros: String com a informação a ser enviada
// Retorno: nenhum
void EnviaInformacoesThingspeak(String StringDados)
{
    if (client.connect(EnderecoAPIThingSpeak, 80))
    {
        // faz a requisição HTTP ao ThingSpeak
        client.print("POST /update HTTP/1.1\n");
        client.print("Host: api.thingspeak.com\n");
        client.print("Connection: close\n");
        client.print("X-THINGSPEAKAPIKEY: " + ChaveEscritaThingSpeak + "\n");
        client.print("Content-Type: application/x-www-form-urlencoded\n");
        client.print("Content-Length: ");
        client.print(StringDados.length());
        client.print("\n\n");
        client.print(StringDados);

        Serial.println("- Informações enviadas ao ThingSpeak!");
    }
}

// Função: faz a conexão WiFi
// Parâmetros: nenhum
// Retorno: nenhum
void FazConexaoWiFi(void)
{
    client.stop();
    Serial.println("Conectando-se à rede WiFi...");
    Serial.println();
    delay(1000);
    WiFi.begin(SSID_REDE, SENHA_REDE);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi conectado com sucesso!");
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());

    delay(1000);
}

// Função: faz a leitura do nível de umidade
// Parâmetros: nenhum
// Retorno: umidade percentual (0-100)
// Observação: o ADC do NodeMCU permite até, no máximo, 3.3V. Dessa forma,
//             para 3.3V, obtem-se (empiricamente) 978 como leitura de ADC
float FazLeituraUmidade(void)
{
    int ValorADC;
    float UmidadePercentual;

    ValorADC = analogRead(0);
    Serial.print("Leitura Sensor de Umidade: ");
    Serial.println(ValorADC);

    UmidadePercentual = map(ValorADC, 1023, 0, 0, 100);
    Serial.print("[Umidade Percentual] ");
    Serial.print(UmidadePercentual);
    Serial.println("%");

    return UmidadePercentual;
}

void setup()
{
    Serial.begin(9600);
    pinMode(leitor, OUTPUT);
    FazConexaoWiFi();
    ThingSpeak.begin(client);
    Serial.println("Projeto TCC - Arthur");
}

// loop principal
void loop()
{
    float UmidadePercentualLida;
    int UmidadePercentualTruncada;
    char FieldUmidade[11];

    // Força desconexão do ThingSpeak (caso ainda esteja conectado)
    if (client.connected())
    {
        client.stop();
        Serial.println("- Desconectado do ThingSpeak");
        Serial.println();
    }

    digitalWrite(leitor, HIGH); // ativa o sensor de umidade
    delay(50);
    UmidadePercentualLida = FazLeituraUmidade();
    UmidadePercentualTruncada = (int)UmidadePercentualLida; // trunca a umidade como número inteiro

    // sprintf(FieldUmidade,"field1=%d",UmidadePercentualTruncada);
    // EnviaInformacoesThingspeak(FieldUmidade);
    ThingSpeak.writeField(CHANNEL_ID, 1, UmidadePercentualTruncada, CHANNEL_API_KEY);

    digitalWrite(leitor, LOW); // desativa o sensor após a leitura
    .shutdown(nv->wss);        // modo de economia de energia (Forced Modem Sleep) para acelerar a transição ao Deep Sleep

    Serial.println(F("Dormindo por dez minutos a partir de agora..."));
    printMillis(); // mostra a diferença de tempo antes de entrar em modo de sono

    // entra em Deep Sleep por 10 minutos (0.6e9 microsegundos)
    ESP.deepSleep(0.6e9, WAKE_RF_DEFAULT);
    // Observações:
    // - Se usar ESP.deepSleep(0, modo), será necessário reiniciar manualmente para sair do sono
    // - O tempo máximo para Deep Sleep programado é cerca de 3 a 4 horas (limite do RTC)
    // - Durante o Deep Sleep, a corrente nos GPIOs é muito baixa (~2 µA), o que pode fazer o LED parecer apagado

    Serial.println("Estou dormindo, se essa mensagem aparecer, algo deu errado!");
}

void printMillis()
{
    Serial.print(F("millis() = ")); // mostra que o millis() não é confiável após o Deep Sleep
    Serial.println(millis());
    Serial.flush(); // garante que toda a mensagem seja enviada antes de entrar em modo de sono
}