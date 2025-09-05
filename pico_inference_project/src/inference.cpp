#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_allocator.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "model_data.h"
#include "model_settings.h"
#include "inference.h"

namespace {

// TFLM globals
tflite::ErrorReporter *error_reporter = nullptr;
const tflite::Model *model = nullptr;
tflite::MicroInterpreter *interpreter = nullptr;
TfLiteTensor *input = nullptr;
TfLiteTensor *output = nullptr;

// Arena (ajuste se necessário)
constexpr int kTensorArenaSize = 200 * 1024; // 200 KiB
alignas(16) uint8_t tensor_arena[kTensorArenaSize];

bool is_initialized = false;

void initialize_interpreter() {
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;
    printf("[INFERENCE] Error reporter initialized\n");

    model = tflite::GetModel(model_data);
    printf("[INFERENCE] Model loaded, version: %d, expected: %d\n", model->version(), TFLITE_SCHEMA_VERSION);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        error_reporter->Report("Modelo incompatível com versão %d, esperado %d", model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

    // Resolver com as ops necessárias
    static tflite::MicroMutableOpResolver<11> resolver;
    resolver.AddConv2D();
    resolver.AddMaxPool2D();
    resolver.AddFullyConnected();
    resolver.AddSoftmax();
    resolver.AddReshape();
    resolver.AddRelu();
    resolver.AddQuantize();
    resolver.AddDequantize();
    resolver.AddDepthwiseConv2D();
    resolver.AddMul(); // Add MUL operation
    resolver.AddAdd();
    printf("[INFERENCE] Operations resolver configured\n");

    static tflite::MicroAllocator *allocator = tflite::MicroAllocator::Create(tensor_arena, kTensorArenaSize);
    if (!allocator) {
        error_reporter->Report("Falha ao criar MicroAllocator");
        return;
    }
    printf("[INFERENCE] Allocator created\n");

    static tflite::MicroInterpreter static_interpreter(model, resolver, allocator, nullptr, nullptr);
    interpreter = &static_interpreter;
    printf("[INFERENCE] Interpreter created\n");

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        error_reporter->Report("Falha ao alocar tensores. Arena size: %d bytes", kTensorArenaSize);
        return;
    }
    printf("[INFERENCE] Tensors allocated\n");

    input  = interpreter->input(0);
    output = interpreter->output(0);

    is_initialized = true;
    error_reporter->Report("Interpretador TFLM inicializado com sucesso");
}

} // namespace

void run_inference_from_buffer(uint8_t *buf, size_t len, int width, int height) {
    if (!buf || len == 0) {
        printf("[INFERENCE] Buffer vazio\n");
        return;
    }

    if (!is_initialized) {
        initialize_interpreter();
        if (!is_initialized) {
            printf("[INFERENCE] Falha ao inicializar interpretador TFLM\n");
            return;
        }
    }

    // Verifica dimensões esperadas conforme model_settings
    size_t expected = static_cast<size_t>(kNumRows) * kNumCols * kNumChannels;
    if (width != kNumRows || height != kNumCols) {
        printf("[INFERENCE] Dimensões inválidas: recebido %dx%d, esperado %dx%d\n", width, height, kNumRows, kNumCols);
        return;
    }
    if (len < expected) {
        printf("[INFERENCE] Buffer pequeno: %u bytes, esperado %u\n", (unsigned)len, (unsigned)expected);
        return;
    }

    // Copia/ajusta dados de entrada conforme o tipo do tensor de entrada
    switch (input->type) {
        case kTfLiteUInt8:
            memcpy(input->data.uint8, buf, expected);
            break;
        case kTfLiteInt8: {
            // Converte [0..255] para int8 com offset 128, comum em modelos quantizados
            int8_t *dst = input->data.int8;
            for (size_t i = 0; i < expected; ++i) {
                dst[i] = static_cast<int8_t>(static_cast<int>(buf[i]) - 128);
            }
            break;
        }
        case kTfLiteFloat32: {
            // Normaliza simples 0..1 caso o modelo seja float
            float *dst = input->data.f;
            for (size_t i = 0; i < expected; ++i) {
                dst[i] = static_cast<float>(buf[i]) / 255.0f;
            }
            break;
        }
        default:
            printf("[INFERENCE] Tipo de input não suportado: %d\n", input->type);
            return;
    }

    // Executa inferência
    if (interpreter->Invoke() != kTfLiteOk) {
        error_reporter->Report("Falha ao executar inferência");
        return;
    }

    // Lê saída e escolhe classe de maior score
    int out_len = (output->dims && output->dims->size > 0) ? output->dims->data[output->dims->size - 1] : kMaxCategoricalOutput;
    if (out_len <= 0) out_len = kMaxCategoricalOutput;

    int max_index = -1;
    int8_t max_q = INT8_MIN;

    float normalized_score = 0.0f;

    if (output->type == kTfLiteInt8 || output->type == kTfLiteUInt8) {
        // Varre logits quantizados (assumindo int8 para classificação)
        if (output->type == kTfLiteInt8) {
            for (int i = 0; i < out_len; ++i) {
                int8_t v = output->data.int8[i];
                if (v > max_q) { max_q = v; max_index = i; }
            }
        } else { // kTfLiteUInt8
            // Para uint8, converte para int8 equivalente ao comparar
            for (int i = 0; i < out_len; ++i) {
                int8_t v = static_cast<int8_t>(static_cast<int>(output->data.uint8[i]) - 128);
                if (v > max_q) { max_q = v; max_index = i; }
            }
        }

        // Dequantização via TfLiteAffineQuantization (API atual)
        const TfLiteAffineQuantization* quant =
            reinterpret_cast<const TfLiteAffineQuantization*>(output->quantization.params);

        if (quant && quant->scale && quant->scale->size > 0 && quant->zero_point && quant->zero_point->size > 0) {
            float scale = quant->scale->data[0];
            int32_t zero_point = quant->zero_point->data[0];
            // max_q representa o valor quantizado (int8). Para uint8 já ajustamos acima.
            normalized_score = (static_cast<int>(max_q) - zero_point) * scale;
        } else {
            // Fallback: sem metadados de quantização
            normalized_score = static_cast<float>(max_q);
        }
    } else if (output->type == kTfLiteFloat32) {
        // Saída em float: pega o maior
        float max_val = -1e30f;
        for (int i = 0; i < out_len; ++i) {
            float v = output->data.f[i];
            if (v > max_val) { max_val = v; max_index = i; }
        }
        normalized_score = max_val;
    } else {
        printf("[INFERENCE] Tipo de output não suportado: %d\n", output->type);
        return;
    }

    if (max_index >= 0 && max_index < kMaxCategoricalOutput) {
        printf("[INFERENCE] Classe prevista: %s (score: %.4f)\n", kCategoryLabels[max_index], normalized_score);
    } else {
        printf("[INFERENCE] Nenhuma classe válida prevista (idx=%d, out_len=%d)\n", max_index, out_len);
    }
}
