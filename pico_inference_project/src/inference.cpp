#include "inference.h"
#include "pico/stdlib.h"

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "image_provider.h"
#include "model_data.h"
#include "model_settings.h"

#include <stdio.h>
#include <algorithm>
#include <stdarg.h>

#define TENSOR_ARENA_SIZE (20 * 1024)
uint8_t tensor_arena[TENSOR_ARENA_SIZE];

class SimpleErrorReporter : public tflite::ErrorReporter {
public:
    int Report(const char* format, va_list args) override {
        vprintf(format, args);
        printf("\n");
        return 0;
    }
};

void run_inference() {
    printf("  [INFERENCE] Iniciando processo de inferência...\n");

    static SimpleErrorReporter error_reporter;
    const tflite::Model* model = tflite::GetModel(model_data);

    static tflite::MicroMutableOpResolver<10> resolver;
    resolver.AddConv2D();
    resolver.AddRelu();
    resolver.AddMaxPool2D();
    resolver.AddSoftmax();
    resolver.AddFullyConnected();
    resolver.AddReshape();
    resolver.AddPad();
    resolver.AddStridedSlice();

    static tflite::MicroInterpreter interpreter(model, resolver, tensor_arena, TENSOR_ARENA_SIZE);
    interpreter.AllocateTensors();

    TfLiteTensor* input = interpreter.input(0);

    printf("  [INFERENCE] Preenchendo imagem de entrada...\n");
    FillImage();
    for (int i = 0; i < kMaxImageSize; ++i) {
        input->data.f[i] = image_data[i];
    }

    printf("  [INFERENCE] Invocando modelo TFLite...\n");
    TfLiteStatus status = interpreter.Invoke();
    if (status != kTfLiteOk) {
        printf("  [INFERENCE] ERRO: Falha ao invocar o modelo.\n");
        return;
    }

    TfLiteTensor* output = interpreter.output(0);
    int predicted_class = std::distance(
        output->data.f,
        std::max_element(output->data.f, output->data.f + output->dims->data[1])
    );

    printf("  ----------------------------------------\n");
    printf("  [INFERENCE] RESULTADO: Classe Predita = %d\n", predicted_class);
    printf("  ----------------------------------------\n");
}
