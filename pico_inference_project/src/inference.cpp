// inference.cpp
// Implementação de run_inference_from_buffer(...) usada pelo receptor UDP.
// Esta versão faz validações básicas e mostra logs.
// Substitua/integre a lógica TFLM aqui conforme seu modelo.

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "model_settings.h" // kNumRows, kNumCols, kNumChannels
#include "inference.h"      // declaração da função (header)

#ifndef K_EXPECTED_IMAGE_BYTES
// Se seu modelo espera kNumRows*kNumCols*(kNumChannels), definimos aqui:
#define K_EXPECTED_IMAGE_BYTES (kNumRows * kNumCols * kNumChannels)
#endif

// Expor a função que o receiver chama
void run_inference_from_buffer(uint8_t* buf, size_t len, int width, int height) {
    // validações simples
    if (!buf || len == 0) {
        printf("[INFERENCE] Buffer vazio\n");
        return;
    }
    printf("[INFERENCE] Buffer recebido: %u bytes (w=%d h=%d)\n", (unsigned)len, width, height);

    // Se o modelo foi treinado para expect grayscale raw of exact size (48x48), cheque:
    size_t expected = (size_t)width * (size_t)height * (size_t)kNumChannels;
    if (len < expected) {
        printf("[INFERENCE] Aviso: bytes recebidos (%u) < esperado (%u). Continuando com o que há.\n",
               (unsigned)len, (unsigned)expected);
    }

    // >>> Aqui é o ponto onde você integra seu TFLM:
    // - Converter os bytes recebidos para o formato do tensor (float ou uint8)
    // - Preencher o tensor de entrada
    // - Invocar o interpreter
    //
    // Por enquanto, apenas log e um resultado de placeholder:

    // (Exemplo: print primeiros 8 bytes para debug)
    printf("[INFERENCE] primeiros bytes (hex):");
    for (size_t i = 0; i < len && i < 8; ++i) {
        printf(" %02X", buf[i]);
    }
    printf("\n");

    // Resultado placeholder (substitua pelo resultado real do modelo)
    int predicted_class = -1;
    // se quiser, execute aqui sua run_inference() existente se já estiver implementada:
    // run_inference(); // se você tiver uma função global que já usa FillImage etc.

    // Exibir resultado (placeholder)
    if (predicted_class >= 0) {
        printf("[INFERENCE] Classe prevista: %d\n", predicted_class);
    } else {
        printf("[INFERENCE] Inferência não implementada - esta é uma stub.\n");
    }
}
