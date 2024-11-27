#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_wifi.h>
#include <mqtt_client.h>
#include <dht/dht.h>
#include <esp_event_loop.h>
#include <driver/gpio.h>

#define WIFI_SSID "PERFOLL"         // SSID da rede Wi-Fi
#define WIFI_PASS "35221153"       // Senha da rede Wi-Fi
#define DHT_GPIO 5                  // Pino D1 no ESP8266
#define LED_GPIO 2                  // Pino para o LED
#define MQTT_BROKER_URI "mqtt://test.mosquitto.org" // Broker MQTT

static const char *TAG = "MQTT_DHT";

// Variáveis para os dados do DHT11
int16_t umidade = 0;
int16_t temperatura = 0;

// Handle do cliente MQTT
esp_mqtt_client_handle_t mqtt_client = NULL;

// Função de evento de Wi-Fi
static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        ESP_LOGI(TAG, "Tentando reconectar ao Wi-Fi...");
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "Conectado ao Wi-Fi");
        break;
    default:
        break;
    }
    return ESP_OK;
}

// Função para inicializar o Wi-Fi
void wifi_init()
{
    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

// Função para inicializar o cliente MQTT
void mqtt_init()
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_BROKER_URI,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(mqtt_client);
}

// Função para acender o LED
void led_on()
{
    gpio_set_level(LED_GPIO, 1); // Acende o LED
}

// Função para apagar o LED
void led_off()
{
    gpio_set_level(LED_GPIO, 0); // Apaga o LED
}

// Tarefa para ler os dados do DHT11 e publicar via MQTT
void temperatura_task(void *arg)
{
    ESP_ERROR_CHECK(dht_init(DHT_GPIO, false));
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    while (1)
    {
        if (dht_read_data(DHT_TYPE_DHT11, DHT_GPIO, &umidade, &temperatura) == ESP_OK)
        {
            // Converter temperatura para string
            char temperatura_str[10];
            snprintf(temperatura_str, sizeof(temperatura_str), "%d.%d", temperatura / 10, temperatura % 10);

            // Converter umidade para string
            char umidade_str[10];
            snprintf(umidade_str, sizeof(umidade_str), "%d.%d", umidade / 10, umidade % 10);

            // Publicar a temperatura
            esp_mqtt_client_publish(mqtt_client, "mestrado/iot/aluno/ademar/temperatura", temperatura_str, 0, 1, 0);
            ESP_LOGI(TAG, "Publicando temperatura: %s", temperatura_str);
            led_on();  // Acende o LED
            vTaskDelay(1000 / portTICK_PERIOD_MS); // Mantém o LED aceso por 1 segundo
            led_off(); // Apaga o LED
			
			vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay de 3 segundos

            // Publicar a umidade
            esp_mqtt_client_publish(mqtt_client, "mestrado/iot/aluno/ademar/umidade", umidade_str, 0, 1, 0);
            ESP_LOGI(TAG, "Publicando umidade: %s", umidade_str);
            led_on();  // Acende o LED
            vTaskDelay(1000 / portTICK_PERIOD_MS); // Mantém o LED aceso por 1 segundo
            led_off(); // Apaga o LED
        }
        else
        {
            ESP_LOGI(TAG, "Falha ao buscar dados do DHT11");
        }

        vTaskDelay(120000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void app_main()
{
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT); // Configura o pino do LED como saída
    wifi_init();
    mqtt_init();
    xTaskCreate(temperatura_task, "temperatura_task", 4096, NULL, 5, NULL);
}
