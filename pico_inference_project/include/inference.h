#ifndef INFERENCE_H_
#define INFERENCE_H_

#include <cstddef>
#include <cstdint>

// Função para rodar a inferência a partir de um buffer de imagem
void run_inference_from_buffer(uint8_t* image_data,
                               size_t image_size,
                               int width,
                               int height);

#endif // INFERENCE_H_
