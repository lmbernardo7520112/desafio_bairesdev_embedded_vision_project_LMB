// =======================================================================
//  ARQUIVO: SRC/MODEL_SETTINGS.H (VERSÃO FINAL E COMPLETA)
// =======================================================================
#ifndef MODEL_SETTINGS_H_
#define MODEL_SETTINGS_H_

// --- CONFIGURAÇÕES DA ENTRADA DA IMAGEM ---
// Adicione estas três linhas. Elas definem as dimensões da imagem
// que o seu modelo de reconhecimento de emoções espera.
constexpr int kNumRows = 48;
constexpr int kNumCols = 48;
constexpr int kMaxImageSize = kNumRows * kNumCols; // 48 * 48 = 2304

// --- CONFIGURAÇÕES DA SAÍDA DO MODELO ---
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
