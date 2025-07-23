#include "image_provider.h"
#include <cmath>

float image_data[48 * 48];  // <--- Aqui você define

void FillImage() {
    for (int i = 0; i < 48 * 48; ++i) {
        image_data[i] = static_cast<float>(i % 256) / 255.0f;  // Exemplo: gradiente
    }
}

