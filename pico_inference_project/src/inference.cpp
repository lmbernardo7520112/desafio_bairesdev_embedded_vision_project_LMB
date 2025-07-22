#include "inference.h"
#include "ssd1306.h"
#include "pico/platform.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include <stdio.h>
#include <algorithm>
#include "tensorflow/lite/micro/kernels/max_pooling.h"
#include "tensorflow/lite/micro/kernels/softmax.h"
#include "tensorflow/lite/micro/kernels/fully_connected.h"
#include "image_provider.h"


// Declarações externas (definidas em model_data.cc)
extern const unsigned char modelo_expressoes_tflite[];
extern const unsigned int modelo_expressoes_tflite_len;

#define TENSOR_ARENA_SIZE (20 * 1024)
uint8_t tensor_arena[TENSOR_ARENA_SIZE];

// Declare o display como global ou passe como argumento se preferir
ssd1306_t display;

// Reporter simples implementando a interface abstrata
class SimpleErrorReporter : public tflite::ErrorReporter {
public:
    int Report(const char* format, va_list args) override {
        vprintf(format, args);
        printf("\n");
        return 0;
    }
};

void run_inference() {
    // Limpa e escreve "Processando..." na tela
    ssd1306_clear(&display);
    ssd1306_draw_string(&display, 0, 0, 1, "Processando...");
    ssd1306_show(&display);
    
    // Configuração do modelo
    static SimpleErrorReporter error_reporter;
    const tflite::Model* model = tflite::GetModel(modelo_expressoes_tflite);
    
    static tflite::MicroMutableOpResolver<10> resolver;
    resolver.AddConv2D();
    resolver.AddRelu(); // Use AddActivation com parâmetro ReLU se necessário
    resolver.AddMaxPool2D();
    resolver.AddSoftmax();
    resolver.AddFullyConnected();
    resolver.AddReshape();
    resolver.AddPad();
    resolver.AddStridedSlice();
    // resolver.AddBatchNorm(); // Não disponível no TFLM
    // resolver.AddDropout(); // Não disponível no TFLM
    
    static tflite::MicroInterpreter interpreter(model, resolver, tensor_arena, TENSOR_ARENA_SIZE);
    
    interpreter.AllocateTensors();
    
    TfLiteTensor* input = interpreter.input(0);
    // Preencha input->data com os valores normalizados da imagem 48x48
    // Exemplo (pseudo):
    for (int i = 0; i < 48*48; ++i) {
        input->data.f[i] = image_data[i];
    }
    
    TfLiteStatus status = interpreter.Invoke();
    if (status != kTfLiteOk) {
        printf("Falha ao invocar o modelo.\n");
        return;
    }
    
    TfLiteTensor* output = interpreter.output(0);
    int predicted_class = std::distance(output->data.f, std::max_element(output->data.f, output->data.f + output->dims->data[1]));
    
    char texto[32];
    snprintf(texto, sizeof(texto), "Classe: %d", predicted_class);
    
    ssd1306_clear(&display);
    ssd1306_draw_string(&display, 0, 0, 1, texto);
    ssd1306_show(&display);
}
