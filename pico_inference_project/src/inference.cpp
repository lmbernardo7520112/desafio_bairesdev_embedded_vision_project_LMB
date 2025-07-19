#include "inference.h"
#include "ssd1306.h"
#include "pico/platform.h"
#include "hardware/uart.h"
#include "model_data.cc"

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#define TENSOR_ARENA_SIZE 20 * 1024
uint8_t tensor_arena[TENSOR_ARENA_SIZE];

void run_inference() {
    ssd1306_clear();
    ssd1306_draw_text(0, 0, "Processando...");
    ssd1306_show();

    static tflite::MicroErrorReporter micro_error_reporter;
    const tflite::Model* model = tflite::GetModel(modelo_expressoes_tflite);
    static tflite::MicroMutableOpResolver<8> resolver;

    resolver.AddConv2D();
    resolver.AddRelu();
    resolver.AddMaxPool2D();
    resolver.AddSoftmax();
    resolver.AddFullyConnected();
    resolver.AddReshape();
    resolver.AddBatchNorm();
    resolver.AddDropout();  // pode ser ignorado se não usado

    static tflite::MicroInterpreter interpreter(model, resolver, tensor_arena, TENSOR_ARENA_SIZE, &micro_error_reporter);
    interpreter.AllocateTensors();

    TfLiteTensor* input = interpreter.input(0);

    // Preencha input->data com os valores normalizados da imagem 48x48
    // Exemplo (pseudo):
    // for (int i = 0; i < 48*48; ++i) {
    //     input->data.f[i] = image_data[i];
    // }

    interpreter.Invoke();

    TfLiteTensor* output = interpreter.output(0);
    int predicted_class = std::distance(output->data.f, std::max_element(output->data.f, output->data.f + output->dims->data[1]));

    char texto[32];
    sprintf(texto, "Classe: %d", predicted_class);
    ssd1306_clear();
    ssd1306_draw_text(0, 0, texto);
    ssd1306_show();
}

