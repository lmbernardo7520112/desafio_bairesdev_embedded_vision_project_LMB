#ifndef MODEL_SETTINGS_H_
#define MODEL_SETTINGS_H_

// Definir o número máximo de saídas categóricas (número de classes do modelo)
// Baseado no seu notebook Colab, o modelo tem 7 classes (emoções).
const int kMaxCategoricalOutput = 7;

// Definir os rótulos das categorias (emoções)
// Certifique-se de que a ordem e os nomes correspondem aos do seu modelo treinado.
const char* const kCategoryLabels[] = {
    "Angry",
    "Disgust",
    "Fear",
    "Happy",
    "Sad",
    "Surprise",
    "Neutral"
};

#endif  // MODEL_SETTINGS_H_
